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
#include "main/editor/EditorState.h"
#include <cmath>
#include "math/RotationMatrix.h"
#include "raymath.h"
#include "external/imgui/imgui.h"

inline Vector2 ToScreen(Vec2 pos) { return { pos.x * Config::MeterToPixel, pos.y * Config::MeterToPixel }; }
inline float ToScreen(float val) { return val * Config::MeterToPixel; }

void Render(World& world, const EditorCamera& camera)
{
    GameObject* selected = EditorState::Get().GetSelected();
    float zoom = camera.GetRaylibCamera().zoom;
    float thickness = 4.0f / zoom; 

    Vec2 worldSize = world.GetWorldSize();
    Vector2 screenWorldSize = ToScreen(worldSize);
    
    // Draw background
    DrawRectangleV({ -screenWorldSize.x / 2.0f, -screenWorldSize.y / 2.0f }, screenWorldSize, WHITE);
    
    // Draw thick border completely outside the world
    float borderThickness = 10.0f;
    Rectangle borderRect = { 
        (-screenWorldSize.x / 2.0f) - borderThickness / 2.0f, 
        (-screenWorldSize.y / 2.0f) - borderThickness / 2.0f, 
        screenWorldSize.x + borderThickness, 
        screenWorldSize.y + borderThickness 
    };
    DrawRectangleLinesEx(borderRect, borderThickness, BLACK);

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

        // --- Debug Visualizations for GameObjects ---
        if (Config::drawAABB && obj->c) {
            BBox bounds = obj->c->GetBounds();
            Vector2 min = ToScreen(bounds.min);
            Vector2 max = ToScreen(bounds.max);
            DrawRectangleLinesEx({min.x, min.y, max.x - min.x, max.y - min.y}, 1.0f, GREEN);
        }

        if (obj->rb) {
            if (Config::drawVelocity) {
                Vector2 start = ToScreen(obj->transform.position);
                // Scale velocity vector (0.2x)
                Vector2 end = ToScreen(obj->transform.position + obj->rb->GetVelocity() * 0.2f);
                DrawLineEx(start, end, 2.0f, BLUE);
                DrawCircleV(end, 3.0f, BLUE);
            }
            if (Config::drawAcceleration) {
                Vector2 start = ToScreen(obj->transform.position);
                // Scale acceleration vector (0.05x) to handle high gravity/forces
                Vector2 end = ToScreen(obj->transform.position + obj->rb->GetAcceleration() * 0.05f);
                DrawLineEx(start, end, 2.0f, RED);
                DrawCircleV(end, 3.0f, RED);
            }
        }
    }

    // --- Debug Visualizations for Contacts ---
    const auto& contacts = world.GetContacts();
    for (const auto& contact : contacts) {
        if (Config::drawNormals) {
            // Draw normal at each contact point (0.2m long)
            for (int i = 0; i < contact.pointCount; i++) {
                Vector2 start = ToScreen(contact.points[i]);
                Vector2 end = ToScreen(contact.points[i] + contact.normal * 0.2f);
                DrawLineEx(start, end, 1.0f, YELLOW);
            }
        }
        if (Config::drawContactPoints) {
            for (int i = 0; i < contact.pointCount; i++) {
                Vector2 p = ToScreen(contact.points[i]);
                DrawCircleV(p, 4.0f, RED);
            }
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

    if (selected && selected->c)
    {
        if (selected->transform.isDirty)
        {
            selected->c->UpdateCache(selected->transform);
        }

        if (selected->c->GetType() == ColliderType::CIRCLE)
        {
            float radius = ToScreen(static_cast<CircleCollider*>(selected->c)->radius);
            Vector2 pos = ToScreen(selected->transform.position);
            DrawRing(pos, radius, radius + thickness, 0.0f, 360.0f, 36, ORANGE);
        }
        else
        {
            const Array<20>& vertices = selected->c->GetVertices();
            for (size_t i = 0; i < vertices.Size(); i++)
            {
                Vector2 p1 = ToScreen(vertices[i]);
                Vector2 p2 = ToScreen(vertices[(i + 1) % vertices.Size()]);
                DrawLineEx(p1, p2, thickness, ORANGE);
            }
        }
    }
}

void GizmoRender(const EditorCamera& camera)
{
    EditorState& state = EditorState::Get();
    GameObject* selected = state.GetSelected();
    if (!selected) return;

    Vec2 origin = camera.WorldToScreenPixels(selected->transform.position);
    Vector2 originV = { origin.x, origin.y };
    GizmoType type = state.GetGizmoType();
    GizmoAxis hovered = state.GetHoveredAxis();
    GizmoAxis active = state.GetActiveAxis();

    float handleLength = 100.0f;
    float thickness = 4.0f;

    switch (type)
    {
        case GizmoType::TRANSLATE :
        {
            Color colorX = (hovered == GizmoAxis::X || active == GizmoAxis::X) ? YELLOW : RED;
            Color colorY = (hovered == GizmoAxis::Y || active == GizmoAxis::Y) ? YELLOW : GREEN;
            Color colorBoth = (hovered == GizmoAxis::BOTH || active == GizmoAxis::BOTH) ? YELLOW : BLUE;

            // X Axis
            DrawLineEx(originV, { originV.x + handleLength, originV.y }, thickness, colorX);
            DrawTriangle({ originV.x + handleLength + 15, originV.y }, 
                        { originV.x + handleLength, originV.y - 7 }, 
                        { originV.x + handleLength, originV.y + 7 }, colorX);

            // Y Axis
            DrawLineEx(originV, { originV.x, originV.y + handleLength }, thickness, colorY);
            DrawTriangle({ originV.x, originV.y + handleLength + 15 }, 
                        { originV.x + 7, originV.y + handleLength }, 
                        { originV.x - 7, originV.y + handleLength }, colorY);

            // Center handle
            DrawRectangleV({ originV.x - 8, originV.y - 8 }, { 16, 16 }, colorBoth);
            DrawRectangleLinesEx({ originV.x - 8, originV.y - 8, 16, 16 }, 1.0f, BLACK);

            break;
        }
        case GizmoType::ROTATE :
        {
            float radius = 80.0f;
            Color colorBoth = (hovered == GizmoAxis::BOTH || active == GizmoAxis::BOTH) ? YELLOW : BLUE;
            DrawCircleLinesV(originV, radius, colorBoth);
            DrawCircleV(originV, 5.0f, colorBoth);
            
            // Draw a line to current mouse if active
            if (active == GizmoAxis::BOTH)
            {
                Vector2 m = GetMousePosition();
                DrawLineV(originV, m, LIGHTGRAY);
            }

            break;
        }
        case GizmoType::SCALE :
        {

            break;
        }
        default:
            break;
    }
}

void GizmoUpdate(const EditorCamera& camera)
{
    EditorState& state = EditorState::Get();
    // Use viewport-relative mouse pos
    Vec2 mousePos = state.GetViewportMousePos();
    GameObject* selected = state.GetSelected();
    if (!selected)
    {
        state.SetActiveAxis(GizmoAxis::NONE);
        return;
    }

    Vec2 origin = camera.WorldToScreenPixels(selected->transform.position);
    float handleLength = 100.0f;

    GizmoType type = state.GetGizmoType();
    
    // Hover Detection
    GizmoAxis hovered = GizmoAxis::NONE;
    if (type == GizmoType::TRANSLATE)
    {
        Vector2 mPos = { mousePos.x, mousePos.y };
        Vector2 oPos = { origin.x, origin.y };
        // Forgiving hitboxes: using slightly larger areas and including the tips
        // Center handle
        if (CheckCollisionPointRec(mPos, { oPos.x - 15, oPos.y - 15, 30, 30 }))
            hovered = GizmoAxis::BOTH;
        // X Axis (includes the triangle tip)
        else if (CheckCollisionPointRec(mPos, { oPos.x, oPos.y - 15, handleLength + 40, 30 }))
            hovered = GizmoAxis::X;
        // Y Axis (includes the triangle tip)
        else if (CheckCollisionPointRec(mPos, { oPos.x - 15, oPos.y, 30, handleLength + 40 }))
            hovered = GizmoAxis::Y;
    }
    else if (type == GizmoType::ROTATE)
    {
        Vector2 mPos = { mousePos.x, mousePos.y };
        Vector2 oPos = { origin.x, origin.y };
        float radius = 80.0f;
        float dist = Vector2Distance(mPos, oPos);
        if (std::abs(dist - radius) < 10.0f || dist < 10.0f)
            hovered = GizmoAxis::BOTH;
    }
    
    state.SetHoveredAxis(hovered);

    // Interaction
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        state.SetActiveAxis(hovered);
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        state.SetActiveAxis(GizmoAxis::NONE);
    }

    if (state.GetActiveAxis() != GizmoAxis::NONE && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        Vector2 d = GetMouseDelta();
        Vec2 delta(d.x, d.y);
        if (delta.x != 0 || delta.y != 0)
        {
            float zoom = camera.GetRaylibCamera().zoom;
            Vec2 worldDelta(
                (delta.x / zoom) * Config::PixelToMeter,
                (delta.y / zoom) * Config::PixelToMeter
            );

            if (type == GizmoType::TRANSLATE)
            {
                if (state.GetActiveAxis() == GizmoAxis::X)
                    selected->transform.SetPosition(selected->transform.position + Vec2(worldDelta.x, 0));
                else if (state.GetActiveAxis() == GizmoAxis::Y)
                    selected->transform.SetPosition(selected->transform.position + Vec2(0, worldDelta.y));
                else if (state.GetActiveAxis() == GizmoAxis::BOTH)
                    selected->transform.SetPosition(selected->transform.position + worldDelta);
            }
            else if (type == GizmoType::ROTATE)
            {
                Vector2 mPos = { mousePos.x, mousePos.y };
                Vector2 oPos = { origin.x, origin.y };
                Vector2 currentDir = Vector2Subtract(mPos, oPos);
                Vector2 prevPos = { mousePos.x - delta.x, mousePos.y - delta.y };
                Vector2 prevDir = Vector2Subtract(prevPos, oPos);
                
                if (Vector2Length(currentDir) > 0.1f && Vector2Length(prevDir) > 0.1f)
                {
                    float angle1 = atan2f(currentDir.y, currentDir.x);
                    float angle2 = atan2f(prevDir.y, prevDir.x);
                    float diff = angle1 - angle2;
                    
                    // Handle wrap around
                    if (diff > PI) diff -= 2.0f * PI;
                    if (diff < -PI) diff += 2.0f * PI;
                    
                    selected->transform.SetRotation(selected->transform.rotation + diff);
                }
            }
        }
    }
}