#include "main/physics/Manifold.h"
#include "main/physics/SAT.h"
#include "main/GameObject.h"
        
CollisionManifold Manifold::GenCircleCircle(Collider* c1, Collider* c2)
{
    Collision collision = SAT::CircleCircle(c1, c2);
    CollisionManifold cm = { collision };

    if (!collision.isColliding) return cm;

    CircleCollider *circle1 = static_cast<CircleCollider*>(c1);
    Vec2 point = c1->parent->transform.position + (collision.normal * circle1->radius);
    cm.points.push_back(point);

    return cm;
}

CollisionManifold Manifold::GenBoxCircle(Collider* c1, Collider* c2)
{
    Collision collision = SAT::BoxCircle(c1, c2);
    CollisionManifold cm = { collision };

    if (!collision.isColliding) return cm;

    std::vector<Vec2> vertices = GetVertices(c1);
    Vec2 center = c2->parent->transform.position;
    cm.points = GetPolygonCircleContacts(vertices, center);

    return cm;
}

CollisionManifold Manifold::GenBoxBox(Collider* c1, Collider* c2)
{
    Collision collision = SAT::BoxBox(c1, c2);
    CollisionManifold cm = { collision };

    if (!collision.isColliding) return cm;

    std::vector<Vec2> vertices1 = GetVertices(c1);
    std::vector<Vec2> vertices2 = GetVertices(c2);
    cm.points = GetPolygonContacts(vertices1, vertices2, collision.normal);

    return cm;
}

CollisionManifold Manifold::GenPolyCircle(Collider* c1, Collider* c2)
{
    Collision collision = SAT::PolygonCircle(c1, c2);
    CollisionManifold cm = { collision };

    if (!collision.isColliding) return cm;

    std::vector<Vec2> vertices = GetVertices(c1);
    Vec2 center = c2->parent->transform.position;
    cm.points = GetPolygonCircleContacts(vertices, center);

    return cm;  
}

CollisionManifold Manifold::GenPolyBox(Collider* c1, Collider* c2)
{
    Collision collision = SAT::PolygonBox(c1, c2);
    CollisionManifold cm = { collision };

    if (!collision.isColliding) return cm;

    std::vector<Vec2> vertices1 = GetVertices(c1);
    std::vector<Vec2> vertices2 = GetVertices(c2);
    cm.points = GetPolygonContacts(vertices1, vertices2, collision.normal);

    return cm;
}

CollisionManifold Manifold::GenPolyPoly(Collider* c1, Collider* c2)
{
    Collision collision = SAT::PolygonPolygon(c1, c2);
    CollisionManifold cm = { collision };

    if (!collision.isColliding) return cm;

    std::vector<Vec2> vertices1 = GetVertices(c1);
    std::vector<Vec2> vertices2 = GetVertices(c2);
    cm.points = GetPolygonContacts(vertices1, vertices2, collision.normal);

    return cm;
}

std::vector<Vec2> Manifold::GetPolygonCircleContacts(std::vector<Vec2> vertices1, Vec2 center)
{
    std::vector<Vec2> points;
    std::vector<Edge> edges = GetEdges(vertices1);

    float minDistSq = INFINITY;
    Vec2 closestPoint;

    for (int i = 0; i < edges.size(); i++)
    {
        Vec2 cLine = center - edges[i].p1;
        float t = cLine.Dot(edges[i].line) / edges[i].line.MagSq();
        
        t = std::max(0.0f, std::min(1.0f, t));
        
        Vec2 nearest = edges[i].p1 + (edges[i].line * t);
        
        Vec2 distVector = center - nearest;
        float distSq = distVector.MagSq();

        if (distSq < minDistSq)
        {
            minDistSq = distSq;
            closestPoint = nearest;
        }
    }

    points.push_back(closestPoint);
    return points;
}

std::vector<Vec2> Manifold::GetPolygonContacts(std::vector<Vec2> vertices1, std::vector<Vec2> vertices2, Vec2 normal)
{
    std::vector<Vec2> points;
    
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

    std::vector<Vec2> clip1;
    float d1 = refDir.Dot(incident.p1 - reference.p1);
    float d2 = refDir.Dot(incident.p2 - reference.p1);

    if (d1 >= 0.0f) clip1.push_back(incident.p1);
    if (d2 >= 0.0f) clip1.push_back(incident.p2);

    if (d1 * d2 < 0.0f) 
    {
        float alpha = d1 / (d1 - d2);
        clip1.push_back(incident.p1 + (incident.p2 - incident.p1) * alpha);
    }
    if (clip1.size() < 2) return points;

    std::vector<Vec2> clip2;
    float d3 = (refDir * -1).Dot(clip1[0] - reference.p2);
    float d4 = (refDir * -1).Dot(clip1[1] - reference.p2);

    if (d3 >= 0.0f) clip2.push_back(clip1[0]);
    if (d4 >= 0.0f) clip2.push_back(clip1[1]);

    if (d3 * d4 < 0.0f) 
    {
        float alpha = d3 / (d3 - d4);
        clip2.push_back(clip1[0] + (clip1[1] - clip1[0]) * alpha);
    }
    if (clip2.size() < 2) return points;

    Vec2 refOutNormal = Vec2(reference.line.y, -reference.line.x).Norm();
    
    for (const auto& p : clip2)
    {
        float separation = refOutNormal.Dot(p - reference.p1);
        
        if (separation <= 0.0f) 
        {
            points.push_back(p);
        }
    }

    return points;    
}

Edge Manifold::GetSupportFace(std::vector<Vec2> vertices, Vec2 normal)
{
    float maxDot = -INFINITY;
    int index = 0;

    for (int i = 0; i < vertices.size(); i++)
    {
        float dot = vertices[i].Dot(normal);
        if (dot > maxDot)
        {
            index = i;
            maxDot = dot;
        }
    }

    Vec2 spear = vertices[index];
    Vec2 prev = vertices[(index - 1 + vertices.size()) % vertices.size()];
    Vec2 next = vertices[(index + 1) % vertices.size()];

    Vec2 v1 = (spear - prev).Norm();
    Vec2 v2 = (spear - next).Norm();

    if (std::abs(v1.Dot(normal)) <= std::abs(v2.Dot(normal)))
        return { prev, spear, v1 };
    else
        return { spear, next, (next - spear).Norm() };
}

std::vector<Edge> Manifold::GetEdges(std::vector<Vec2> vertices)
{
    std::vector<Edge> edges;

    for(int i = 0; i < vertices.size(); i++)
    {
        Vec2 p1 = vertices[i];
        Vec2 p2 = vertices[(i + 1) % vertices.size()];
        edges.push_back( { p1, p2, p2 - p1} );
    }

    return edges;
}

std::vector<Vec2> Manifold::GetVertices(Collider* c)
{
    Vec2 worldPosition = c->parent->transform.position;
    float rotation = c->parent->transform.rotation;

    std::vector<Vec2> vertices;

    switch (c->GetType())
    {
        case ColliderType::CIRCLE:
            vertices.push_back(Vec2(0,0));
            break;

        case ColliderType::BOX:
        {
            Vec2 size = static_cast<BoxCollider*>(c)->size;
            float x = size.x / 2.0f;
            float y = size.y / 2.0f;

            vertices.push_back(Vec2(-x, -y));
            vertices.push_back(Vec2(x, -y));
            vertices.push_back(Vec2(x, y));
            vertices.push_back(Vec2(-x, y));
            break;
        }

        case ColliderType::POLYGON:
            vertices = static_cast<PolygonCollider*>(c)->vertices;
            break;

        default:
            break;
    }

    for (int i = 0; i < vertices.size(); i++)
    {
        Vec2 vertex = vertices[i];
        float x = vertex.x * std::cos(rotation) - vertex.y * std::sin(rotation);
        float y = vertex.x * std::sin(rotation) + vertex.y * std::cos(rotation);
        vertices[i] = Vec2(x + worldPosition.x, y + worldPosition.y);                
    }

    return vertices;
}