#include <iostream>
#include "parser.h"
#include "ppm.h"
#include "Ray.h"

#define EPSILON 0.0001

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
Vec3f GetFaceNormal(Ray ray, float t, Face face, Scene &scene);
Vec3f GetSphereNormal(Ray ray, float t, Sphere sphere, Scene &scene);
Vec3f GetTriangleNormal(Ray ray, float t, Triangle triangle, Scene &scene);
Vec3f GetFaceNormal(Ray ray, float t, Face face, Scene &scene);

float RaySphereIntersect(Ray ray, Sphere sphere, Scene &scene);
float RayTriangleIntersect(Ray ray, Triangle triangle, Scene &scene, Camera &camera);
float RayMeshIntersect(Ray ray, Mesh mesh, Scene &scene, Camera &camera);

Vec3f GetColor(Ray ray, float t, float tmin, int material_id, Vec3f normal, ObjectType object_type, Scene &scene);


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
                        normal = GetSphereNormal(ray, t, sphere, scene);
                    }
                }
                for (Mesh mesh: scene.meshes)
                {
                    //TODO: Bounding Sphere
                    float t = RayMeshIntersect(ray, mesh, scene, camera);
                    if (t > 0 && t < tmin)
                    {
                        collision = true;
                        tmin = t;
                        material_id = mesh.material_id;
                        object_type = ObjectType::MESH;
                        normal = GetFaceNormal(ray, t, mesh.faces[0], scene); // TODO: Calculate normal
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
                        normal = GetTriangleNormal(ray, t, triangle, scene);
                    }
                }
                if(collision)
                {                    
                    Vec3f color = GetColor(ray, tmin, camera.near_distance, material_id, normal, object_type, scene);
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
    float su = (x + 0.5) * 2 / camera.image_width;
    float sv = (y + 0.5) * 2 / camera.image_height;
    Vec3f s = q + (u * su) - (camera.up * sv);
    Vec3f d = s - camera.position;
    return d;
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

float RayMeshIntersect(Ray ray, Mesh mesh, Scene &scene, Camera &camera){
    Vec3f o = ray.getOrigin();
    Vec3f d = ray.getDirection();
    float t = INFINITY;
    bool check = false;
    for(Face face : mesh.faces){
        float temp = RayTriangleIntersect(ray, Triangle{mesh.material_id,face}, scene, camera);
        if (temp < 0) continue;
        if (temp < t){
            t = temp;
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

Vec3f GetSphereNormal(Ray ray, float t, Sphere sphere, Scene &scene)
{
    Vec3f intersection_point = ray.getOrigin() + ray.getDirection() * t;
    Vec3f center = scene.vertex_data[sphere.center_vertex_id - 1];
    Vec3f normal = intersection_point - center;
    return normal;
}

Vec3f GetTriangleNormal(Ray ray, float t, Triangle triangle, Scene &scene)
{
    Vec3f intersection_point = ray.getOrigin() + ray.getDirection() * t;
    Vec3f v0 = scene.vertex_data[triangle.indices.v0_id - 1];
    Vec3f v1 = scene.vertex_data[triangle.indices.v1_id - 1];
    Vec3f v2 = scene.vertex_data[triangle.indices.v2_id - 1];
    Vec3f normal = (v1 - v0).cross(v2 - v0);
    return normal;
}
Vec3f GetFaceNormal(Ray ray, float t, Face face, Scene &scene)
{
    Vec3f intersection_point = ray.getOrigin() + ray.getDirection() * t;
    Vec3f v0 = scene.vertex_data[face.v0_id - 1];
    Vec3f v1 = scene.vertex_data[face.v1_id - 1];
    Vec3f v2 = scene.vertex_data[face.v2_id - 1];
    Vec3f normal = (v1 - v0).cross(v2 - v0);
    return normal;
}

Vec3f GetColor(Ray ray, float t, float tmin, int material_id, Vec3f normal, ObjectType object_type, Scene &scene)
{
    float color_r = 0;
    float color_g = 0;
    float color_b = 0;

    Material material = scene.materials[material_id - 1]; // Material of the object
    Vec3f intersection_point = ray.getOrigin() + ray.getDirection() * t; // Intersection point
    float distanceFromLight = 0;

    for (PointLight light: scene.point_lights)
    {
        Vec3f light_direction = light.position - intersection_point;
        
        // TODO: Check if the intersection point is in shadow
        bool inShadow;


        // Diffuse 
        float distanceFromLight_squared = light_direction.length() * light_direction.length();
        float cos = light_direction.dot(normal);
        if (cos < 0)
            cos = 0;

        color_r += material.diffuse.x * light.intensity.x * cos / distanceFromLight_squared;
        color_g += material.diffuse.y * light.intensity.y * cos / distanceFromLight_squared;
        color_b += material.diffuse.z * light.intensity.z * cos / distanceFromLight_squared;


        // Specular
        /*
        Vec3f h = (light_direction + ray.getDirection()) / (light_direction + ray.getDirection()).length();
        float cosa = h.dot(normal);
        if (cosa < 0)
            cosa = 0;
        
        float specular_r = material.specular.x * light.intensity.x * pow(cosa, material.phong_exponent) / distanceFromLight_squared;
        float specular_g = material.specular.y * light.intensity.y * pow(cosa, material.phong_exponent) / distanceFromLight_squared;
        float specular_b = material.specular.z * light.intensity.z * pow(cosa, material.phong_exponent) / distanceFromLight_squared;
        */
    }

    // Ambient
    color_r += material.ambient.x * scene.ambient_light.x;
    color_g += material.ambient.y * scene.ambient_light.y;
    color_b += material.ambient.z * scene.ambient_light.z;

    if (object_type == ObjectType::SPHERE)
    {
    }
    else if (object_type == ObjectType::MESH)
    {
    }
    else if (object_type == ObjectType::TRIANGLE)
    {
    }
    if(color_r > 255) color_r = 255;
    if(color_g > 255) color_g = 255;
    if(color_b > 255) color_b = 255;
    
    return Vec3f{color_r, color_g, color_b};
}
