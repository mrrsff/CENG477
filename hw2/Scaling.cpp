#include <iomanip>
#include "Scaling.h"
#include "Helpers.h"
#include "Matrix4.h"
#include <cmath>

Scaling::Scaling() {
    this->scalingId = -1;
    this->sx = 0;
    this->sy = 0;
    this->sz = 0;
}


Matrix4 getIdentityMatrix();

Matrix4 getIdentityMatrix();
Scaling::Scaling(int scalingId, double sx, double sy, double sz)
{
    this->scalingId = scalingId;
    this->sx = sx;
    this->sy = sy;
    this->sz = sz;
}
Matrix4 Scaling::getScalingMatrix()
{
    double m_val[4][4] = {{sx, 0, 0, 0},
                          {0, sy, 0, 0},
                          {0, 0, sz, 0},
                          {0, 0, 0, 1}};

    return Matrix4(m_val);
}

std::ostream &operator<<(std::ostream &os, const Scaling &s)
{
    os << std::fixed << std::setprecision(3) << "Scaling " << s.scalingId << " => [" << s.sx << ", " << s.sy << ", " << s.sz << "]";

    return os;
}
