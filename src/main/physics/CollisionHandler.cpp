#include "main/physics/CollisionHandler.h"
#include "main/physics/SAT.h"
#include "main/components/Collider.h"

Collision CollisionHandler::SortCollision(Collider* c1, Collider* c2)
{
    if (c1->GetType() > c2->GetType()) {
        Collision col = SortCollision(c2, c1);
        col.normal = Vec2(-col.normal.x, -col.normal.y);
        return col;
    }

    ColliderType t1 = c1->GetType();
    ColliderType t2 = c2->GetType();

    if (t1 == ColliderType::CIRCLE && t2 == ColliderType::CIRCLE) {
        return SAT::CircleCircle(c1, c2);
    }
    if (t1 == ColliderType::CIRCLE && t2 == ColliderType::BOX) {
        Collision col = SAT::BoxCircle(c2, c1);
        col.normal = Vec2(-col.normal.x, -col.normal.y);
        return col;
    }
    if (t1 == ColliderType::CIRCLE && t2 == ColliderType::POLYGON) {
        Collision col = SAT::PolygonCircle(c2, c1);
        col.normal = Vec2(-col.normal.x, -col.normal.y);
        return col;
    }
    if (t1 == ColliderType::BOX && t2 == ColliderType::BOX) {
        return SAT::BoxBox(c1, c2);
    }
    if (t1 == ColliderType::BOX && t2 == ColliderType::POLYGON) {
        Collision col = SAT::PolygonBox(c2, c1);
        col.normal = Vec2(-col.normal.x, -col.normal.y);
        return col;
    }
    if (t1 == ColliderType::POLYGON && t2 == ColliderType::POLYGON) {
        return SAT::PolygonPolygon(c1, c2);
    }

    return { false, Vec2(), 0.0f };
}