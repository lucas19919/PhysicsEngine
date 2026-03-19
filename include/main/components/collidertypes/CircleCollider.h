#pragma once
#include "math/Vec2.h"
#include "main/components/Collider.h"

class GameObject;

class CircleCollider : public Collider
{
    public:
        float radius;    
        CircleCollider(float r);
        ColliderType getType() const override;
};