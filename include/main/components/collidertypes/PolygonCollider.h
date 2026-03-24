#pragma once
#include "math/Vec2.h"
#include "main/components/Collider.h"
#include <vector>

class GameObject;

class PolygonCollider : public Collider
{
    public:
        std::vector<Vec2> vertices;
        PolygonCollider(const std::vector<Vec2>& vertices);
        ColliderType GetType() const override;
};