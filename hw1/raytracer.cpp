#include <iostream>
#include "parser.h"
#include "ppm.h"
#include "Ray.h"

using namespace parser;
enum class ObjectType
{
    SPHERE,
    MESH
};

typedef unsigned char RGB[3];

Vec3f GetRayDirection(Camera camera, int x, int y);
float RaySphereIntersect(Ray ray, Sphere sphere, Scene &scene);

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
                for (Sphere sphere: scene.spheres)
                {
                    float t = RaySphereIntersect(ray, sphere, scene);
                    if (t > 0 && t < tmin)
                    {
                        collision = true;
                        tmin = t;
                        material_id = sphere.material_id;
                    }
                }
                for (Mesh mesh: scene.meshes)
                {
                    float t = -1; // TODO: Calculate t
                    if (t > 0 && t < tmin)
                    {
                        collision = true;
                        tmin = t;
                        material_id = mesh.material_id;
                    }
                }
                for (Triangle triangle: scene.triangles)
                {
                    float t = -1; // TODO: Calculate t
                    if (t > 0 && t < tmin)
                    {
                        collision = true;
                        tmin = t;
                        material_id = triangle.material_id;
                    }
                }
                if(collision)
                {
                    float r = scene.materials[material_id].ambient.x;
                    float g = scene.materials[material_id].ambient.y;
                    float b = scene.materials[material_id].ambient.z;
                    image[i++] = r; // R
                    image[i++] = g; // G
                    image[i++] = b; // B
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
