#pragma once
#include "Vec2.h"
#include <cmath>

struct Matrix2x2
{
    float m00, m01;
    float m10, m11;

    Matrix2x2(float m00, float m01, float m10, float m11) : m00(m00), m01(m01), m10(m10), m11(m11) {}

    Vec2 operator*(const Vec2& v) const
    {
        return Vec2(m00 * v.x + m01 * v.y, m10 * v.x + m11 * v.y);
    }

    Matrix2x2 operator*(const Matrix2x2& other) const
    {
        return Matrix2x2(
            m00 * other.m00 + m01 * other.m10, m00 * other.m01 + m01 * other.m11,
            m10 * other.m00 + m11 * other.m10, m10 * other.m01 + m11 * other.m11
        );
    }

    Matrix2x2 operator*(float scalar) const
    {
        return Matrix2x2(m00 * scalar, m01 * scalar, m10 * scalar, m11 * scalar);
    }

    Matrix2x2 operator+(const Matrix2x2& other) const
    {
        return Matrix2x2(m00 + other.m00, m01 + other.m01, m10 + other.m10, m11 + other.m11);
    }

    Matrix2x2 operator+=(const Matrix2x2& other)
    {
        *this = *this + other;
        return *this;
    }

    float Determinant() const
    {
        return m00 * m11 - m01 * m10;
    }

    Matrix2x2 Transpose() const
    {
        return Matrix2x2(m00, m10, m01, m11);
    }

    Matrix2x2 Inverse() const
    {
        float det = m00 * m11 - m01 * m10;
        if (std::abs(det) <= 1e-6f) return Matrix2x2(0, 0, 0, 0);
        
        float invDet = 1.0f / det;
        return Matrix2x2(m11 * invDet, -m01 * invDet, -m10 * invDet, m00 * invDet);
    }
};