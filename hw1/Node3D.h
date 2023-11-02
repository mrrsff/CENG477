#pragma once

#include <vector>

#include "parser.h"

using namespace parser;
using namespace std;

class Node3D {
    public:
        Vec3f min;
        Vec3f max;
        int depth;
        vector<Face> faces;
        vector<Vec3f> vertices;
        Node3D* left;
        Node3D* right;

        Node3D(vector<Triangle> triangles, int depth);
        
};