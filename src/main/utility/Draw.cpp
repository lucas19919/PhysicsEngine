#include "main/utility/Draw.h"
#include "main/components/Collider.h"
#include "main/components/Renderer.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include <cmath>

void Render(GameObject *obj)
{
    if (obj->GetRenderer() == nullptr) return;

    RigidBody* rb = obj->GetRigidBody();
    Renderer *r = obj->GetRenderer();
    Shape shape = r->GetShape();

    //debugging for sleeping states
    bool debug = false;
    if (debug == true)
    {
        if (rb->isSleeping && rb->isSurrounded)
            shape.color = BLUE; // both sleeping and surrounded
        else if (rb->isSurrounded)
            shape.color = GREEN; // surrunded
        else if (rb->isSleeping)
            shape.color = GRAY; //is sleeping
    }

    switch (shape.form)
    {
    case RenderShape::R_CIRCLE:
    {
        float radius = std::get<float>(shape.scale);
        Vec2 pos = obj->transform.position;
        float rot = obj->transform.rotation;

        DrawCircle(pos.x, pos.y, radius, shape.color);
        
        DrawRing({ pos.x, pos.y }, radius - 2.0f, radius, 0.0f, 360.0f, 36, BLACK);
        break;
    }
    case RenderShape::R_BOX:
    {
        Vec2 size = std::get<Vec2>(shape.scale);
        
        DrawRectanglePro(
            Rectangle{ obj->transform.position.x, obj->transform.position.y, size.x, size.y },
            { size.x / 2.0f, size.y / 2.0f },
            obj->transform.rotation * RAD2DEG,
            shape.color
        );        

        Array<20> vertices = r->UpdateWorldCoordinates(obj->transform.position, obj->transform.rotation);
        
        for (size_t i = 0; i < vertices.size(); i++) 
        {
            Vec2 p1 = vertices[i];
            Vec2 p2 = vertices[(i + 1) % vertices.size()];
            DrawLineEx({ p1.x, p1.y }, { p2.x, p2.y }, 2.0f, BLACK);
        }
        break;
    }
    case RenderShape::R_POLYGON:
    {
        Array<20> vertices = r->UpdateWorldCoordinates(obj->transform.position, obj->transform.rotation);
        int vertexCount = vertices.size();
        
        if (vertexCount < 3) break; 

        std::vector<Vector2> raylibVerts(vertexCount);
        for (int i = 0; i < vertexCount; i++) 
        {
            raylibVerts[i] = { vertices[i].x, vertices[i].y }; 
        }

        for (int i = 1; i < vertexCount - 1; i++) 
        {
            DrawTriangle(raylibVerts[0], raylibVerts[i + 1], raylibVerts[i], shape.color);
        }
        
        for (int i = 0; i < vertexCount; i++) 
        {
            DrawLineEx(raylibVerts[i], raylibVerts[(i + 1) % vertexCount], 2.0f, BLACK);
        }
        break;
    }
    default:
        break;
    }
}