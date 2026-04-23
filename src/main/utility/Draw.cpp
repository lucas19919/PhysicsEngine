#include "main/utility/Draw.h"
#include "main/components/Collider.h"
#include "main/components/Renderer.h"
#include "main/physics/Config.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Motor.h"
#include "main/utility/EditorState.h"
#include <cmath>
#include "math/RotationMatrix.h"

inline Vector2 ToScreen(Vec2 pos) { return { pos.x * Config::MeterToPixel, pos.y * Config::MeterToPixel }; }
inline float ToScreen(float val) { return val * Config::MeterToPixel; }

void Render(World& world, const EditorCamera& camera)
{
    GameObject* selected = EditorState::Get().GetSelected();
    float zoom = camera.GetRaylibCamera().zoom;
    float thickness = 4.0f / zoom; // 4 screen pixels thick

    // Draw World Area
    Vec2 worldSize = world.GetWorldSize();
    Vector2 screenWorldSize = ToScreen(worldSize);
    DrawRectangleV({ -screenWorldSize.x / 2.0f, -screenWorldSize.y / 2.0f }, screenWorldSize, WHITE);

    for (const auto& objPtr : world.GetGameObjects())
    {
        GameObject* obj = objPtr.get();
        if (obj->GetComponent<Renderer>() == nullptr) continue;

        Renderer *r = obj->GetComponent<Renderer>();
        Shape shape = r->GetShape();

        // If selected, draw a highlight first or after. Let's draw after for clarity.

        switch (shape.form)
        {
        case RenderShape::R_CIRCLE:
        {
            float radius = ToScreen(std::get<float>(shape.scale));
            Vector2 pos = ToScreen(obj->transform.position);
            float rot = obj->transform.rotation;

            DrawCircleV(pos, radius, shape.color);
            
            DrawRing(pos, radius - 2.0f, radius, 0.0f, 360.0f, 36, BLACK);
            break;
        }
        case RenderShape::R_BOX:
        {
            Vec2 size = std::get<Vec2>(shape.scale);
            Vector2 screenPos = ToScreen(obj->transform.position);
            Vector2 screenSize = ToScreen(size);
            
            DrawRectanglePro(
                Rectangle{ screenPos.x, screenPos.y, screenSize.x, screenSize.y },
                { screenSize.x / 2.0f, screenSize.y / 2.0f },
                obj->transform.rotation * RAD2DEG,
                shape.color
            );        

            Array<20> vertices = r->UpdateWorldCoordinates(obj->transform.position, obj->transform.rotation);
            
            for (size_t i = 0; i < vertices.Size(); i++) 
            {
                Vector2 p1 = ToScreen(vertices[i]);
                Vector2 p2 = ToScreen(vertices[(i + 1) % vertices.Size()]);
                DrawLineEx(p1, p2, 2.0f, BLACK);
            }
            break;
        }
        case RenderShape::R_POLYGON:
        {
            Array<20> vertices = r->UpdateWorldCoordinates(obj->transform.position, obj->transform.rotation);
            int vertexCount = vertices.Size();
            
            if (vertexCount < 3) break; 

            std::vector<Vector2> raylibVerts(vertexCount);
            for (int i = 0; i < vertexCount; i++) 
            {
                raylibVerts[i] = ToScreen(vertices[i]); 
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

    for (auto& c : world.GetConstraints())
    {
        if (c->GetType() == ConstraintType::DISTANCE)
        {
            DistanceConstraint* dc = static_cast<DistanceConstraint*>(c.get());

            RotMatrix rot1(dc->anchor->transform.rotation);
            Vec2 rAnchorOffset = rot1.Rotate(dc->anchorOffset);
            RotMatrix rot2(dc->attached->transform.rotation);
            Vec2 rAttachedOffset = rot2.Rotate(dc->attachedOffset);

            Vector2 pos1 = ToScreen(dc->anchor->transform.position + rAnchorOffset);
            Vector2 pos2 = ToScreen(dc->attached->transform.position + rAttachedOffset);

            DrawLineEx(pos1, pos2, 2.0f, DARKGRAY);
        }
        if (c->GetType() == ConstraintType::PIN)
        {
            PinConstraint* pc = static_cast<PinConstraint*>(c.get());
            for (const auto& att : pc->attachments)
            {
                Vector2 pos = ToScreen(pc->position); //center of pin

                bool fixedX = pc->fixedX;
                bool fixedY = pc->fixedY;

                if (fixedX && fixedY)
                {
                    DrawTriangle( pos, { pos.x - 25.0f, pos.y + 25.0f }, { pos.x + 25.0f, pos.y + 25.0f }, BLACK);

                    for (int i = 0; i < 9; i++)
                    {
                        DrawLine(pos.x - 25.0f + i * 5.0f, pos.y + 30.0f, pos.x - 20.0f + i * 5.0f, pos.y + 25.0f, BLACK);
                    }
                
                }
                else if (fixedX)
                {
                    DrawTriangle( pos, { pos.x - 25.0f, pos.y - 25.0f }, { pos.x - 25.0f, pos.y + 25.0f }, BLACK);
                    DrawLine(pos.x - 30.0f, pos.y - 25.0f, pos.x - 30.0f, pos.y + 25.0f, BLACK);
                }
                else if (fixedY)
                {
                    DrawTriangle( pos, { pos.x - 25.0f, pos.y + 25.0f }, { pos.x + 25.0f, pos.y + 25.0f }, BLACK);
                    DrawLine(pos.x - 25.0f, pos.y + 30.0f, pos.x + 25.0f, pos.y + 30.0f, BLACK);
                }

                DrawCircleV(pos, 5.0f, BLACK);
                DrawCircleLines(pos.x, pos.y, 5.0f, DARKGRAY);
            }
        }
        if (c->GetType() == ConstraintType::MOTOR)
        {
            MotorConstraint* mc = static_cast<MotorConstraint*>(c.get());
            Vec2 r = RotMatrix(mc->rotor->transform.rotation).Rotate(mc->localPosition);
            Vector2 pos = ToScreen(mc->rotor->transform.position + r);

            DrawCircleV(pos, 10.0f, RED);
            DrawCircleLines(pos.x, pos.y, 10.0f, BLACK);
        }

        if (c->GetType() == ConstraintType::JOINT)
        {
            JointConstraint* jc = static_cast<JointConstraint*>(c.get());
            Vector2 pos = ToScreen(jc->position);
            DrawCircleV(pos, 5.0f, DARKGRAY);
            DrawCircleLines(pos.x, pos.y, 5.0f, BLACK);
        }
    }

    // Draw selection highlight at the very end to ensure it is on top
    if (selected && selected->c)
    {
        if (selected->c->GetType() == ColliderType::CIRCLE)
        {
            float radius = ToScreen(static_cast<CircleCollider*>(selected->c)->radius);
            Vector2 pos = ToScreen(selected->transform.position);
            // Dynamic thickness to keep highlight consistent on screen, no gap
            DrawRing(pos, radius, radius + thickness, 0.0f, 360.0f, 36, ORANGE);
        }
        else
        {
            const Array<20>& vertices = selected->c->GetVertices();
            for (size_t i = 0; i < vertices.Size(); i++)
            {
                Vector2 p1 = ToScreen(vertices[i]);
                Vector2 p2 = ToScreen(vertices[(i + 1) % vertices.Size()]);
                // Dynamic thickness for lines
                DrawLineEx(p1, p2, thickness, ORANGE);
            }
        }
    }
}