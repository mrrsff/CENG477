#pragma once

#include <vector>

#include "parser.h"
#include "Node3D.h"

using namespace parser;
using namespace std;

class treeBuilder{
    public:                                    
        treeBuilder();
        ~treeBuilder();
        // Builds the tree recursively
        Node3D* buildTree(vector<Face> faces, vector<int> vertex_ids,Scene &scene, int depth, Vec3f minVertex, Vec3f maxVertex);
        // Finds the bounding box of the given faces
        void findBoundingBox (vector<Face> &faces,Scene &scene, Vec3f &minVertex, Vec3f &maxVertex);
        // Sorts the vertices according to its position on the axis (sorted vertices are stored in vertex_ids)
        void sortVertices (vector<int> &vertex_ids,Scene &scene, int axis);
        //void uniqueVertices (vector<Face> &faces, vector<int> &vertex_ids);
};