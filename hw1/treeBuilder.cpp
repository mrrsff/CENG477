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
        for(int i = 0; i < 3; i++){
            if(vertex[i] < minVertex[i]){
                minVertex[i] = vertex[i];
            }
            if(vertex[i] > maxVertex[i]){
                maxVertex[i] = vertex[i];
            }
        }
    }
    for(int i = 0; i < 3; i++){ // To avoid the case where minVertex == maxVertex
        if(minVertex[i] == maxVertex[i]){
            minVertex[i] -= 0.01;
            maxVertex[i] += 0.01;
        }
    }
}

Node3D* treeBuilder::buildTree(vector<Face> faces, vector<int> vertex_ids,Scene &scene, int depth, Vec3f minVertex, Vec3f maxVertex){
    if (vertex_ids.size() == 0){
        // zero element given
        return nullptr;
    }

    Node3D* node = new Node3D();
    node->faces = faces;
    node->vertex_ids = vertex_ids;
    node->boundingMin = minVertex;
    node->boundingMax = maxVertex;
    node->depth = depth;

    int axis = depth % 3;
    sortVertices(vertex_ids, scene, axis); // Sorts the array in place. Sorts the vertices according to its position on the axis
    
    int middleIndex = vertex_ids.size() / 2;
    auto middle = scene.GetVertex(vertex_ids[middleIndex]);
    node->vertex = middle;

    if(vertex_ids.size() < 128){
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }

    vector<int> leftVertices = vector<int>(vertex_ids.begin(), vertex_ids.begin() + vertex_ids.size() / 2 + 1);
    vector<int> rightVertices = vector<int>(vertex_ids.begin() + vertex_ids.size() / 2, vertex_ids.end());
    vector<Face> leftFaces = vector<Face>();
    vector<Face> rightFaces = vector<Face>();
    
    for(Face face : faces)
    {
        auto v0 = scene.vertex_data[face.v0_id - 1];
        auto v1 = scene.vertex_data[face.v1_id - 1];
        auto v2 = scene.vertex_data[face.v2_id - 1];

        // A face can be in both left and right nodes because of the way we split the vertices, but this is not a problem
        if(v0[axis] <= middle[axis] && v1[axis] <= middle[axis] && v2[axis] <= middle[axis]){
            leftFaces.push_back(face);
        }
        if(v0[axis] >= middle[axis] && v1[axis] >= middle[axis] && v2[axis] >= middle[axis]){
            rightFaces.push_back(face);
        }
        else{
            leftFaces.push_back(face);
            rightFaces.push_back(face);
        }
    }
    Vec3f leftMin = minVertex;
    Vec3f leftMax = maxVertex;
    leftMax[axis] = middle[axis]; 
    Vec3f rightMin = minVertex;
    rightMin[axis] = middle[axis];
    Vec3f rightMax = maxVertex;
    /* findBoundingBox(leftFaces, scene, leftMin, leftMax);
    findBoundingBox(rightFaces, scene, rightMin, rightMax); */
    node->left = buildTree(leftFaces, leftVertices, scene, depth + 1, leftMin, leftMax);
    node->right = buildTree(rightFaces, rightVertices, scene, depth + 1, rightMin, rightMax);
    
    return node;
}

void treeBuilder::sortVertices(vector<int> &vertex_ids,Scene &scene, int axis){
    sort(vertex_ids.begin(), vertex_ids.end(), [&scene, &axis](int a, int b) { 
        return scene.vertex_data[a - 1][axis] < scene.vertex_data[b - 1][axis];
    });
}