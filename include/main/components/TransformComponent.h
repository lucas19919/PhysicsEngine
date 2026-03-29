#pragma once
#include "math/Vec2.h"

class GameObject;

class TransformComponent
{
    public:
        TransformComponent();

        Vec2 position;
        float rotation;
};