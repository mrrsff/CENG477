#include <iostream>
#include <vector>
#include "Scene.h"

using namespace std;

/*
    1. Transform vertices to the viewport ---- DONE 
    2. Line Clipping Algorithm
        2.1 Cohen-Sutherland (simple)
        2.2 Liang-Barsky (better)
    3. Polygon Clipping Algorithm
        3.1 Sutherland-Hodgman
    4. Line drawing algorithm
        4.1 Midpoint Algorithm
        4.2 Bresenham's Algorithm
    5. Triangle rasterization
        5.1 Barycentric coordinates
    6. Backface Culling
        6.1 Dot product of the normal and the view vector
    7. Depth Buffer Algorithm

    Notes:
    - Projection transformations: Orthographic and Perspective
    - Clipping: Liang-Barsky
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