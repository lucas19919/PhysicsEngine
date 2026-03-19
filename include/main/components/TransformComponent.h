#pragma once
#include "math/Vec2.h"

class GameObject;

class TransformComponent
{
    public:
        TransformComponent(GameObject* parentObj);

        GameObject* parent;

        Vec2 position;
        float rotation;
        Vec2 scale;
};