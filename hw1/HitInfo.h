#include "parser.h"
#include "Ray.h"

using namespace parser;

class HitInfo
{
public:
    bool is_hit;
    int material_id;
    Face face;  
    Vec3f normal;
    float t;
    Vec3f hit_point;
    Ray hitting_ray;
    HitInfo();
    ~HitInfo();
};
