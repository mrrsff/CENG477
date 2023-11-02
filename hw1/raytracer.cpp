#define EPSILON 0.001
#define NUM_THREADS 64

#include <iostream>
#include <cmath>
#include "parser.h"
#include "ppm.h"
#include "Ray.h"
#include "HitInfo.h"
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>

using namespace parser;
using namespace std;

typedef unsigned char* Image;

float determinant (Vec3f a, Vec3f b, Vec3f c);

Vec3f GetRayDirection(Camera camera, int x, int y);
Vec3f GetFaceNormal(Ray ray, Face face, Scene &scene);
Vec3f GetSphereNormal(Ray ray, Vec3f intersection_point, Sphere sphere, Scene &scene);
Ray Reflect(Vec3f normal, Vec3f incoming_direction, Vec3f intersection_point);

float RaySphereIntersect(Ray ray, Sphere sphere, Scene &scene);
float RayTriangleIntersect(Ray ray, Triangle triangle, Scene &scene, Camera &camera);
float RayMeshIntersect(Ray ray, Mesh mesh, Scene &scene, Camera &camera, Face &face_out);

HitInfo RayIntersect(Ray ray, Camera &camera, Scene &scene);
Vec3i GetColor(HitInfo hit_info, Camera &camera, Scene &scene, int depth);

std::mutex imageMutex;
int numThreads = NUM_THREADS;

void RenderImage(Camera camera, Scene &scene,Image &image, int startRow, int endRow)
{
    int width = camera.image_width;
    for (int y = startRow; y < endRow; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            // Logic
            Ray ray = Ray(camera.position, GetRayDirection(camera, x, y));
            HitInfo hit_info = RayIntersect(ray, camera, scene);
            
            int pixelIndex = (y * width + x) * 3; // Index in the image buffer

            if(hit_info.is_hit)
            {                    
                Vec3i color = GetColor(hit_info, camera, scene, 0);
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
    Scene scene;

    scene.loadFromXml(argv[1]);

    
    for (Camera camera : scene.cameras)
    {
        int width = camera.image_width;
        int height = camera.image_height;

        Image image = new unsigned char [width * height * 3]; // 3 channels per pixel (RGB)

        int numRowsPerThread = height / NUM_THREADS;

        std::vector<std::thread> threads;

        for (int i = 0; i < numThreads; ++i)
        {
            int startRow = i * numRowsPerThread;
            int endRow = (i == numThreads - 1) ? height : (startRow + numRowsPerThread);

            threads.emplace_back(RenderImage, camera, std::ref(scene), std::ref(image), startRow, endRow);
        }

        // Wait for all threads to finish
        for (std::thread &thread : threads)
        {
            thread.join();
        }

        /* int i = 0;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x) 
            {
                Ray ray = Ray(camera.position, GetRayDirection(camera, x, y));
                HitInfo hit_info = RayIntersect(ray, camera, scene);
                if(hit_info.is_hit)
                {                    
                    Vec3i color = GetColor(hit_info, camera, scene, 0);
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
        } */

        std::unique_lock<std::mutex> lock(imageMutex);
        write_ppm(camera.image_name.c_str(), image, width, height);
        cout << "Image " << camera.image_name << " is written." << endl;
    }
}


Vec3f GetRayDirection(Camera camera, int x, int y)
{
    // v = Camera up vector
    // -w = Camera gaze vector
    // u = cross(v,-w) 

    Vec3f u = camera.up.cross(camera.gaze * -1);
    Vec3f m = camera.position + camera.gaze * camera.near_distance;
    Vec3f q = m + (u * -1) + (camera.up * 1);
    float l = camera.near_plane.x;
    float r = camera.near_plane.y;
    float b = camera.near_plane.z;
    float t = camera.near_plane.w;
    float su = (x + 0.5) * (r-l) / camera.image_width;
    float sv = (y + 0.5) * (t-b) / camera.image_height;
    Vec3f s = q + (u * su) - (camera.up * sv);
    Vec3f d = s - camera.position;
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

float RayTriangleIntersect(Ray ray, Triangle triangle, Scene &scene, Camera &camera){
    Vec3f o = ray.getOrigin();
    Vec3f d = ray.getDirection();
    
    Vec3f a = scene.vertex_data[triangle.indices.v0_id - 1];
    Vec3f b = scene.vertex_data[triangle.indices.v1_id - 1];
    Vec3f c = scene.vertex_data[triangle.indices.v2_id - 1];

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

float RayMeshIntersect(Ray ray, Mesh mesh, Scene &scene, Camera &camera, Face &face_out){
    Vec3f o = ray.getOrigin();
    Vec3f d = ray.getDirection();
    float t = INFINITY;
    bool check = false;
    for(Face face : mesh.faces){
        float temp = RayTriangleIntersect(ray, Triangle{mesh.material_id,face}, scene, camera);
        if (temp < 0) continue;
        if (temp < t){
            t = temp;
            face_out = face;
            check = true;
        }
    }
    //TODO: remove check
    if(check) return t;
    return -1;
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

HitInfo RayIntersect(Ray ray, Camera &camera, Scene &scene)
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
        //TODO: Bounding Sphere
        Face face;
        float t = RayMeshIntersect(ray, mesh, scene, camera, face);
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
        }
    }
    for (Triangle triangle: scene.triangles)
    {
        float t = RayTriangleIntersect(ray, triangle, scene, camera);
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

Vec3i GetColor(HitInfo hit_info, Camera &camera, Scene &scene, int depth)
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
        
        // TODO: Check if the intersection point is in shadow
        auto inShadow = RayIntersect(Ray(intersection_point + (light_direction * scene.shadow_ray_epsilon), light_direction), camera, scene);
        if(inShadow.is_hit == true ) continue;
        
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
        Vec3f deneme = ray.getDirection();
        
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
    
    Ray reflection_ray = Reflect(normal, ray.getDirection(), intersection_point);
    auto reflection_info = RayIntersect(reflection_ray, camera, scene);
    if (reflection_info.is_hit)
    {
        auto reflection_color = GetColor(reflection_info, camera, scene, depth + 1);
        color_r += material.mirror.x * reflection_color.x;
        color_g += material.mirror.y * reflection_color.y;
        color_b += material.mirror.z * reflection_color.z; 
    }

    if(color_r > 255) color_r = 255;
    if(color_g > 255) color_g = 255;
    if(color_b > 255) color_b = 255;

    int icolor_r = (int) round(color_r);
    int icolor_g = (int) round(color_g);
    int icolor_b = (int) round(color_b);
    
    return Vec3i{icolor_r, icolor_g, icolor_b};
}

