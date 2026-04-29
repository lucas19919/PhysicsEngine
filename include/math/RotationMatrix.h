#pragma once
#include <cmath>

#include "Vec2.h"

struct RotMatrix
{
    float cos;
    float sin;

    RotMatrix(float angle)
    {
        cos = std::cos(angle);
        sin = std::sin(angle);
    }

    Vec2 Rotate(const Vec2& v) const
    {
        return Vec2(cos * v.x - sin * v.y, sin * v.x + cos * v.y);
    }
};