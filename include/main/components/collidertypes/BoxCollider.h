#pragma once
#include "math/Vec2.h"
#include "main/components/Collider.h"

class GameObject;

class BoxCollider : public Collider
{
    public:
        Vec2 size;
        BoxCollider(Vec2 s);
        ColliderType getType() const override;
};