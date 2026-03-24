#include "main/utility/Draw.h"
#include "main/components/Collider.h"
#include "main/components/Renderer.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"

void Render(GameObject *obj)
{
    if (obj->GetRenderer() == nullptr) return;

    Renderer *r = obj->GetRenderer();
    Shape shape = r->GetShape();

    switch (shape.form)
    {
    case RenderShape::R_CIRCLE:
        DrawCircle(obj->transform.position.x, obj->transform.position.y, std::get<float>(shape.scale), shape.color);
        break;
    case RenderShape::R_BOX:
    {
        Vec2 size = std::get<Vec2>(shape.scale);
        DrawRectanglePro(
            Rectangle{ obj->transform.position.x, obj->transform.position.y, size.x, size.y },
            { size.x / 2.0f, size.y / 2.0f },
            obj->transform.rotation * RAD2DEG,
            shape.color
        );        

        break;
    }
    case RenderShape::R_POLYGON:
    {
        std::vector<Vec2> vertices = r->UpdateWorldCoordinates(obj->transform.position, obj->transform.rotation);
        int vertexCount = vertices.size();
        if (vertexCount < 3) break; 

        std::vector<Vector2> raylibVerts(vertexCount);
        for (int i = 0; i < vertexCount; i++) {
            raylibVerts[i] = { vertices[i].x, vertices[i].y }; 
        }

        DrawTriangleFan(raylibVerts.data(), vertexCount, shape.color);
        break;
    }
    default:
        break;
    }
}