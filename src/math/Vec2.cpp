#include "math/Vec2.h"

float Vec2::Mag() const
{
    return std::sqrt((x * x) + (y * y));
}

float Vec2::MagSq() const
{
    return (x * x) + (y * y);
}

Vec2 Vec2::Norm() const
{
    float m = Mag();
    if (m <= 0) return Vec2(0, 0);
    return Vec2(x, y) * (1 / m);
}

float Vec2::Dot(const Vec2& other) const
{
    return (x * other.x) + (y * other.y);
}

float Vec2::Cross(const Vec2& other) const
{
    return (x * other.y) - (y * other.x);
}