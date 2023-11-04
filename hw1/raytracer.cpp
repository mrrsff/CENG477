#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <set>
#include "parser.h"
#include "ppm.h"
#include "Ray.h"
#include "HitInfo.h"
#include "Node3D.h"
#include "treeBuilder.h"

#define EPSILON 0.001

using namespace parser;
using namespace std;

enum class ObjectType
{
    SPHERE,
    MESH,
    TRIANGLE
};

typedef unsigned char RGB[3];

float determinant (Vec3f a, Vec3f b, Vec3f c);

Vec3f GetRayDirection(Camera camera, int x, int y);
Vec3f GetFaceNormal(Ray ray, Face face, Scene &scene);
Vec3f GetSphereNormal(Ray ray, Vec3f intersection_point, Sphere sphere, Scene &scene);
Ray Reflect(Vec3f normal, Vec3f incoming_direction, Vec3f intersection_point);

float RaySphereIntersect(Ray ray, Sphere sphere, Scene &scene);
float RayFaceIntersect(Ray ray, Triangle triangle, Scene &scene);
float RayMeshIntersect(Ray ray, Mesh mesh, Node3D *tree, Scene &scene, Face &face_out);
float RayTreeIntersect(Ray ray, Node3D *tree, Scene &scene, Face& face_out);
bool RayBoundingBoxIntersect(Ray ray, Vec3f minVertex, Vec3f maxVertex, Scene& scene);

HitInfo RayIntersect(Ray ray, Camera &camera, Scene &scene, Node3D **meshTrees);
Vec3i GetColor(HitInfo hit_info, Camera &camera, Scene &scene, int depth, Node3D **meshTrees);

void findUniqueVertices(vector<Face> &faces, vector<int> &vertices);

void PrintTree(Node3D* tree, Scene &scene, int mt_id = -1)
{
    if(tree == nullptr)
        return;
    cout << "Depth: " << tree->depth << endl;
    cout << "Bounding Min: " << tree->boundingMin.x << " " << tree->boundingMin.y << " " << tree->boundingMin.z << endl;
    cout << "Bounding Max: " << tree->boundingMax.x << " " << tree->boundingMax.y << " " << tree->boundingMax.z << endl;
    cout << "Material ID: " << mt_id << endl;
    cout << "Faces: " << endl;
    for(Face face : tree->faces){
        cout << face.v0_id << " " << face.v1_id << " " << face.v2_id << endl;
    }
    cout << "Vertices: " << endl;
    for(int vertex : tree->vertex_ids){
        cout << vertex << " position: " << scene.vertex_data[vertex - 1].x << " " << scene.vertex_data[vertex - 1].y << " " << scene.vertex_data[vertex - 1].z << endl;
    }
    cout << endl;
    PrintTree(tree->left,scene,mt_id);
    PrintTree(tree->right,scene,mt_id);
}

int main(int argc, char* argv[])
{
    Scene scene;

    scene.loadFromXml(argv[1]);

    Node3D* meshTrees[scene.meshes.size()];
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

        unsigned char* image = new unsigned char [width * height * 3]; // 3 channels per pixel (RGB)
        
        int i = 0; 
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                Ray ray = Ray(camera.position, GetRayDirection(camera, x, y));
                HitInfo hit_info = RayIntersect(ray, camera, scene, meshTrees);
                if(hit_info.is_hit)
                {                    
                    auto color = GetColor(hit_info, camera, scene, 0, meshTrees);
                    image[i++] = color.x; // R
                    image[i++] = color.y; // G
                    image[i++] = color.z; // B
                }
                else
                {
                    image[i++] = scene.background_color.x; // R
                    image[i++] = scene.background_color.y; // G
                    image[i++] = scene.background_color.z; // B
                }
            }
        }

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

float RayFaceIntersect(Ray ray, Face face, Scene &scene){
    Vec3f o = ray.getOrigin();
    Vec3f d = ray.getDirection();
    
    Vec3f a = scene.vertex_data[face.v0_id - 1];
    Vec3f b = scene.vertex_data[face.v1_id - 1];
    Vec3f c = scene.vertex_data[face.v2_id - 1];

    Vec3f normal = (b - a).cross(c - a).normalize(); // Backface culling
    if(normal.dot(d) > 0) return -1;

    float A = determinant(a-b,a-c,d);
    float beta = determinant(a-o,a-c,d)/A;
    float gamma = determinant(a-b,a-o,d)/A;
    float t = determinant(a-b,a-c,a-o)/A;

    //float tmax = camera.near_distance;
    //TODO: Tmin and Tmax (?)

    if(beta >= 0 && gamma >= 0 && beta + gamma <= 1){
        return t;
    }
    return -1;
}

float RayMeshIntersect(Ray ray, Mesh mesh, Node3D *tree, Scene &scene, Face &face_out)
{   
    return RayTreeIntersect(ray, tree, scene, face_out);
}
float RayTreeIntersect(Ray ray, Node3D *node, Scene &scene, Face& face_out)
{
    if(node == nullptr)
        return -1;
    // First check the bounding box, then if node has children, check them else check the faces
    if(!RayBoundingBoxIntersect(ray, node->boundingMin, node->boundingMax, scene))
        return -1;

    if(node->left != nullptr || node->right != nullptr)
    {
        float t1 = RayTreeIntersect(ray, node->left, scene, face_out);
        float t2 = RayTreeIntersect(ray, node->right, scene, face_out);
        
        if (t1 > 0 && t2 > 0) return t1 < t2 ? t1 : t2;
        else if (t1 > 0) return t1;
        else if (t2 > 0) return t2;
        else return -1;
    }
    else
    {
        float tmin = INFINITY;
        for(Face face : node->faces){
            float t = RayFaceIntersect(ray, face, scene);
            if(t > 0 && t < tmin){
                tmin = t;
                face_out = face;
            }
        }
        return tmin;
    }
    
}
bool RayBoundingBoxIntersect(Ray ray, Vec3f minVertex, Vec3f maxVertex, Scene& scene)
{
    Vec3f origin = ray.getOrigin();
    Vec3f direction = ray.getDirection();

    float tmin = (minVertex.x - origin.x) / direction.x;
    float tmax = (maxVertex.x - origin.x) / direction.x;

    if (tmin > tmax)
        swap(tmin, tmax);
    
    float tymin = (minVertex.y - origin.y) / direction.y;
    float tymax = (maxVertex.y - origin.y) / direction.y;

    if (tymin > tymax)
        swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;
    
    if (tymin > tmin)
        tmin = tymin;
    
    if (tymax < tmax)
        tmax = tymax;
    
    float tzmin = (minVertex.z - origin.z) / direction.z;
    float tzmax = (maxVertex.z - origin.z) / direction.z;

    if (tzmin > tzmax)
        swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    return true; // This line should be here, outside the last if block.
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
Vec3f GetFaceNormal(Ray ray, Face face, Scene &scene)
{
    Vec3f v0 = scene.vertex_data[face.v0_id - 1];
    Vec3f v1 = scene.vertex_data[face.v1_id - 1];
    Vec3f v2 = scene.vertex_data[face.v2_id - 1];
    Vec3f normal = (v1 - v0).cross(v2 - v0);
    return normal.normalize();
}

void PrintHitInfo(HitInfo hit_info)
{
    /* cout << "Hit Info: " << endl; */
    cout << "Material ID: " << hit_info.material_id << endl;
    /* cout << "Normal: " << hit_info.normal.x << " " << hit_info.normal.y << " " << hit_info.normal.z << endl;
    cout << "Hit Point: " << hit_info.hit_point.x << " " << hit_info.hit_point.y << " " << hit_info.hit_point.z << endl; */
}

HitInfo RayIntersect(Ray ray, Camera &camera, Scene &scene, Node3D **meshTrees)
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
        float t = RayMeshIntersect(ray, mesh, meshTrees[mesh.mesh_id] , scene, face);
        if (t > 0 && t < tmin)
        {
            tmin = t;
            info.is_hit = true;
            info.t = t;
            info.material_id = mesh.material_id;
            info.hit_point = ray.getOrigin() + ray.getDirection() * t;
            info.normal = GetFaceNormal(ray, face, scene);
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
        float t = RayFaceIntersect(ray, triangle.indices, scene);
        if (t > 0 && t < tmin)
        {
            tmin = t;
            info.is_hit = true;
            info.t = t;
            info.material_id = triangle.material_id;
            info.hit_point = ray.getOrigin() + ray.getDirection() * t;
            info.normal = GetFaceNormal(ray, triangle.indices, scene);
            //hitting ray missing
            info.hitting_ray = ray;
        }
    }
    return info;
}

Vec3i GetColor(HitInfo hit_info, Camera &camera, Scene &scene, int depth, Node3D **meshTrees)
{
    if(depth > scene.max_recursion_depth)
        return Vec3i{0,0,0};

    int material_id = hit_info.material_id;
    Vec3f intersection_point = hit_info.hit_point;
    Vec3f normal = hit_info.normal;
    Ray ray = hit_info.hitting_ray;

    float color_r = 0;
    float color_g = 0;
    float color_b = 0;
    
    Material material = scene.materials[material_id - 1]; // Material of the object

    float distanceFromLight = 0;

    for (PointLight light: scene.point_lights)
    {
        Vec3f light_direction = light.position - intersection_point;
        Vec3f l = light_direction.normalize();
        Vec3f v = ray.getDirection().normalize() * -1;
        
        // Shadow
        auto inShadow = RayIntersect(Ray(intersection_point + (normal * scene.shadow_ray_epsilon), light_direction), camera, scene, meshTrees);
        if(inShadow.is_hit == true && inShadow.t > 0 && inShadow.t < 1) continue;
        
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
        Ray reflection_ray = Reflect(normal, ray.getDirection(), intersection_point);
        auto reflection_info = RayIntersect(reflection_ray, camera, scene, meshTrees);
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

