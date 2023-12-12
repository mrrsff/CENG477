#ifndef __SCALING_H__
#define __SCALING_H__
#include <iostream>
#include "Matrix4.h"

class Scaling
{
public:
    int scalingId;
    double sx, sy, sz;

    Scaling();
    Scaling(int scalingId, double sx, double sy, double sz);
    Matrix4 getScalingMatrix();
    friend std::ostream &operator<<(std::ostream &os, const Scaling &s);
};

#endif