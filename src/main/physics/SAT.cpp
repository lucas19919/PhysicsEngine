#include "main/physics/SAT.h"
#include "main/GameObject.h"
#include "main/components/Collider.h"

Collision SAT::CircleCircle(Collider *c1, Collider *c2)
{
    float distanceSq = (c1->parent->transform.position - c2->parent->transform.position).MagSq();
    float radiusSum = static_cast<CircleCollider*>(c1)->radius + static_cast<CircleCollider*>(c2)->radius;

    if (distanceSq <= radiusSum * radiusSum) {
        if (distanceSq == 0.0f) {
            return { true, Vec2(1.0f, 0.0f), radiusSum };
        }
        
        float distance = std::sqrt(distanceSq);
        Vec2 axis = (c2->parent->transform.position - c1->parent->transform.position).Norm();

        return { true, axis, radiusSum - distance };
    }
    
    return { false, Vec2(), 0.0f };
}

Collision SAT::BoxBox(Collider *b1, Collider *b2)
{
    std::vector<Vec2> vertices1 = GetVertices(static_cast<BoxCollider*>(b1));
    std::vector<Vec2> vertices2 = GetVertices(static_cast<BoxCollider*>(b2));
    std::vector<Vec2> normals1 = GetNormals(vertices1);
    std::vector<Vec2> normals2 = GetNormals(vertices2);

    float minOverlap = INFINITY;
    Vec2 smallestAxis;

    for (Vec2 axis : normals1)
    {
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

    for (Vec2 axis : normals2)
    {
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

    Vec2 direction = b2->parent->transform.position - b1->parent->transform.position;

    if (smallestAxis.Dot(direction) < 0.0f) {
        smallestAxis = Vec2(-smallestAxis.x, -smallestAxis.y);
    }

    return { true, smallestAxis, minOverlap };
}

Collision SAT::BoxCircle(Collider *b1, Collider *c1)
{
    CircleCollider* circle = static_cast<CircleCollider*>(c1);
    Vec2 center = circle->parent->transform.position;

    std::vector<Vec2> vertices = GetVertices(static_cast<BoxCollider*>(b1));
    std::vector<Vec2> normals = GetNormals(vertices);
    
    float minOverlap = INFINITY;
    Vec2 smallestAxis;

    Vec2 distAxis = center - vertices[0];
    float dist = (center - vertices[0]).MagSq();
    for (size_t i = 1; i < vertices.size(); i++)
    {
        float currentDistSq = (center - vertices[i]).MagSq();
        if (dist > currentDistSq) {
            distAxis = center - vertices[i];
            dist = currentDistSq;
        }
    }

    if (dist > 0.0f) {
        normals.push_back(distAxis.Norm());
    } else {
        normals.push_back(Vec2(1.0f, 0.0f));
    }

    for (Vec2 axis : normals)
    {
        Projection p1 = Project(vertices, axis);
        Projection p2 = CircleProject(circle, axis);

        if (p1.max < p2.min || p2.max < p1.min) {
            return { false, Vec2(), 0.0f };
        }

        float overlap = std::min(p1.max, p2.max) - std::max(p1.min, p2.min);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }           

    Vec2 direction = c1->parent->transform.position - b1->parent->transform.position;

    if (smallestAxis.Dot(direction) < 0.0f) {
        smallestAxis = Vec2(-smallestAxis.x, -smallestAxis.y);
    }

    return { true, smallestAxis, minOverlap };
}

Collision SAT::PolygonCircle(Collider *p1, Collider *c1)
{
    CircleCollider* circle = static_cast<CircleCollider*>(c1);
    Vec2 center = circle->parent->transform.position;

    std::vector<Vec2> vertices = GetVertices(static_cast<PolygonCollider*>(p1));
    std::vector<Vec2> normals = GetNormals(vertices);
    
    float minOverlap = INFINITY;
    Vec2 smallestAxis;

    Vec2 distAxis = center - vertices[0];
    float dist = (center - vertices[0]).MagSq();
    for (size_t i = 1; i < vertices.size(); i++)
    {
        float currentDistSq = (center - vertices[i]).MagSq();
        if (dist > currentDistSq) {
            distAxis = center - vertices[i];
            dist = currentDistSq;
        }
    }

    if (dist > 0.0f) {
        normals.push_back(distAxis.Norm());
    } else {
        normals.push_back(Vec2(1.0f, 0.0f));
    }

    for (Vec2 axis : normals)
    {
        Projection p1 = Project(vertices, axis);
        Projection p2 = CircleProject(circle, axis);

        if (p1.max < p2.min || p2.max < p1.min) {
            return { false, Vec2(), 0.0f };
        }

        float overlap = std::min(p1.max, p2.max) - std::max(p1.min, p2.min);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }           

    Vec2 direction = c1->parent->transform.position - p1->parent->transform.position;

    if (smallestAxis.Dot(direction) < 0.0f) {
        smallestAxis = Vec2(-smallestAxis.x, -smallestAxis.y);
    }

    return { true, smallestAxis, minOverlap };
}

Collision SAT::PolygonBox(Collider *p1, Collider *b1)
{
    std::vector<Vec2> vertices1 = GetVertices(static_cast<PolygonCollider*>(p1));
    std::vector<Vec2> vertices2 = GetVertices(static_cast<BoxCollider*>(b1));
    std::vector<Vec2> normals1 = GetNormals(vertices1);
    std::vector<Vec2> normals2 = GetNormals(vertices2);

    float minOverlap = INFINITY;
    Vec2 smallestAxis;

    for (Vec2 axis : normals1) {
        Projection p1 = Project(vertices1, axis);
        Projection p2 = Project(vertices2, axis);

        if (p1.max < p2.min || p2.max < p1.min) {
            return { false, Vec2(), 0.0f }; // Found a gap! No collision.
        }

        float overlap = std::min(p1.max, p2.max) - std::max(p1.min, p2.min);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }

    for (Vec2 axis : normals2)
    {
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

    Vec2 direction = b1->parent->transform.position - p1->parent->transform.position;

    if (smallestAxis.Dot(direction) < 0.0f) {
        smallestAxis = Vec2(-smallestAxis.x, -smallestAxis.y);
    }

    return { true, smallestAxis, minOverlap };
}

Collision SAT::PolygonPolygon(Collider *p1, Collider *p2)
{
    std::vector<Vec2> vertices1 = GetVertices(static_cast<PolygonCollider*>(p1));
    std::vector<Vec2> vertices2 = GetVertices(static_cast<PolygonCollider*>(p2));
    std::vector<Vec2> normals1 = GetNormals(vertices1);
    std::vector<Vec2> normals2 = GetNormals(vertices2);

    float minOverlap = INFINITY;
    Vec2 smallestAxis;

    for (Vec2 axis : normals1) {
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

    for (Vec2 axis : normals2)
    {
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

    Vec2 direction = p2->parent->transform.position - p1->parent->transform.position;

    if (smallestAxis.Dot(direction) < 0.0f) {
        smallestAxis = Vec2(-smallestAxis.x, -smallestAxis.y);
    }

    return { true, smallestAxis, minOverlap };
}

SAT::Projection SAT::CircleProject(CircleCollider* c, const Vec2 axis)
{
    float centerDot = c->parent->transform.position.Dot(axis);
    
    return { 
        centerDot - c->radius, 
        centerDot + c->radius 
    };
}

SAT::Projection SAT::Project(std::vector<Vec2> vertices, const Vec2 axis)
{
    Projection project;

    project.min = axis.Dot(vertices[0]);
    project.max = project.min;
    
    for (int i = 0; i < vertices.size(); i++)
    {
        float dot = axis.Dot(vertices[i]);
        if (dot < project.min) project.min = dot;
        if (dot > project.max) project.max = dot;
    }

    return project;
}

std::vector<Vec2> SAT::GetVertices(Collider *c)
{
    if (c == nullptr || c->parent == nullptr) 
        return {}; 

    TransformComponent transform = c->parent->transform;
    std::vector<Vec2> localVertices;

    if (c->GetType() == ColliderType::BOX) 
    {
        BoxCollider* b = static_cast<BoxCollider*>(c);
        float x = b->size.x / 2.0f;
        float y = b->size.y / 2.0f;
        
        localVertices = {
            Vec2(-x, -y), // bot left
            Vec2( x, -y), // bot right
            Vec2( x,  y), // top right
            Vec2(-x,  y)  // top left
        };
    } 
    else if (c->GetType() == ColliderType::POLYGON) 
    {
        PolygonCollider* p = static_cast<PolygonCollider*>(c);
        localVertices = p->vertices; 
    }
    else 
    {
        return {}; 
    }

    std::vector<Vec2> worldVertices(localVertices.size());
    
    float cos = std::cos(transform.rotation);
    float sin = std::sin(transform.rotation);

    for (size_t i = 0; i < localVertices.size(); i++)
    {
        float x = localVertices[i].x;
        float y = localVertices[i].y;

        worldVertices[i] = Vec2(
            (x * cos) - (y * sin) + transform.position.x,
            (x * sin) + (y * cos) + transform.position.y
        );
    }

    return worldVertices;
}

std::vector<Vec2> SAT::GetNormals(std::vector<Vec2> vertices)
{
    std::vector<Vec2> normals;
    for (int i = 0; i < vertices.size(); i++)
    {
        Vec2 line = vertices[(i + 1) % vertices.size()] - vertices[i];
        Vec2 pLine = Vec2(line.y, -line.x);
        normals.push_back(pLine.Norm());
    }

    return normals;
}