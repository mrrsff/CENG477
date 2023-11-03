#include "parser.h"
#include "Ray.h"
#include "HitInfo.h"

using namespace parser;

HitInfo::HitInfo()
{
    this->hitting_ray = Ray();
    this->is_hit = false;
    this->material_id = -1;
    this->face = Face();
    this->normal = Vec3f{0, 0, 0};
    this->t = 0;
    this->hit_point = Vec3f{0, 0, 0};
}

HitInfo::~HitInfo()
{
}
