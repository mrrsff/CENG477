#include "Node3D.h"
#include "parser.h"

using namespace parser;
using namespace std;

Node3D::Node3D(vector<Triangle> triangles, int depth)
{
    this->triangles = triangles;
    this->depth = depth;
    this->left = NULL;
    this->right = NULL;
    this->min = Vec3f{0,0,0};
    this->max = Vec3f{0,0,0};
}