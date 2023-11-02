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
        Node3D* buildTree(vector<Triangle> triangles, int depth);
        void boundingBox (Mesh mesh,Scene &scene, Vec3f &min, Vec3f &max);

};