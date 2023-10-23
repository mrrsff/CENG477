#include <iostream>
#include "parser.h"
#include "ppm.h"
#include "Ray.h"

#define EPSILON 0.001

using namespace parser;
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
Vec3f CreateMirrorRay(Vec3f normal, Vec3f ray);

float RaySphereIntersect(Ray ray, Sphere sphere, Scene &scene);
float RayTriangleIntersect(Ray ray, Triangle triangle, Scene &scene, Camera &camera);
float RayMeshIntersect(Ray ray, Mesh mesh, Scene &scene, Camera &camera, Face &face_out);

bool RayIntersect(Ray ray, Camera &camera, Scene &scene);
Vec3f GetColor(Ray ray, Vec3f intersection_point, int material_id, Vec3f normal, Camera &camera, Scene &scene, int depth);


int main(int argc, char* argv[])
{
    Scene scene;

    scene.loadFromXml(argv[1]);

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
                bool collision = false;
                float tmin = INFINITY;
                int material_id = 0;
                ObjectType object_type;
                Vec3f normal;
                Vec3f intersection_point;
                for (Sphere sphere: scene.spheres)
                {
                    // TODO: faster intersection test with dot product and radius !!!

                    float t = RaySphereIntersect(ray, sphere, scene);
                    if (t > 0 && t < tmin)
                    {
                        collision = true;
                        tmin = t;
                        material_id = sphere.material_id;
                        object_type = ObjectType::SPHERE;
                        intersection_point = ray.getOrigin() + ray.getDirection() * t;
                        normal = GetSphereNormal(ray, intersection_point, sphere, scene);
                    }
                }
                for (Mesh mesh: scene.meshes)
                {
                    //TODO: Bounding Sphere
                    Face face;
                    float t = RayMeshIntersect(ray, mesh, scene, camera, face);
                    if (t > 0 && t < tmin)
                    {
                        collision = true;
                        tmin = t;
                        material_id = mesh.material_id;
                        object_type = ObjectType::MESH;
                        intersection_point = ray.getOrigin() + ray.getDirection() * t;
                        normal = GetFaceNormal(ray, face, scene);
                    }
                }
                for (Triangle triangle: scene.triangles)
                {
                    float t = RayTriangleIntersect(ray, triangle, scene, camera);
                    if (t > 0 && t < tmin)
                    {
                        collision = true;
                        tmin = t;
                        material_id = triangle.material_id;
                        object_type = ObjectType::TRIANGLE;
                        intersection_point = ray.getOrigin() + ray.getDirection() * t;
                        normal = GetFaceNormal(ray, triangle.indices, scene);
                    }
                }
                if(collision)
                {                    
                    Vec3f color = GetColor(ray, intersection_point, material_id, normal, camera, scene, 0);
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
Vec3f CreateMirrorRay(Vec3f normal, Vec3f ray)
{
    return ray - (normal * 2 * ray.dot(normal));
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

bool RayIntersect(Ray ray, Camera &camera, Scene &scene)
{
    for (Sphere sphere: scene.spheres)
    {
        float t = RaySphereIntersect(ray, sphere, scene);
        if (t > 0 && t < camera.near_distance)
            return true;
    }
    for (Mesh mesh: scene.meshes)
    {
        Face face;
        float t = RayMeshIntersect(ray, mesh, scene, camera, face);
        if (t > 0 && t < camera.near_distance)
            return true;
    }
    for (Triangle triangle: scene.triangles)
    {
        float t = RayTriangleIntersect(ray, triangle, scene, camera);
        if (t > 0 && t < camera.near_distance)
            return true;
    }
    return false;
}

Vec3f GetColor(Ray ray, Vec3f intersection_point, int material_id, Vec3f normal, Camera &camera, Scene &scene, int depth)
{
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
        bool inShadow = RayIntersect(Ray(intersection_point + (light_direction * scene.shadow_ray_epsilon), light_direction), camera, scene);
        if(inShadow) continue;

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
    Vec3f dir = CreateMirrorRay(normal, ray.getDirection());
    Ray reflection_ray = Ray(intersection_point, dir);
    Vec3f reflection_color = Vec3f{0,0,0};
    if (depth < scene.max_recursion_depth && RayIntersect(reflection_ray, camera, scene))
    {
        reflection_color = GetColor(reflection_ray, intersection_point, material_id, normal, camera, scene, depth + 1);
    }

    color_r += material.mirror.x * reflection_color.x;
    color_g += material.mirror.y * reflection_color.y;
    color_b += material.mirror.z * reflection_color.z;

    if(color_r > 255) color_r = 255;
    if(color_g > 255) color_g = 255;
    if(color_b > 255) color_b = 255;
    
    return Vec3f{color_r, color_g, color_b};
}

