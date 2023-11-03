#pragma once

#include <vector>
#include "parser.h"

using namespace parser;
using namespace std;

class Node3D {
    public:
        Vec3f boundingMin;
        Vec3f boundingMax;
        int depth;
        vector<Face> faces;
        vector<int> vertex_ids;
        Node3D* left;
        Node3D* right;
        Node3D();
};