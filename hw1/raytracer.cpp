#define EPSILON 0.001

#include <iostream>
#include <cmath>
#include <algorithm>
#include <set>
#include "parser.h"
#include "ppm.h"
#include "Ray.h"
#include "HitInfo.h"
#include "Node3D.h"
#include "treeBuilder.h"
#include <thread>
#include <mutex>
#include <vector>


using namespace parser;
using namespace std;

typedef unsigned char* Image;

float determinant (Vec3f a, Vec3f b, Vec3f c);
void CalculateFaceNormals(Scene &scene);

Vec3f GetRayDirection(Camera camera, int x, int y);
Vec3f GetFaceNormal(Ray ray, Face face, Scene &scene);
Vec3f GetSphereNormal(Ray ray, Vec3f intersection_point, Sphere sphere, Scene &scene);
Ray Reflect(Vec3f normal, Vec3f incoming_direction, Vec3f intersection_point);

float RaySphereIntersect(Ray ray, Sphere sphere, Scene &scene);
float RayFaceIntersect(Ray ray, Face face, Scene &scene, bool backface_culling);
float RayMeshIntersect(Ray ray, Mesh mesh, Node3D *tree, Scene &scene, Face &face_out, bool backface_culling);
float RayTreeIntersect(Ray ray, Node3D *node, Scene &scene, Face& face_out, bool backface_culling);
float RayBoundingBoxIntersect(Ray ray, Vec3f minVertex, Vec3f maxVertex);

HitInfo RayIntersect(Ray ray, Camera &camera, Scene &scene, vector<Node3D*> &meshTrees, bool backface_culling);
Vec3i GetColor(HitInfo hit_info, Camera &camera, Scene &scene, int depth, vector<Node3D*> &meshTrees);

void findUniqueVertices(vector<Face> &faces, vector<int> &vertices);

std::mutex imageMutex;

void RenderImage(Camera camera, Scene &scene,Image &image, int startRow, int endRow, vector<Node3D*> &meshTrees)
{
    int width = camera.image_width;
    for (int y = startRow; y < endRow; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            // Logic
            Ray ray = Ray(camera.position, GetRayDirection(camera, x, y));
            HitInfo hit_info = RayIntersect(ray, camera, scene, meshTrees, true);
            
            int pixelIndex = (y * width + x) * 3; // Index in the image buffer

            if(hit_info.is_hit)
            {                    
                Vec3i color = GetColor(hit_info, camera, scene, 0, meshTrees);
                // Lock the image buffer before writing
                std::unique_lock<std::mutex> lock(imageMutex);
                image[pixelIndex] = color.x; // R
                image[pixelIndex + 1] = color.y; // G
                image[pixelIndex + 2] = color.z; // B
            }
            else
            {
                // Lock the image buffer before writing
                std::unique_lock<std::mutex> lock(imageMutex);
                image[pixelIndex] = scene.background_color.x; // R
                image[pixelIndex + 1] = scene.background_color.y; // G
                image[pixelIndex + 2] = scene.background_color.z; // B
            }
        }
    }
}

int main(int argc, char* argv[])
{
    int numThreads = thread::hardware_concurrency();
    Scene scene;

    scene.loadFromXml(argv[1]);

    // Calculate Face Normals
    CalculateFaceNormals(scene);

    vector<Node3D*> meshTrees(scene.meshes.size());
    treeBuilder treebuilder;
    for (int i = 0; i < scene.meshes.size(); ++i)
    {   
        Mesh* mesh = &scene.meshes[i];
        mesh->mesh_id = i;
        vector <int> uniqueVertices;
        findUniqueVertices(mesh->faces, uniqueVertices);
        Vec3f boundingMin;
        Vec3f boundingMax;
        treebuilder.findBoundingBox(mesh->faces, scene, boundingMin, boundingMax);
        Node3D* root = treebuilder.buildTree(mesh->faces, uniqueVertices, scene, 0, boundingMin, boundingMax);
        //PrintTree(root, scene, mesh->material_id);
        meshTrees[mesh->mesh_id] = root;
    }
    for (Camera camera : scene.cameras)
    {
        int width = camera.image_width;
        int height = camera.image_height;

        Image image = new unsigned char [width * height * 3]; // 3 channels per pixel (RGB)

        int numRowsPerThread = height / numThreads;

        std::vector<std::thread> threads;

        for (int i = 0; i < numThreads; ++i)
        {
            int startRow = i * numRowsPerThread;
            int endRow = (i == numThreads - 1) ? height : (startRow + numRowsPerThread);

            threads.emplace_back(RenderImage, ref(camera), ref(scene), ref(image), startRow, endRow, ref(meshTrees));
        }

        // Wait for all threads to finish
        for (std::thread &thread : threads)
        {
            thread.join();
        }
        std::unique_lock<std::mutex> lock(imageMutex);
        write_ppm(camera.image_name.c_str(), image, width, height);
    }
       
}

Vec3f GetRayDirection(Camera camera, int i, int j)
{
    // v = Camera up vector
    // -w = Camera gaze vector
    // u = cross(v,w) 

    Vec3f e = camera.position;
    Vec3f v = camera.up;
    Vec3f w = camera.gaze * -1;
    Vec3f u = v.cross(w);

    float l = camera.near_plane.x;
    float r = camera.near_plane.y;
    float b = camera.near_plane.z;
    float t = camera.near_plane.w;

    Vec3f m = e + w * camera.near_distance * -1;

    Vec3f q = m + u * l + v * t;
    float su = (i+0.5) * (r-l) / camera.image_width;
    float sv = (j+0.5) * (t-b) / camera.image_height;

    Vec3f s = q + u * su - v * sv;

    Vec3f d = s-e;
    return d;
}


Ray Reflect(Vec3f normal, Vec3f incoming_direction, Vec3f intersection_point)
{
    incoming_direction = incoming_direction.normalize();
    normal = normal.normalize();
    Vec3f reflection_direction = incoming_direction - normal * (incoming_direction.dot(normal) * 2);
    return Ray(intersection_point + (reflection_direction * EPSILON), reflection_direction);
}

float RaySphereIntersect(Ray ray, Sphere sphere, Scene &scene)
{
    Vec3f o = ray.getOrigin();
    Vec3f d = ray.getDirection();
    Vec3f c = scene.vertex_data[sphere.center_vertex_id - 1];
    float d1 = pow(d.dot(o - c), 2);
    float d2 = -(d.dot(d)) * ((o - c).dot(o - c) - pow(sphere.radius, 2));
    float discriminant = d1 + d2;

    if(discriminant < 0) 
        return -1;
    
    float t1 = ((d.dot(o - c) * -1) - pow(discriminant,0.5)) / d.dot(d);
    float t2 = ((d.dot(o - c) * -1) + pow(discriminant,0.5)) / d.dot(d);

    return t1 < t2 ? t1 : t2;
}

float RayFaceIntersect(Ray ray, Face face, Scene &scene, bool backface_culling){
    Vec3f o = ray.getOrigin();
    Vec3f d = ray.getDirection();
    
    Vec3f a = scene.vertex_data[face.v0_id - 1];
    Vec3f b = scene.vertex_data[face.v1_id - 1];
    Vec3f c = scene.vertex_data[face.v2_id - 1];

    if (backface_culling)
    {
        if(face.normal.dot(d) > 0)
            return -1;
    }

    float A = determinant(a-b,a-c,d);
    float beta = determinant(a-o,a-c,d)/A;
    float gamma = determinant(a-b,a-o,d)/A;
    float t = determinant(a-b,a-c,a-o)/A;

    if(beta >= 0 && gamma >= 0 && beta + gamma <= 1){
        return t;
    }
    return -1;
}

float RayMeshIntersect(Ray ray, Mesh mesh, Node3D *tree, Scene &scene, Face &face_out, bool backface_culling)
{   
    return RayTreeIntersect(ray, tree, scene, face_out, backface_culling);
}
float RayTreeIntersect(Ray ray, Node3D *node, Scene &scene, Face& face_out, bool backface_culling)
{
    if(node == nullptr)
        return -1;

    // First check the bounding box, then if node has children, check them else check the faces

    if(RayBoundingBoxIntersect(ray, node->boundingMin, node->boundingMax) == -1)
        return -1;

    // If node has children, check them
    if(node->left != nullptr || node->right != nullptr)
    {
        Face f1 = Face();
        Face f2 = Face();
        float t1 = RayTreeIntersect(ray, node->left, scene, f1, backface_culling);
        float t2 = RayTreeIntersect(ray, node->right, scene, f2, backface_culling);
        
        if (t1 > 0 && t2 > 0) 
            if(t1 < t2)
            {
                face_out = f1;
                return t1;
            }
            else
            {
                face_out = f2;
                return t2;
            }
        // If one of the children is hit, return it
        else if (t1 > 0) 
            {
                face_out = f1;
                return t1;
            }
        else if (t2 > 0) 
            {
                face_out = f2;
                return t2;
            }
        else 
            return -1; // If no child is hit, return -1
    }
    // If node has no children, check the faces
    else
    {
        float tmin = INFINITY;
        for(Face face : node->faces){
            float t = RayFaceIntersect(ray, face, scene, backface_culling);
            if(t > 0 && t < tmin){
                tmin = t;
                face_out = face;
            }
        }
        return tmin == INFINITY ? -1 : tmin; // If no face is hit, return -1
    }
    
}

float RayBoundingBoxIntersect(Ray ray, Vec3f minVertex, Vec3f maxVertex)
{
    Vec3f o = ray.getOrigin();
    Vec3f d = ray.getDirection();
    Vec3f invD = Vec3f{1/d.x, 1/d.y, 1/d.z};

    double tMinX = (minVertex.x - o.x) * invD.x;
    double tMaxX = (maxVertex.x - o.x) * invD.x;
    if (tMinX > tMaxX) swap(tMinX, tMaxX);

    double iDy = 1 / d.y;
    double tMinY = (minVertex.y - o.y) * iDy;
    double tMaxY = (maxVertex.y - o.y) * iDy;
    if (tMinY > tMaxY) swap(tMinY, tMaxY);

    double iDz = 1 / d.z;
    double tMinZ = (minVertex.z - o.z) * iDz;
    double tMaxZ = (maxVertex.z - o.z) * iDz;
    if (tMinZ > tMaxZ) swap(tMinZ, tMaxZ);

    double tEnter = max(tMinX, max(tMinY, tMinZ));
    double tExit = min(tMaxX, min(tMaxY, tMaxZ));

    if (tEnter <= tExit && tExit > 0)
    {
        return tEnter;
    }
    else return -1;
}

float determinant (Vec3f a, Vec3f b, Vec3f c){
    return a.x * (b.y * c.z - b.z * c.y) + a.y * (b.z * c.x - b.x * c.z) + a.z * (b.x * c.y - b.y * c.x);
}

Vec3f GetSphereNormal(Ray ray, Vec3f intersection_point, Sphere sphere, Scene &scene)
{
    Vec3f center = scene.vertex_data[sphere.center_vertex_id - 1];
    Vec3f normal = intersection_point - center;
    return normal.normalize();
}

void PrintHitInfo(HitInfo hit_info)
{
    /* cout << "Hit Info: " << endl; */
    cout << "Material ID: " << hit_info.material_id << endl;
    /* cout << "Normal: " << hit_info.normal.x << " " << hit_info.normal.y << " " << hit_info.normal.z << endl;
    cout << "Hit Point: " << hit_info.hit_point.x << " " << hit_info.hit_point.y << " " << hit_info.hit_point.z << endl; */
}

HitInfo RayIntersect(Ray ray, Camera &camera, Scene &scene, vector<Node3D*> &meshTrees, bool backface_culling)
{
    HitInfo info = HitInfo();
    float tmin = INFINITY;
    for (Sphere sphere: scene.spheres)
    {
        // TODO: faster intersection test with dot product and radius !!!

        float t = RaySphereIntersect(ray, sphere, scene);
        if (t > 0 && t < tmin)
        {
            tmin = t;
            info.is_hit = true;
            info.t = t;
            info.material_id = sphere.material_id;
            info.hit_point = ray.getOrigin() + ray.getDirection() * t;
            info.normal = GetSphereNormal(ray, info.hit_point, sphere, scene);
            //hitting ray missing
            info.hitting_ray = ray;
        }
    }
    for (Mesh mesh: scene.meshes)
    {
        Face face;
        float t = RayMeshIntersect(ray, mesh, meshTrees[mesh.mesh_id] , scene, face, backface_culling);
        if (t > 0 && t < tmin)
        {
            tmin = t;
            info.is_hit = true;
            info.t = t;
            info.material_id = mesh.material_id;
            info.hit_point = ray.getOrigin() + ray.getDirection() * t;
            info.normal = face.normal;
            //hitting ray missing
            info.hitting_ray = ray;
            /* if(info.is_hit)
            {
                PrintHitInfo(info);
            } */
        }

    }
    for (Triangle triangle: scene.triangles)
    {
        float t = RayFaceIntersect(ray, triangle.indices, scene, backface_culling);
        if (t > 0 && t < tmin)
        {
            tmin = t;
            info.is_hit = true;
            info.t = t;
            info.material_id = triangle.material_id;
            info.hit_point = ray.getOrigin() + ray.getDirection() * t;
            info.normal = triangle.indices.normal;
            //hitting ray missing
            info.hitting_ray = ray;
        }
    }
    return info;
}

Vec3i GetColor(HitInfo hit_info, Camera &camera, Scene &scene, int depth, vector<Node3D*> &meshTrees)
{
    if(depth > scene.max_recursion_depth)
        return Vec3i{0,0,0};

    Vec3f intersection_point = hit_info.hit_point;
    Vec3f normal = hit_info.normal;
    Ray ray = hit_info.hitting_ray;
    Vec3f rayDir = ray.getDirection();

    float color_r = 0;
    float color_g = 0;
    float color_b = 0;
    
    Material material = scene.GetMaterial(hit_info.material_id); // Get material of the object that is hit

    for (PointLight light: scene.point_lights)
    {
        Vec3f light_direction = light.position - intersection_point;
        
        // Shadow
        auto inShadow = RayIntersect(Ray(intersection_point + (normal * scene.shadow_ray_epsilon), light_direction), camera, scene, meshTrees, false);
        if(inShadow.is_hit == true && inShadow.t >= 0 && inShadow.t <= 1) continue;
        
        Vec3f l = light_direction.normalize();
        Vec3f v = rayDir.normalize() * -1;
        // Diffuse 
        float distanceFromLight_squared = light_direction.length() * light_direction.length();
        float dotProduct = l.dot(normal);
        if (dotProduct < 0)
            dotProduct = 0;

        color_r += material.diffuse.x * light.intensity.x * dotProduct / distanceFromLight_squared;
        color_g += material.diffuse.y * light.intensity.y * dotProduct / distanceFromLight_squared;
        color_b += material.diffuse.z * light.intensity.z * dotProduct / distanceFromLight_squared;
        

        // Specular
        Vec3f h = (l + v) / ((l + v).length());
        
        dotProduct = h.dot(normal);
        if (dotProduct < 0)
            dotProduct = 0;
        float phong = pow(dotProduct, material.phong_exponent);
        
        color_r += material.specular.x * light.intensity.x * phong / distanceFromLight_squared;
        color_g += material.specular.y * light.intensity.y * phong / distanceFromLight_squared;
        color_b += material.specular.z * light.intensity.z * phong / distanceFromLight_squared;
    }

    // Ambient
    color_r += material.ambient.x * scene.ambient_light.x;
    color_g += material.ambient.y * scene.ambient_light.y;
    color_b += material.ambient.z * scene.ambient_light.z;
    // Reflection
    
    if(material.is_mirror)
    {
        Ray reflection_ray = Reflect(normal, rayDir, intersection_point);
        auto reflection_info = RayIntersect(reflection_ray, camera, scene, meshTrees, false);
        if (reflection_info.is_hit)
        {
            auto reflection_color = GetColor(reflection_info, camera, scene, depth + 1, meshTrees);
            color_r += material.mirror.x * reflection_color.x;
            color_g += material.mirror.y * reflection_color.y;
            color_b += material.mirror.z * reflection_color.z; 
        }   
    }

    if(color_r > 255) color_r = 255;
    if(color_g > 255) color_g = 255;
    if(color_b > 255) color_b = 255;

    int icolor_r = (int) round(color_r);
    int icolor_g = (int) round(color_g);
    int icolor_b = (int) round(color_b);
    
    return Vec3i{icolor_r, icolor_g, icolor_b};
}

void findUniqueVertices(vector<Face> &faces, vector<int> &vertices){
    set<int> uniqueVertices;
    
    for(Face face : faces){
        uniqueVertices.insert(face.v0_id);
        uniqueVertices.insert(face.v1_id);
        uniqueVertices.insert(face.v2_id);
    }

    for(int vertex : uniqueVertices){
        vertices.push_back(vertex);
    }
}

void CalculateFaceNormals(Scene& scene)
{
    for (int i = 0; i < scene.meshes.size(); ++i)
    {
        Mesh* mesh = &scene.meshes[i];
        for (Face& face : mesh->faces)
        {
            Vec3f v0 = scene.GetVertex(face.v0_id);
            Vec3f v1 = scene.GetVertex(face.v1_id);
            Vec3f v2 = scene.GetVertex(face.v2_id);
            Vec3f normal = (v1 - v0).cross(v2 - v0);
            face.normal = normal.normalize();
        }
    }
    for (int i = 0; i < scene.triangles.size(); ++i)
    {
        Triangle* triangle = &scene.triangles[i];
        Vec3f v0 = scene.GetVertex(triangle->indices.v0_id);
        Vec3f v1 = scene.GetVertex(triangle->indices.v1_id);
        Vec3f v2 = scene.GetVertex(triangle->indices.v2_id);
        Vec3f normal = (v1 - v0).cross(v2 - v0);
        triangle->indices.normal = normal.normalize();
    }
}