#include <vector>

#include "parser.h"
#include "Node3D.h"
#include "treeBuilder.h"

using namespace parser;
using namespace std;

treeBuilder::treeBuilder(){

}

treeBuilder::~treeBuilder(){

}

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

Node3D* treeBuilder::buildTree(vector<Face> &faces, vector<int> &vertex_ids,Scene &scene, int depth, Vec3f min, Vec3f max){
    if (vertex_ids.size() == 0){
        // zero element given
        return NULL;
    }
    Node3D* node = new Node3D();
    node->faces = faces;
    node->vertex_ids = vertex_ids;
    node->min = min;
    node->max = max;
    node->depth = depth;
    if (vertex_ids.size() < 6){
        // leaf node
        node->left = NULL;
        node->right = NULL;
        return node;
    }
    sortVertices(vertex_ids,scene,depth);
    vector<int> leftvertices;
    vector<int> rightvertices;
    vector<Face> leftfaces;
    vector<Face> rightfaces;
    int i = 0;
    for(; i < vertex_ids.size() / 2; i++){
        leftvertices.push_back(vertex_ids[i]);
    }
    for(; i < vertex_ids.size(); i++){
        rightvertices.push_back(vertex_ids[i]);
    }
    if(depth % 3 == 0){
        // x axis
        for(Face face : faces){
            //unique distribution is a must
            if(scene.vertex_data[face.v0_id - 1].x < scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].x
                && scene.vertex_data[face.v1_id - 1].x < scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].x 
                && scene.vertex_data[face.v2_id - 1].x < scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].x){
                leftfaces.push_back(face);
            }
            else if(scene.vertex_data[face.v0_id - 1].x >= scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].x 
                && scene.vertex_data[face.v1_id - 1].x >= scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].x 
                && scene.vertex_data[face.v2_id - 1].x >= scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].x){
                rightfaces.push_back(face);
            }
            else {
                // split
                leftfaces.push_back(face);
                rightfaces.push_back(face);
            }
        }
    }
    if(depth % 3 == 1){
        // y axis
        for(Face face : faces){
            //unique distribution is a must
            if(scene.vertex_data[face.v0_id - 1].y < scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].y
                && scene.vertex_data[face.v1_id - 1].y < scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].y 
                && scene.vertex_data[face.v2_id - 1].y < scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].y){
                leftfaces.push_back(face);
            }
            else if(scene.vertex_data[face.v0_id - 1].y >= scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].y 
                && scene.vertex_data[face.v1_id - 1].y >= scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].y 
                && scene.vertex_data[face.v2_id - 1].y >= scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].y){
                rightfaces.push_back(face);
            }
            else {
                // split
                leftfaces.push_back(face);
                rightfaces.push_back(face);
            }
        }
    }
    if(depth % 3 == 2){
        // z axis
        for(Face face : faces){
            //unique distribution is a must
            if(scene.vertex_data[face.v0_id - 1].z < scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].z
                && scene.vertex_data[face.v1_id - 1].z < scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].z 
                && scene.vertex_data[face.v2_id - 1].z < scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].z){
                leftfaces.push_back(face);
            }
            else if(scene.vertex_data[face.v0_id - 1].z >= scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].z 
                && scene.vertex_data[face.v1_id - 1].z >= scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].z 
                && scene.vertex_data[face.v2_id - 1].z >= scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].z){
                rightfaces.push_back(face);
            }
            else {
                // split
                leftfaces.push_back(face);
                rightfaces.push_back(face);
            }
        }
    }
    node->left = buildTree(leftfaces,leftvertices,scene,depth + 1,min,Vec3f{scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].x,max.y,max.z});
    node->right = buildTree(rightfaces,rightvertices,scene,depth + 1,Vec3f{scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1].x,min.y,min.z},max);
    
    return node;
}

void treeBuilder::sortVertices(vector<int> &vertex_ids,Scene &scene, int axis){
    if(axis % 3 == 0){
        // x axis
        for(int i = 0; i < vertex_ids.size(); i++){
            for(int j = i + 1; j < vertex_ids.size(); j++){
                if(scene.vertex_data[vertex_ids[i] - 1].x > scene.vertex_data[vertex_ids[j] - 1].x){
                    int temp = vertex_ids[i];
                    vertex_ids[i] = vertex_ids[j];
                    vertex_ids[j] = temp;
                }
            }
        }
    }
    if(axis % 3 == 1){
        // y axis
        for(int i = 0; i < vertex_ids.size(); i++){
            for(int j = i + 1; j < vertex_ids.size(); j++){
                if(scene.vertex_data[vertex_ids[i] - 1].y > scene.vertex_data[vertex_ids[j] - 1].y){
                    int temp = vertex_ids[i];
                    vertex_ids[i] = vertex_ids[j];
                    vertex_ids[j] = temp;
                }
            }
        }
    }
    if(axis % 3 == 2){
        // z axis
        for(int i = 0; i < vertex_ids.size(); i++){
            for(int j = i + 1; j < vertex_ids.size(); j++){
                if(scene.vertex_data[vertex_ids[i] - 1].z > scene.vertex_data[vertex_ids[j] - 1].z){
                    int temp = vertex_ids[i];
                    vertex_ids[i] = vertex_ids[j];
                    vertex_ids[j] = temp;
                }
            }
        }
    }
}