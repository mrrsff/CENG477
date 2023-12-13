#include <cmath>
#include <iomanip>
#include "Rotation.h"
#include "Helpers.h"
#include "Matrix4.h"
#include "Vec3.h"

#define M_PI 3.14159265358979323846

Rotation::Rotation() {
    this->rotationId = -1;
    this->angle = 0;
    this->ux = 0;
    this->uy = 0;
    this->uz = 0;
}

Rotation::Rotation(int rotationId, double angle, double x, double y, double z)
{
    this->rotationId = rotationId;
    this->angle = angle;
    this->ux = x;
    this->uy = y;
    this->uz = z;
}
// Find rotation matrix around a unit vector with given angle.
Matrix4 Rotation::getRotationMatrix()
{
    /* Steps:
        1. Set smallest value of u to 0. (absolutes of u.x, u.y, u.z)
        2. Swap the other two values of u, negating one of them. This is v.
        3. Set w to be the cross product of u and v.
        4. Normalize v, and w. (u is already normalized)
        5. Create a matrix4 with u, v, w as the first three columns, fourth column is {0, 0, 0, 1}.
        6. Transpose the matrix.
        7. Apply the rotation matrix.
        8. Transpose the matrix again.
    */
    Vec3 u = Vec3(this->ux, this->uy, this->uz);
    Vec3 v, w;
    double absUx = std::abs(u.x);
    double absUy = std::abs(u.y);
    double absUz = std::abs(u.z);
    if (absUx <= absUy && absUx <= absUz)
    {
        v = Vec3(0, -u.z, u.y);
    }
    else if (absUy <= absUx && absUy <= absUz)
    {
        v = Vec3(-u.z, 0, u.x);
    }
    else
    {
        v = Vec3(-u.y, u.x, 0);
    }
    w = crossProductVec3(u, v);
    v = normalizeVec3(v);
    w = normalizeVec3(w);
    double m_val[4][4] = {{u.x, u.y, u.z, 0},
                          {v.x, v.y, v.z, 0},
                          {w.x, w.y, w.z, 0},
                          {0, 0, 0, 1}};
    Matrix4 m = Matrix4(m_val);

    Matrix4 mT = m.transpose();
    // Rotate around x by angle.
    double rot_val[4][4] = {{1, 0, 0, 0},
                      {0, cos(this->angle * M_PI / 180), -sin(this->angle * M_PI / 180), 0},
                      {0, sin(this->angle * M_PI / 180), cos(this->angle * M_PI / 180), 0},
                      {0, 0, 0, 1}};
    Matrix4 rot = Matrix4(rot_val);
    rot = rot * m;
    Matrix4 result = mT * rot;
    return mT * rot;

}

std::ostream &operator<<(std::ostream &os, const Rotation &r)
{
    os << std::fixed << std::setprecision(3) << "Rotation " << r.rotationId << " => [angle=" << r.angle << ", " << r.ux << ", " << r.uy << ", " << r.uz << "]";
    return os;
}