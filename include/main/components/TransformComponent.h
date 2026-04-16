#pragma once
#include "math/Vec2.h"
#include "main/components/Component.h"

class GameObject;

class TransformComponent : public Component
{
    public:
        TransformComponent();

        Vec2 position;
        float rotation;
};