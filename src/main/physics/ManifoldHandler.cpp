#include "main/physics/ManifoldHandler.h"
#include "main/physics/Manifold.h"
#include "main/components/Collider.h"

CollisionManifold ManifoldHandler::SortManifold(GameObject* obj1, GameObject* obj2)
{
    Collider* c1 = obj1->GetCollider();
    Collider* c2 = obj2->GetCollider();

    if (!c1 || !c2) 
    {
        return {};
    }

    if (c1->GetType() > c2->GetType()) 
    {
        CollisionManifold col = SortManifold(obj2, obj1);
        col.Collision.normal = Vec2(-col.Collision.normal.x, -col.Collision.normal.y);
        return col;
    }

    ColliderType t1 = c1->GetType();
    ColliderType t2 = c2->GetType();

    if (t1 == ColliderType::CIRCLE && t2 == ColliderType::CIRCLE) 
    {
        return Manifold::GenCircleCircle(c1, c2);
    }

    if (t1 == ColliderType::CIRCLE && t2 == ColliderType::BOX) 
    {
        CollisionManifold col = Manifold::GenBoxCircle(c2, c1);
        col.Collision.normal = Vec2(-col.Collision.normal.x, -col.Collision.normal.y);
        return col;
    }

    if (t1 == ColliderType::CIRCLE && t2 == ColliderType::POLYGON) 
    {
        CollisionManifold col = Manifold::GenPolyCircle(c2, c1);
        col.Collision.normal = Vec2(-col.Collision.normal.x, -col.Collision.normal.y);
        return col;
    }

    if (t1 == ColliderType::BOX && t2 == ColliderType::BOX) 
    {
        return Manifold::GenBoxBox(c1, c2);
    }

    if (t1 == ColliderType::BOX && t2 == ColliderType::POLYGON) 
    {
        CollisionManifold col = Manifold::GenPolyBox(c2, c1);
        col.Collision.normal = Vec2(-col.Collision.normal.x, -col.Collision.normal.y);
        return col;
    }

    if (t1 == ColliderType::POLYGON && t2 == ColliderType::POLYGON) 
    {
        return Manifold::GenPolyPoly(c1, c2);
    }

    return {};
}