#include <vector>

#include "parser.h"
#include "Node3D.h"
#include "treeBuilder.h"

using namespace parser;
using namespace std;

void treeBuilder::boundingBox (Mesh mesh,Scene &scene, Vec3f &min, Vec3f &max){
        min = Vec3f{INFINITY,INFINITY,INFINITY};
        max = Vec3f{-INFINITY,-INFINITY,-INFINITY};
        for(Face face : mesh.faces){
            Vec3f centroid = scene.vertex_data[face.v0_id - 1];
            if(centroid.x < min.x) min.x = centroid.x;
            if(centroid.y < min.y) min.y = centroid.y;
            if(centroid.z < min.z) min.z = centroid.z;
            if(centroid.x > max.x) max.x = centroid.x;
            if(centroid.y > max.y) max.y = centroid.y;
            if(centroid.z > max.z) max.z = centroid.z;
            centroid = scene.vertex_data[face.v1_id - 1];
            if(centroid.x < min.x) min.x = centroid.x;
            if(centroid.y < min.y) min.y = centroid.y;
            if(centroid.z < min.z) min.z = centroid.z;
            if(centroid.x > max.x) max.x = centroid.x;
            if(centroid.y > max.y) max.y = centroid.y;
            if(centroid.z > max.z) max.z = centroid.z;
            centroid = scene.vertex_data[face.v2_id - 1];
            if(centroid.x < min.x) min.x = centroid.x;
            if(centroid.y < min.y) min.y = centroid.y;
            if(centroid.z < min.z) min.z = centroid.z;
            if(centroid.x > max.x) max.x = centroid.x;
            if(centroid.y > max.y) max.y = centroid.y;
            if(centroid.z > max.z) max.z = centroid.z;
    }
}