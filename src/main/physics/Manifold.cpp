#include "main/physics/Manifold.h"
#include "main/physics/SAT.h"
#include "main/GameObject.h"
        
CollisionManifold Manifold::GenCircleCircle(GameObject* obj1, GameObject* obj2)
{
    Collision collision = SAT::CircleCircle(obj1, obj2);
    CollisionManifold cm;
    cm.collision = collision;

    if (!collision.isColliding) return cm;

    CircleCollider *circle1 = static_cast<CircleCollider*>(obj1->c);
    Vec2 point = obj1->transform.position + (collision.normal * circle1->radius);
    cm.points.PushBack(point);

    return cm;
}

CollisionManifold Manifold::GenBoxCircle(GameObject* obj1, GameObject* obj2)
{
    Collision collision = SAT::BoxCircle(obj1, obj2);
    CollisionManifold cm;
    cm.collision = collision;

    if (!collision.isColliding) return cm;

    Collider* c1 = obj1->c;
    Array<20> vertices = c1->GetVertices();

    Vec2 center = obj2->transform.position;
    cm.points = GetPolygonCircleContacts(vertices, center);

    return cm;
}

CollisionManifold Manifold::GenBoxBox(GameObject* obj1, GameObject* obj2)
{
    Collision collision = SAT::BoxBox(obj1, obj2);
    CollisionManifold cm;
    cm.collision = collision;

    if (!collision.isColliding) return cm;

    Collider* c1 = obj1->c;
    Collider* c2 = obj2->c;

    Array<20> vertices1 = c1->GetVertices();
    Array<20> vertices2 = c2->GetVertices();

    cm.points = GetPolygonContacts(vertices1, vertices2, collision.normal);

    return cm;
}

CollisionManifold Manifold::GenPolyCircle(GameObject* obj1, GameObject* obj2)
{
    Collision collision = SAT::PolygonCircle(obj1, obj2);
    CollisionManifold cm;
    cm.collision = collision;

    if (!collision.isColliding) return cm;

    Collider* c1 = obj1->c;
    Array<20> vertices = c1->GetVertices();

    Vec2 center = obj2->transform.position;
    cm.points = GetPolygonCircleContacts(vertices, center);

    return cm;  
}

CollisionManifold Manifold::GenPolyBox(GameObject* obj1, GameObject* obj2)
{
    Collision collision = SAT::PolygonBox(obj1, obj2);
    CollisionManifold cm;
    cm.collision = collision;

    if (!collision.isColliding) return cm;

    Collider *c1 = obj1->c;
    Collider *c2 = obj2->c;

    Array<20> vertices1 = c1->GetVertices();
    Array<20> vertices2 = c2->GetVertices();
    
    cm.points = GetPolygonContacts(vertices1, vertices2, collision.normal);

    return cm;
}

CollisionManifold Manifold::GenPolyPoly(GameObject* obj1, GameObject* obj2)
{
    Collision collision = SAT::PolygonPolygon(obj1, obj2);
    CollisionManifold cm;
    cm.collision = collision;

    if (!collision.isColliding) return cm;

    Collider *c1 = obj1->c;
    Collider *c2 = obj2->c;

    Array<20> vertices1 = c1->GetVertices();
    Array<20> vertices2 = c2->GetVertices();
    
    cm.points = GetPolygonContacts(vertices1, vertices2, collision.normal);

    return cm;
}

Array<4> Manifold::GetPolygonCircleContacts(const Array<20>& vertices1, Vec2 center)
{
    Array<4> points;
    
    float minDistSq = INFINITY;
    Vec2 closestPoint;

    for (size_t i = 0; i < vertices1.Size(); i++)
    {
        Vec2 p1 = vertices1[i];
        Vec2 p2 = vertices1[(i + 1) % vertices1.Size()];
        Vec2 line = p2 - p1;

        Vec2 cLine = center - p1;
        float t = cLine.Dot(line) / line.MagSq();
        
        t = std::max(0.0f, std::min(1.0f, t));
        
        Vec2 nearest = p1 + (line * t);
        
        Vec2 distVector = center - nearest;
        float distSq = distVector.MagSq();

        if (distSq < minDistSq)
        {
            minDistSq = distSq;
            closestPoint = nearest;
        }
    }

    points.PushBack(closestPoint);
    return points;
}

Array<4> Manifold::GetPolygonContacts(const Array<20>& vertices1, const Array<20>& vertices2, Vec2 normal)
{
    Array<4> points;
    
    Edge edge1 = GetSupportFace(vertices1, normal);
    Edge edge2 = GetSupportFace(vertices2, normal * -1.0f); 

    Edge incident;
    Edge reference;

    if (std::abs(edge1.line.Dot(normal)) <= std::abs(edge2.line.Dot(normal))) 
    {
        reference = edge1;
        incident = edge2;
    } 
    else 
    {
        reference = edge2;
        incident = edge1;
    }

    Vec2 refDir = reference.line.Norm();

    Array<20> clip1;
    float d1 = refDir.Dot(incident.p1 - reference.p1);
    float d2 = refDir.Dot(incident.p2 - reference.p1);

    if (d1 >= 0.0f) clip1.PushBack(incident.p1);
    if (d2 >= 0.0f) clip1.PushBack(incident.p2);

    if (d1 * d2 < 0.0f) 
    {
        float alpha = d1 / (d1 - d2);
        clip1.PushBack(incident.p1 + (incident.p2 - incident.p1) * alpha);
    }
    if (clip1.Size() < 2) return points;

    Array<20> clip2;
    float d3 = (refDir * -1.0f).Dot(clip1[0] - reference.p2);
    float d4 = (refDir * -1.0f).Dot(clip1[1] - reference.p2);

    if (d3 >= 0.0f) clip2.PushBack(clip1[0]);
    if (d4 >= 0.0f) clip2.PushBack(clip1[1]);

    if (d3 * d4 < 0.0f) 
    {
        float alpha = d3 / (d3 - d4);
        clip2.PushBack(clip1[0] + (clip1[1] - clip1[0]) * alpha);
    }
    if (clip2.Size() < 2) return points;

    Vec2 refOutNormal = Vec2(reference.line.y, -reference.line.x).Norm();
    
    for (size_t i = 0; i < clip2.Size(); i++)
    {
        Vec2 p = clip2[i];
        float separation = refOutNormal.Dot(p - reference.p1);
        
        if (separation <= 0.0f) 
        {
            points.PushBack(p);
        }
    }

    return points;    
}

Edge Manifold::GetSupportFace(const Array<20>& vertices, Vec2 normal)
{
    float maxDot = -INFINITY;
    int index = 0;

    for (size_t i = 0; i < vertices.Size(); i++)
    {
        float dot = vertices[i].Dot(normal);
        if (dot > maxDot)
        {
            index = static_cast<int>(i);
            maxDot = dot;
        }
    }

    Vec2 spear = vertices[index];
    Vec2 prev = vertices[(index - 1 + vertices.Size()) % vertices.Size()];
    Vec2 next = vertices[(index + 1) % vertices.Size()];

    Vec2 v1 = (spear - prev).Norm();
    Vec2 v2 = (spear - next).Norm();

    if (std::abs(v1.Dot(normal)) <= std::abs(v2.Dot(normal)))
        return { prev, spear, (spear - prev).Norm() }; 
    else
        return { spear, next, (next - spear).Norm() };
}