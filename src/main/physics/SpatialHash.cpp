#include "main/physics/SpatialHash.h"
#include "main/physics/SAT.h"
#include <vector>

unsigned int SpatialHash::GetHash(Vec2 position) const
{
    int x = static_cast<int>(std::floor(position.x / cellSize));
    int y = static_cast<int>(std::floor(position.y / cellSize));

    unsigned int hash = (x * 73856093) ^ (y * 19349663);
    return hash;
}

std::vector<Vec2> SpatialHash::GetBounding(GameObject* obj)
{
    Collider* c = obj->GetCollider();

    if (c->GetType() == ColliderType::CIRCLE)
    {
        CircleCollider* circle = static_cast<CircleCollider*>(c);
        Vec2 pos = obj->transform.position;
        float r = circle->radius;
        
        return {
            Vec2(pos.x - r, pos.y - r),
            Vec2(pos.x + r, pos.y + r)
        };
    }

    std::vector<Vec2> vertices = SAT::GetVertices(obj);

    SAT::Projection xProj = SAT::Project(vertices, Vec2(1, 0));
    SAT::Projection yProj = SAT::Project(vertices, Vec2(0, 1));

    return {
        Vec2(xProj.min, yProj.min),
        Vec2(xProj.max, yProj.max)
    };
}