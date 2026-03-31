#include "main/physics/SAT.h"
#include "main/GameObject.h"
#include "main/components/Collider.h"
#include <cmath>

bool SAT::TestBounds(GameObject* obj1, GameObject* obj2)
{
    BBox bounds1 = obj1->GetCollider()->GetBounds();
    BBox bounds2 = obj2->GetCollider()->GetBounds();

    return !(bounds1.max.x < bounds2.min.x || bounds2.max.x < bounds1.min.x || bounds1.max.y < bounds2.min.y || bounds2.max.y < bounds1.min.y);
}

Collision SAT::CircleCircle(GameObject *obj1, GameObject *obj2)
{
    float distanceSq = (obj1->transform.position - obj2->transform.position).MagSq();
    float radiusSum = static_cast<CircleCollider*>(obj1->GetCollider())->radius + static_cast<CircleCollider*>(obj2->GetCollider())->radius;

    if (distanceSq <= radiusSum * radiusSum) {
        if (distanceSq == 0.0f) {
            return { true, Vec2(1.0f, 0.0f), radiusSum };
        }
        
        float distance = std::sqrt(distanceSq);
        Vec2 axis = (obj2->transform.position - obj1->transform.position).Norm();

        return { true, axis, radiusSum - distance };
    }
    
    return { false, Vec2(), 0.0f };
}

Collision SAT::BoxBox(GameObject *obj1, GameObject *obj2)
{
    Array<20> vertices1 = obj1->cachedVertices;
    Array<20> vertices2 = obj2->cachedVertices;
    Array<20> normals1 = obj1->cachedNormals;
    Array<20> normals2 = obj2->cachedNormals;

    float minOverlap = INFINITY;
    Vec2 smallestAxis;

    for (size_t i = 0; i < normals1.Size(); i++)
    {
        Vec2 axis = normals1[i];
        Projection p1 = Project(vertices1, axis);
        Projection p2 = Project(vertices2, axis);

        if (p1.max < p2.min || p2.max < p1.min) {
            return { false, Vec2(), 0.0f };
        }

        float overlap = std::min(p1.max, p2.max) - std::max(p1.min, p2.min);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }

    for (size_t i = 0; i < normals2.Size(); i++)
    {
        Vec2 axis = normals2[i];
        Projection p1 = Project(vertices1, axis);
        Projection p2 = Project(vertices2, axis);

        if (p1.max < p2.min || p2.max < p1.min) {
            return { false, Vec2(), 0.0f };
        }

        float overlap = std::min(p1.max, p2.max) - std::max(p1.min, p2.min);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }    

    Vec2 direction = obj2->transform.position - obj1->transform.position;

    if (smallestAxis.Dot(direction) < 0.0f) {
        smallestAxis = Vec2(-smallestAxis.x, -smallestAxis.y);
    }

    return { true, smallestAxis, minOverlap };
}

Collision SAT::BoxCircle(GameObject *obj1, GameObject *obj2)
{
    Vec2 center = obj2->transform.position;

    Array<20> vertices = obj1->cachedVertices;
    Array<20> normals = obj1->cachedNormals;
    
    float minOverlap = INFINITY;
    Vec2 smallestAxis;

    Vec2 distAxis = center - vertices[0];
    float dist = (center - vertices[0]).MagSq();
    for (size_t i = 1; i < vertices.Size(); i++)
    {
        float currentDistSq = (center - vertices[i]).MagSq();
        if (dist > currentDistSq) {
            distAxis = center - vertices[i];
            dist = currentDistSq;
        }
    }

    if (dist > 0.0f) {
        normals.PushBack(distAxis.Norm());
    } else {
        normals.PushBack(Vec2(1.0f, 0.0f));
    }

    for (size_t i = 0; i < normals.Size(); i++)
    {
        Vec2 axis = normals[i];
        Projection p1 = Project(vertices, axis);
        Projection p2 = CircleProject(obj2, axis);

        if (p1.max < p2.min || p2.max < p1.min) {
            return { false, Vec2(), 0.0f };
        }

        float overlap = std::min(p1.max, p2.max) - std::max(p1.min, p2.min);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }           

    Vec2 direction = obj2->transform.position - obj1->transform.position;

    if (smallestAxis.Dot(direction) < 0.0f) {
        smallestAxis = Vec2(-smallestAxis.x, -smallestAxis.y);
    }

    return { true, smallestAxis, minOverlap };
}

Collision SAT::PolygonCircle(GameObject *obj1, GameObject *obj2)
{
    Vec2 center = obj2->transform.position;

    Array<20> vertices = obj1->cachedVertices;
    Array<20> normals = obj1->cachedNormals;
    
    float minOverlap = INFINITY;
    Vec2 smallestAxis;

    Vec2 distAxis = center - vertices[0];
    float dist = (center - vertices[0]).MagSq();
    for (size_t i = 1; i < vertices.Size(); i++)
    {
        float currentDistSq = (center - vertices[i]).MagSq();
        if (dist > currentDistSq) {
            distAxis = center - vertices[i];
            dist = currentDistSq;
        }
    }

    if (dist > 0.0f) {
        normals.PushBack(distAxis.Norm());
    } else {
        normals.PushBack(Vec2(1.0f, 0.0f));
    }

    for (size_t i = 0; i < normals.Size(); i++)
    {
        Vec2 axis = normals[i];
        Projection p1 = Project(vertices, axis);
        Projection p2 = CircleProject(obj2, axis);

        if (p1.max < p2.min || p2.max < p1.min) {
            return { false, Vec2(), 0.0f };
        }

        float overlap = std::min(p1.max, p2.max) - std::max(p1.min, p2.min);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }           

    Vec2 direction = obj2->transform.position - obj1->transform.position;

    if (smallestAxis.Dot(direction) < 0.0f) {
        smallestAxis = Vec2(-smallestAxis.x, -smallestAxis.y);
    }

    return { true, smallestAxis, minOverlap };
}

Collision SAT::PolygonBox(GameObject *obj1, GameObject *obj2)
{
    Array<20> vertices1 = obj1->cachedVertices;
    Array<20> vertices2 = obj2->cachedVertices;
    Array<20> normals1 = obj1->cachedNormals;
    Array<20> normals2 = obj2->cachedNormals;

    float minOverlap = INFINITY;
    Vec2 smallestAxis;

    for (size_t i = 0; i < normals1.Size(); i++) 
    {
        Vec2 axis = normals1[i];
        Projection p1 = Project(vertices1, axis);
        Projection p2 = Project(vertices2, axis);

        if (p1.max < p2.min || p2.max < p1.min) {
            return { false, Vec2(), 0.0f }; 
        }

        float overlap = std::min(p1.max, p2.max) - std::max(p1.min, p2.min);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }

    for (size_t i = 0; i < normals2.Size(); i++)
    {
        Vec2 axis = normals2[i];
        Projection p1 = Project(vertices1, axis);
        Projection p2 = Project(vertices2, axis);

        if (p1.max < p2.min || p2.max < p1.min) {
            return { false, Vec2(), 0.0f };
        }

        float overlap = std::min(p1.max, p2.max) - std::max(p1.min, p2.min);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }

    Vec2 direction = obj2->transform.position - obj1->transform.position;

    if (smallestAxis.Dot(direction) < 0.0f) {
        smallestAxis = Vec2(-smallestAxis.x, -smallestAxis.y);
    }

    return { true, smallestAxis, minOverlap };
}

Collision SAT::PolygonPolygon(GameObject *obj1, GameObject *obj2)
{
    Array<20> vertices1 = obj1->cachedVertices;
    Array<20> vertices2 = obj2->cachedVertices;
    Array<20> normals1 = obj1->cachedNormals;
    Array<20> normals2 = obj2->cachedNormals;

    float minOverlap = INFINITY;
    Vec2 smallestAxis;

    for (size_t i = 0; i < normals1.Size(); i++) 
    {
        Vec2 axis = normals1[i];
        Projection p1 = Project(vertices1, axis);
        Projection p2 = Project(vertices2, axis);

        if (p1.max < p2.min || p2.max < p1.min) {
            return { false, Vec2(), 0.0f }; 
        }

        float overlap = std::min(p1.max, p2.max) - std::max(p1.min, p2.min);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }

    for (size_t i = 0; i < normals2.Size(); i++)
    {
        Vec2 axis = normals2[i];
        Projection p1 = Project(vertices1, axis);
        Projection p2 = Project(vertices2, axis);

        if (p1.max < p2.min || p2.max < p1.min) {
            return { false, Vec2(), 0.0f };
        }

        float overlap = std::min(p1.max, p2.max) - std::max(p1.min, p2.min);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }

    Vec2 direction = obj2->transform.position - obj1->transform.position;

    if (smallestAxis.Dot(direction) < 0.0f) {
        smallestAxis = Vec2(-smallestAxis.x, -smallestAxis.y);
    }

    return { true, smallestAxis, minOverlap };
}

SAT::Projection SAT::CircleProject(GameObject* obj, const Vec2 axis)
{
    float centerDot = obj->transform.position.Dot(axis);
    
    CircleCollider* c = static_cast<CircleCollider*>(obj->GetCollider());
    return { 
        centerDot - c->radius, 
        centerDot + c->radius
    };
}

SAT::Projection SAT::Project(const Array<20>& vertices, const Vec2 axis)
{
    Projection project;

    project.min = axis.Dot(vertices[0]);
    project.max = project.min;
    
    for (size_t i = 0; i < vertices.Size(); i++)
    {
        float dot = axis.Dot(vertices[i]);
        if (dot < project.min) project.min = dot;
        if (dot > project.max) project.max = dot;
    }

    return project;
}

Array<20> SAT::GetVertices(GameObject* obj1)
{
    Array<20> worldVertices;

    if (obj1 == nullptr || obj1->GetCollider() == nullptr) 
        return worldVertices; 

    TransformComponent transform = obj1->transform;
    Array<20> localVertices;

    if (obj1->GetCollider()->GetType() == ColliderType::BOX) 
    {
        BoxCollider* b = static_cast<BoxCollider*>(obj1->GetCollider());
        float x = b->size.x / 2.0f;
        float y = b->size.y / 2.0f;
        
        localVertices.PushBack(Vec2(-x, -y));
        localVertices.PushBack(Vec2( x, -y));
        localVertices.PushBack(Vec2( x,  y));
        localVertices.PushBack(Vec2(-x,  y));
    } 
    else if (obj1->GetCollider()->GetType() == ColliderType::POLYGON) 
    {
        PolygonCollider* p = static_cast<PolygonCollider*>(obj1->GetCollider());
        localVertices = p->vertices;
    }
    else 
    {
        return worldVertices; 
    }
    
    float cos = std::cos(transform.rotation);
    float sin = std::sin(transform.rotation);

    for (size_t i = 0; i < localVertices.Size(); i++)
    {
        float x = localVertices[i].x;
        float y = localVertices[i].y;

        worldVertices.PushBack(Vec2(
            (x * cos) - (y * sin) + transform.position.x,
            (x * sin) + (y * cos) + transform.position.y
        ));
    }

    return worldVertices;
}

Array<20> SAT::GetNormals(const Array<20>& vertices)
{
    Array<20> normals;
    for (size_t i = 0; i < vertices.Size(); i++)
    {
        Vec2 line = vertices[(i + 1) % vertices.Size()] - vertices[i];
        Vec2 pLine = Vec2(line.y, -line.x);
        normals.PushBack(pLine.Norm());
    }

    return normals;
}