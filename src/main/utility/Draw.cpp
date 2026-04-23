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
#include <cmath>
#include "math/RotationMatrix.h"

void Render(World& world)
{
    for (const auto& objPtr : world.GetGameObjects())
    {
        GameObject* obj = objPtr.get();
        if (obj->GetComponent<Renderer>() == nullptr) continue;

        Renderer *r = obj->GetComponent<Renderer>();
        Shape shape = r->GetShape();

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
            
            for (size_t i = 0; i < vertices.Size(); i++) 
            {
                Vec2 p1 = vertices[i];
                Vec2 p2 = vertices[(i + 1) % vertices.Size()];
                DrawLineEx({ p1.x, p1.y }, { p2.x, p2.y }, 2.0f, BLACK);
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

    for (auto& c : world.GetConstraints())
    {
        if (c->GetType() == ConstraintType::DISTANCE)
        {
            DistanceConstraint* dc = static_cast<DistanceConstraint*>(c.get());

            RotMatrix rot1(dc->anchor->transform.rotation);
            Vec2 rAnchorOffset = rot1.Rotate(dc->anchorOffset);
            RotMatrix rot2(dc->attached->transform.rotation);
            Vec2 rAttachedOffset = rot2.Rotate(dc->attachedOffset);

            Vec2 pos1 = dc->anchor->transform.position + rAnchorOffset;
            Vec2 pos2 = dc->attached->transform.position + rAttachedOffset;

            DrawLineEx({ pos1.x, pos1.y }, { pos2.x, pos2.y }, 2.0f, DARKGRAY);
        }
        if (c->GetType() == ConstraintType::PIN)
        {
            PinConstraint* pc = static_cast<PinConstraint*>(c.get());
            for (const auto& att : pc->attachments)
            {
                Vec2 pos = pc->position; //center of pin

                bool fixedX = pc->fixedX;
                bool fixedY = pc->fixedY;

                if (fixedX && fixedY)
                {
                    DrawTriangle( { pos.x, pos.y }, { pos.x - 25.0f, pos.y + 25.0f }, { pos.x + 25.0f, pos.y + 25.0f }, BLACK);

                    for (int i = 0; i < 9; i++)
                    {
                        DrawLine(pos.x - 25.0f + i * 5.0f, pos.y + 30.0f, pos.x - 20.0f + i * 5.0f, pos.y + 25.0f, BLACK);
                    }
                
                }
                else if (fixedX)
                {
                    DrawTriangle( { pos.x, pos.y }, { pos.x - 25.0f, pos.y - 25.0f }, { pos.x - 25.0f, pos.y + 25.0f }, BLACK);
                    DrawLine(pos.x - 30.0f, pos.y - 25.0f, pos.x - 30.0f, pos.y + 25.0f, BLACK);
                }
                else if (fixedY)
                {
                    DrawTriangle( { pos.x, pos.y }, { pos.x - 25.0f, pos.y + 25.0f }, { pos.x + 25.0f, pos.y + 25.0f }, BLACK);
                    DrawLine(pos.x - 25.0f, pos.y + 30.0f, pos.x + 25.0f, pos.y + 30.0f, BLACK);
                }

                DrawCircle(pos.x, pos.y, 5.0f, BLACK);
                DrawCircleLines(pos.x, pos.y, 5.0f, DARKGRAY);
            }
        }
        if (c->GetType() == ConstraintType::MOTOR)
        {
            MotorConstraint* mc = static_cast<MotorConstraint*>(c.get());
            Vec2 r = RotMatrix(mc->rotor->transform.rotation).Rotate(mc->localPosition);
            Vec2 pos = mc->rotor->transform.position + r;

            DrawCircle(pos.x, pos.y, 10.0f, RED);
            DrawCircleLines(pos.x, pos.y, 10.0f, BLACK);
        }

        if (c->GetType() == ConstraintType::JOINT)
        {
            JointConstraint* jc = static_cast<JointConstraint*>(c.get());
            Vec2 pos = jc->position;
            DrawCircle(pos.x, pos.y, 5.0f, DARKGRAY);
            DrawCircleLines(pos.x, pos.y, 5.0f, BLACK);
        }
    }
}