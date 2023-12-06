#include <iostream>
#include <vector>
#include "Scene.h"

using namespace std;

//TODO: Start implementations here
/*
    1. Transform vertices to the viewport
    2. Line drawing algorithm
    3. Triangle rasterization
    4. Clipping Algorithm
        4.1 Cohen-Sutherland (simple)
        4.2 Liang-Barsky (better)
    5. Backface Culling

    Notes:
    - The rasterizer should be able to handle multiple objects
    - No lighting, just interpolation of colors
    - Projection transformations: Orthographic and Perspective
    - Clipping: Cohen-Sutherland and Liang-Barsky
    - Backface Culling: Dot product of the normal and the view vector
    - Rasterization: Barycentric coordinates
    - Line drawing: Midpoint Algorithm
    - Wireframe and Solid rendering
    - Depth Buffer Algorithm
    - A makefile should be provided (make rasterizer)
*/

Scene *scene;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Please run the rasterizer as:" << endl
             << "\t./rasterizer <input_file_name>" << endl;
        return 1;
    }
    else
    {
        const char *xmlPath = argv[1];

        scene = new Scene(xmlPath);

        for (int i = 0; i < scene->cameras.size(); i++)
        {
            // initialize image with basic values
            scene->initializeImage(scene->cameras[i]);

            // do forward rendering pipeline operations
            scene->forwardRenderingPipeline(scene->cameras[i]);

            // generate PPM file
            scene->writeImageToPPMFile(scene->cameras[i]);

            // Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
            // Change/remove implementation if necessary.
            scene->convertPPMToPNG(scene->cameras[i]->outputFilename);
        }

        return 0;
    }
}