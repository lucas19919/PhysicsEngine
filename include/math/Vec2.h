#pragma once
#include <cmath>

struct Vec2 {
    float x;
    float y;

    Vec2() : x(0.0f), y(0.0f) {};
    Vec2(float xIn, float yIn) : x(xIn), y(yIn) {};

    Vec2 operator+(const Vec2& other) const
    {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 operator-(const Vec2& other) const
    {
        return Vec2(x - other.x, y - other.y);
    }

    Vec2 operator*(float scalar) const
    {
        return Vec2(x * scalar, y * scalar);
    }

    Vec2 operator/(float scalar) const
    {
        return Vec2(x / scalar, y / scalar);
    }

    void operator+=(const Vec2& other)
    {
        x += other.x;
        y += other.y;
    }

    void operator-=(const Vec2& other)
    {
        x -= other.x;
        y -= other.y;
    }

    void operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
    }

    void operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
    }

    float Mag() const;
    float MagSq() const;
    Vec2 Norm() const;
    float Dot(const Vec2& other) const;
    float Cross(const Vec2& other) const;
};

inline Vec2 operator*(float scalar, const Vec2& vec) 
{
    return vec * scalar; 
}