#pragma once
#include "math/Vec2.h"

template<size_t Max>
struct Array 
{
    Vec2 data[Max];
    size_t count = 0;

    void PushBack(const Vec2& v) 
    {
        if (count < Max) {
            data[count] = v;
            count++;
        }
    }
    
    Vec2& operator[](size_t i) { return data[i]; }
    const Vec2& operator[](size_t i) const { return data[i]; }
    size_t Size() const { return count; }
    bool Empty() const { return count == 0; }
};