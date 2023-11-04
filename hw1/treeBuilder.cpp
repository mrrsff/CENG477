#include <vector>
#include <algorithm> 

#include "parser.h"
#include "Node3D.h"
#include "treeBuilder.h"

using namespace parser;
using namespace std;

treeBuilder::treeBuilder(){

}

treeBuilder::~treeBuilder(){

}

void treeBuilder::findBoundingBox (vector<Face> &faces,Scene &scene, Vec3f &minVertex, Vec3f &maxVertex){
    minVertex = Vec3f{INFINITY,INFINITY,INFINITY};
    maxVertex = Vec3f{-INFINITY,-INFINITY,-INFINITY};
    vector<Vec3f> vertices;
    for(Face face : faces){
        vertices.push_back(scene.vertex_data[face.v0_id - 1]);
        vertices.push_back(scene.vertex_data[face.v1_id - 1]);
        vertices.push_back(scene.vertex_data[face.v2_id - 1]);
    }
    for(Vec3f vertex : vertices){
        if(vertex[0] < minVertex[0]){
            minVertex.x = vertex[0];
        }
        if(vertex[1] < minVertex[1]){
            minVertex.y = vertex[1];
        }
        if(vertex[2] < minVertex[2]){
            minVertex.z = vertex[2];
        }
        if(vertex[0] > maxVertex[0]){
            maxVertex.x = vertex[0];
        }
        if(vertex[1] > maxVertex[1]){
            maxVertex.y = vertex[1];
        }
        if(vertex[2] > maxVertex[2]){
            maxVertex.z = vertex[2];
        }
    }
    if(minVertex[0] == INFINITY){
        minVertex.x = 0;
    }
    if(minVertex[1] == INFINITY){
        minVertex.y = 0;
    }
    if(minVertex[2] == INFINITY){
        minVertex.z = 0;
    }
    if(maxVertex[0] == -INFINITY){
        maxVertex.x = 0;
    }
    if(maxVertex[1] == -INFINITY){
        maxVertex.y = 0;
    }
    if(maxVertex[2] == -INFINITY){
        maxVertex.z = 0;
    }
}

Node3D* treeBuilder::buildTree(vector<Face> &faces, vector<int> &vertex_ids,Scene &scene, int depth, Vec3f minVertex, Vec3f maxVertex){
    if (vertex_ids.size() == 0){
        // zero element given
        return NULL;
    }

    Node3D* node = new Node3D();
    node->faces = faces;
    node->vertex_ids = vertex_ids;
    node->boundingMin = minVertex;
    node->boundingMax = maxVertex;
    node->depth = depth;

    if(vertex_ids.size() < 5){
        node->left = NULL;
        node->right = NULL;
        return node;
    }

    sortVertices(vertex_ids, scene, depth % 3); // Sorts the array in place. Sorts the vertices according to its position on the axis

    vector<int> leftVertices = vector<int>(vertex_ids.begin(), vertex_ids.begin() + vertex_ids.size() / 2);
    vector<int> rightVertices = vector<int>(vertex_ids.begin() + vertex_ids.size() / 2, vertex_ids.end());
    vector<Face> leftFaces;
    vector<Face> rightFaces;
    
    auto middle = scene.vertex_data[vertex_ids[vertex_ids.size() / 2] - 1];
    int axis = depth % 3;
    for(Face face : faces)
    {
        auto v0 = scene.vertex_data[face.v0_id - 1];
        auto v1 = scene.vertex_data[face.v1_id - 1];
        auto v2 = scene.vertex_data[face.v2_id - 1];

        // A face can be in both left and right nodes because of the way we split the vertices, but this is not a problem
        if(v0[axis] <= middle[axis] || v1[axis] <= middle[axis] || v2[axis] <= middle[axis])
        {
            leftFaces.push_back(face);
        }
        if(v0[axis] >= middle[axis] || v1[axis] >= middle[axis] || v2[axis] >= middle[axis])
        {
            rightFaces.push_back(face);
        }
    }
    Vec3f leftMin;
    Vec3f leftMax;
    Vec3f rightMin;
    Vec3f rightMax;
    findBoundingBox(leftFaces, scene, leftMin, leftMax);
    findBoundingBox(rightFaces, scene, rightMin, rightMax);
    node->left = buildTree(leftFaces, leftVertices, scene, depth + 1, leftMin, leftMax);
    node->right = buildTree(rightFaces, rightVertices, scene, depth + 1, rightMin, rightMax);
    
    return node;
}

void treeBuilder::sortVertices(vector<int> &vertex_ids,Scene &scene, int axis){

    /* sort(vertex_ids.begin(), vertex_ids.end(), [&scene, &axis](int a, int b) { 
        return scene.vertex_data[a - 1][axis] < scene.vertex_data[b - 1][axis];
    }); */
    for(int i = 0; i < vertex_ids.size(); i++){
        for(int j = i + 1; j < vertex_ids.size(); j++){
            if(scene.vertex_data[vertex_ids[i] - 1][axis] > scene.vertex_data[vertex_ids[j] - 1][axis]){
                int temp = vertex_ids[i];
                vertex_ids[i] = vertex_ids[j];
                vertex_ids[j] = temp;
            }
        }
    }
}