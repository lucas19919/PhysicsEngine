#pragma once
#include "math/Vec2.h"

template<size_t Max>
struct Array 
{
    Vec2 data[Max];
    size_t count = 0;

    void push_back(const Vec2& v) 
    {
        if (count < Max) {
            data[count] = v;
            count++;
        }
    }
    
    Vec2& operator[](size_t i) { return data[i]; }
    const Vec2& operator[](size_t i) const { return data[i]; }
    size_t size() const { return count; }
    bool empty() const { return count == 0; }

    size_t size() { return count; }
};