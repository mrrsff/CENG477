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
        //TODO: buildTree function
        Node3D* buildTree(vector<Face> &faces, vector<int> &vertex_ids,Scene &scene, int depth, Vec3f min, Vec3f max);
        void boundingBox (Mesh mesh,Scene &scene, Vec3f &min, Vec3f &max);
        void sortVertices (vector<int> &vertex_ids,Scene &scene, int axis);
        //void uniqueVertices (vector<Face> &faces, vector<int> &vertex_ids);
};