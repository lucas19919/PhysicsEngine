#pragma once
#include "math/Vec2.h"
#include "main/components/Collider.h"
#include "main/utility/templates/Array.h"
#include <vector>

class GameObject;

class PolygonCollider : public Collider
{
    public:
        Array<20> vertices;
        PolygonCollider(const Array<20>& vertices);
        ColliderType GetType() const override;
};