#include <iostream>
#include "parser.h"
#include "ppm.h"
#include "Ray.h"

using namespace parser;

typedef unsigned char RGB[3];

Vec3f GetRayDirection(Camera camera, int x, int y);

int main(int argc, char* argv[])
{
    // Sample usage for reading an XML scene file
    Scene scene;

    scene.loadFromXml(argv[1]);

    // The code below creates a test pattern and writes
    // it to a PPM file to demonstrate the usage of the
    // ppm_write function.
    //
    // Normally, you would be running your ray tracing
    // code here to produce the desired image.


    const RGB BAR_COLOR[8] =
    {
        { 255, 255, 255 },  // 100% White
        { 255, 255,   0 },  // Yellow
        {   0, 255, 255 },  // Cyan
        {   0, 255,   0 },  // Green
        { 255,   0, 255 },  // Magenta
        { 255,   0,   0 },  // Red
        {   0,   0, 255 },  // Blue
        {   0,   0,   0 },  // Black
    };

    int width = 640, height = 480;
    int columnWidth = width / 8;

    unsigned char* image = new unsigned char [width * height * 3]; // 3 channels per pixel (RGB)

    for (Camera camera : scene.cameras)
    {
        int i = 0; 
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                Ray ray = Ray(camera.position, GetRayDirection(camera, x, y));

                // TODO: Check collision with objects
                bool collision = false;
                float tmin = INFINITY;
                for (Sphere sphere: scene.spheres)
                {
                    
                }
                for (Mesh mesh: scene.meshes)
                {
                    
                }

                image[i++] = scene.background_color.x; // R
                image[i++] = scene.background_color.y; // G
                image[i++] = scene.background_color.z; // B
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
    Vec3f s = q + (u * sv) - (camera.up * sv);
    Vec3f d = s - camera.position;
    return d;
}