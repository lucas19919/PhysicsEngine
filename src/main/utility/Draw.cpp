#include "main/utility/Draw.h"
#include "main/World.h"
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
    std::string selectedGroup = EditorState::Get().GetSelectedGroup();
    float zoom = camera.GetRaylibCamera().zoom;
    float thickness = 4.0f / zoom; 

    Vec2 worldSize = world.GetWorldSize();
    Vector2 screenWorldSize = ToScreen(worldSize);
    
    // background
    DrawRectangleV({ -screenWorldSize.x / 2.0f, -screenWorldSize.y / 2.0f }, screenWorldSize, WHITE);
    
    // spatial hash (render behind objects)
    if (Config::spatialHashMode != Config::SpatialHashMode::None) {
        float cellSize = Config::spatialHashCellSize;
        float halfW = worldSize.x * 0.5f;
        float halfH = worldSize.y * 0.5f;

        float lineThickness = 1.0f / zoom;
        if (lineThickness < 1.0f) lineThickness = 1.0f;

        float startX = std::floor(-halfW / cellSize) * cellSize;
        float endX = std::ceil(halfW / cellSize) * cellSize;
        float startY = std::floor(-halfH / cellSize) * cellSize;
        float endY = std::ceil(halfH / cellSize) * cellSize;

        for (float x = startX; x <= endX; x += cellSize) {
            DrawLineEx(ToScreen(Vec2(x, -halfH)), ToScreen(Vec2(x, halfH)), lineThickness, Fade(DARKGRAY, 0.4f));
        }
        for (float y = startY; y <= endY; y += cellSize) {
            DrawLineEx(ToScreen(Vec2(-halfW, y)), ToScreen(Vec2(halfW, y)), lineThickness, Fade(DARKGRAY, 0.4f));
        }

        if (Config::spatialHashMode == Config::SpatialHashMode::ActiveCells) {
            const auto& grid = world.GetSpatialGrid();
            for (const auto& pair : grid) {
                for (GameObject* obj : pair.second) {
                    if (!obj->c) continue;
                    BBox b = obj->c->GetBounds();
                    int minX = (int)std::floor(b.min.x / cellSize);
                    int maxX = (int)std::floor(b.max.x / cellSize);
                    int minY = (int)std::floor(b.min.y / cellSize);
                    int maxY = (int)std::floor(b.max.y / cellSize);
                    for (int x = minX; x <= maxX; x++) {
                        for (int y = minY; y <= maxY; y++) {
                            Rectangle cellRect = { x * cellSize * Config::MeterToPixel, y * cellSize * Config::MeterToPixel, cellSize * Config::MeterToPixel, cellSize * Config::MeterToPixel };
                            DrawRectangleRec(cellRect, Fade(PURPLE, 0.2f));
                            DrawRectangleLinesEx(cellRect, 2.0f / zoom, Fade(PURPLE, 0.5f));
                        }
                    }
                }
            }
        }
    }

    // border
    float borderThickness = 10.0f;
    Rectangle borderRect = { 
        (-screenWorldSize.x / 2.0f) - borderThickness / 2.0f, 
        (-screenWorldSize.y / 2.0f) - borderThickness / 2.0f, 
        screenWorldSize.x + borderThickness, 
        screenWorldSize.y + borderThickness 
    };
    DrawRectangleLinesEx(borderRect, borderThickness, DARKGRAY);

    for (const auto& objPtr : world.GetGameObjects())
    {
        GameObject* obj = objPtr.get();
        if (obj->GetComponent<Renderer>() == nullptr) continue;

        Renderer *r = obj->GetComponent<Renderer>();
        Shape shape = r->GetShape();

        // highlight color if in selected group
        Color renderColor = shape.color;
        if (!selectedGroup.empty() && obj->GetGroupName() == selectedGroup) {
            renderColor = ORANGE;
        }

        switch (shape.form)
        {
        case RenderShape::R_CIRCLE:
        {
            float radius = ToScreen(std::get<float>(shape.scale));
            Vector2 pos = ToScreen(obj->transform.position);
            DrawCircleV(pos, radius, renderColor);
            DrawRing(pos, radius - 2.0f, radius, 0.0f, 360.0f, 36, BLACK);
            
            // Draw rotation line
            Vector2 end = { 
                pos.x + std::cos(obj->transform.rotation) * radius, 
                pos.y + std::sin(obj->transform.rotation) * radius 
            };
            DrawLineEx(pos, end, 2.0f / zoom, BLACK);
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
                renderColor
            );        
            Array<20> vertices = r->UpdateWorldCoordinates(obj->transform.position, obj->transform.rotation);
            for (size_t i = 0; i < vertices.Size(); i++) 
                DrawLineEx(ToScreen(vertices[i]), ToScreen(vertices[(i + 1) % vertices.Size()]), 2.0f, BLACK);
            break;
        }
        case RenderShape::R_POLYGON:
        {
            Array<20> vertices = r->UpdateWorldCoordinates(obj->transform.position, obj->transform.rotation);
            int vertexCount = vertices.Size();
            if (vertexCount < 3) break; 
            std::vector<Vector2> raylibVerts(vertexCount);
            for (int i = 0; i < vertexCount; i++) raylibVerts[i] = ToScreen(vertices[i]); 
            for (int i = 1; i < vertexCount - 1; i++) DrawTriangle(raylibVerts[0], raylibVerts[i + 1], raylibVerts[i], renderColor);
            for (int i = 0; i < vertexCount; i++) DrawLineEx(raylibVerts[i], raylibVerts[(i + 1) % vertexCount], 2.0f, BLACK);
            break;
        }
        default: break;
        }

        // debug
        if (Config::drawAABB && obj->c) {
            BBox b = obj->c->GetBounds();
            Vector2 min = ToScreen(b.min);
            Vector2 max = ToScreen(b.max);
            DrawRectangleLinesEx({min.x, min.y, max.x - min.x, max.y - min.y}, 1.0f / zoom, GREEN);
        }

        if (obj->rb) {
            if (Config::drawVelocity) {
                Vector2 start = ToScreen(obj->transform.position);
                Vector2 end = ToScreen(obj->transform.position + obj->rb->GetVelocity() * 0.2f);
                DrawLineEx(start, end, 2.0f / zoom, BLUE);
                DrawCircleV(end, 3.0f / zoom, BLUE);
            }
            if (Config::drawAcceleration) {
                Vector2 start = ToScreen(obj->transform.position);
                Vector2 end = ToScreen(obj->transform.position + obj->rb->GetAcceleration() * 0.05f);
                DrawLineEx(start, end, 2.0f / zoom, ORANGE);
                DrawCircleV(end, 3.0f / zoom, ORANGE);
            }
        }
    }

    const auto& contacts = world.GetContacts();
    for (const auto& contact : contacts) {
        if (Config::drawNormals) {
            for (int i = 0; i < contact.pointCount; i++) {
                Vector2 start = ToScreen(contact.points[i]);
                Vector2 end = ToScreen(contact.points[i] + contact.normal * 0.2f);
                DrawLineEx(start, end, 1.0f / zoom, YELLOW);
            }
        }
        if (Config::drawContactPoints) {
            for (int i = 0; i < contact.pointCount; i++) 
                DrawCircleV(ToScreen(contact.points[i]), 4.0f / zoom, RED);
        }
    }

    for (auto& c : world.GetConstraints())
    {
        if (c->GetType() == ConstraintType::DISTANCE)
        {
            DistanceConstraint* dc = static_cast<DistanceConstraint*>(c.get());
            Vector2 pos1 = ToScreen(dc->anchor->transform.position + RotMatrix(dc->anchor->transform.rotation).Rotate(dc->anchorOffset));
            Vector2 pos2 = ToScreen(dc->attached->transform.position + RotMatrix(dc->attached->transform.rotation).Rotate(dc->attachedOffset));
            DrawLineEx(pos1, pos2, 2.0f / zoom, DARKGRAY);
        }
        if (c->GetType() == ConstraintType::PIN)
        {
            PinConstraint* pc = static_cast<PinConstraint*>(c.get());
            for (const auto& att : pc->attachments)
            {
                Vector2 pos = ToScreen(pc->position); 
                if (pc->fixedX && pc->fixedY)
                {
                    DrawTriangle( pos, { pos.x - 25.0f, pos.y + 25.0f }, { pos.x + 25.0f, pos.y + 25.0f }, BLACK);
                    for (int i = 0; i < 9; i++)
                        DrawLine(pos.x - 25.0f + i * 5.0f, pos.y + 30.0f, pos.x - 20.0f + i * 5.0f, pos.y + 25.0f, BLACK);
                }
                else if (pc->fixedX)
                {
                    DrawTriangle( pos, { pos.x - 25.0f, pos.y - 25.0f }, { pos.x - 25.0f, pos.y + 25.0f }, BLACK);
                    DrawLine(pos.x - 30.0f, pos.y - 25.0f, pos.x - 30.0f, pos.y + 25.0f, BLACK);
                }
                else if (pc->fixedY)
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
            Vector2 pos = ToScreen(mc->rotor->transform.position + RotMatrix(mc->rotor->transform.rotation).Rotate(mc->localPosition));
            DrawCircleV(pos, 10.0f / zoom, RED);
            DrawCircleLines(pos.x, pos.y, 10.0f / zoom, BLACK);
        }
        if (c->GetType() == ConstraintType::JOINT)
        {
            JointConstraint* jc = static_cast<JointConstraint*>(c.get());
            DrawCircleV(ToScreen(jc->position), 5.0f / zoom, DARKGRAY);
            DrawCircleLines(ToScreen(jc->position).x, ToScreen(jc->position).y, 5.0f / zoom, BLACK);
        }
    }

    if (selected && selected->c)
    {
        if (selected->transform.isDirty) selected->c->UpdateCache(selected->transform);
        if (selected->c->GetType() == ColliderType::CIRCLE)
        {
            float radius = ToScreen(static_cast<CircleCollider*>(selected->c)->radius);
            DrawRing(ToScreen(selected->transform.position), radius, radius + thickness, 0.0f, 360.0f, 36, ORANGE);
        }
        else
        {
            const Array<20>& vertices = selected->c->GetVertices();
            for (size_t i = 0; i < vertices.Size(); i++)
                DrawLineEx(ToScreen(vertices[i]), ToScreen(vertices[(i + 1) % vertices.Size()]), thickness, ORANGE);
        }
    }
}

void GizmoRender(World& world, const EditorCamera& camera)
{
    EditorState& state = EditorState::Get();
    GameObject* selected = state.GetSelected();
    std::string selectedGroup = state.GetSelectedGroup();
    
    Vec2 worldPos;
    if (selected) {
        worldPos = selected->transform.position;
    } else if (!selectedGroup.empty()) {
        GeneratorDef* gen = world.GetGenerator(selectedGroup);
        if (!gen) return;
        worldPos = Vec2(gen->startX, gen->startY);
    } else {
        return;
    }

    Vec2 origin = camera.WorldToScreenPixels(worldPos);
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
            DrawLineEx(originV, { originV.x + handleLength, originV.y }, thickness, colorX);
            DrawTriangle({ originV.x + handleLength + 15, originV.y }, { originV.x + handleLength, originV.y - 7 }, { originV.x + handleLength, originV.y + 7 }, colorX);
            DrawLineEx(originV, { originV.x, originV.y + handleLength }, thickness, colorY);
            DrawTriangle({ originV.x, originV.y + handleLength + 15 }, { originV.x + 7, originV.y + handleLength }, { originV.x - 7, originV.y + handleLength }, colorY);
            DrawRectangleV({ originV.x - 8, originV.y - 8 }, { 16, 16 }, colorBoth);
            DrawRectangleLinesEx({ originV.x - 8, originV.y - 8, 16, 16 }, 1.0f, BLACK);
            break;
        }
        case GizmoType::ROTATE :
        {
            if (!selected) break; // Don't rotate generators yet
            float radius = 80.0f;
            Color colorBoth = (hovered == GizmoAxis::BOTH || active == GizmoAxis::BOTH) ? YELLOW : BLUE;
            DrawCircleLinesV(originV, radius, colorBoth);
            DrawCircleV(originV, 5.0f, colorBoth);
            if (active == GizmoAxis::BOTH)
                DrawLineV(originV, { GetMousePosition().x, GetMousePosition().y }, LIGHTGRAY);
            break;
        }
        default: break;
    }
}

void GizmoUpdate(World& world, const EditorCamera& camera)
{
    EditorState& state = EditorState::Get();
    Vec2 mousePos = state.GetViewportMousePos();
    GameObject* selected = state.GetSelected();
    std::string selectedGroup = state.GetSelectedGroup();
    
    Vec2 worldPos;
    GeneratorDef* genDef = nullptr;
    if (selected) {
        worldPos = selected->transform.position;
    } else if (!selectedGroup.empty()) {
        genDef = world.GetGenerator(selectedGroup);
        if (!genDef) return;
        worldPos = Vec2(genDef->startX, genDef->startY);
    } else {
        state.SetActiveAxis(GizmoAxis::NONE);
        return;
    }

    Vec2 origin = camera.WorldToScreenPixels(worldPos);
    float handleLength = 100.0f;
    GizmoType type = state.GetGizmoType();
    
    GizmoAxis hovered = GizmoAxis::NONE;
    if (type == GizmoType::TRANSLATE)
    {
        Vector2 mPos = { mousePos.x, mousePos.y };
        Vector2 oPos = { origin.x, origin.y };
        if (CheckCollisionPointRec(mPos, { oPos.x - 15, oPos.y - 15, 30, 30 })) hovered = GizmoAxis::BOTH;
        else if (CheckCollisionPointRec(mPos, { oPos.x, oPos.y - 15, handleLength + 40, 30 })) hovered = GizmoAxis::X;
        else if (CheckCollisionPointRec(mPos, { oPos.x - 15, oPos.y, 30, handleLength + 40 })) hovered = GizmoAxis::Y;
    }
    else if (type == GizmoType::ROTATE && selected)
    {
        Vector2 mPos = { mousePos.x, mousePos.y };
        Vector2 oPos = { origin.x, origin.y };
        float radius = 80.0f;
        float dist = Vector2Distance(mPos, oPos);
        if (std::abs(dist - radius) < 10.0f || dist < 10.0f) hovered = GizmoAxis::BOTH;
    }
    
    state.SetHoveredAxis(hovered);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) state.SetActiveAxis(hovered);
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) state.SetActiveAxis(GizmoAxis::NONE);

    if (state.GetActiveAxis() != GizmoAxis::NONE && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        Vector2 d = GetMouseDelta();
        Vec2 delta(d.x, d.y);
        if (delta.x != 0 || delta.y != 0)
        {
            float zoom = camera.GetRaylibCamera().zoom;
            Vec2 worldDelta( (delta.x / zoom) * Config::PixelToMeter, (delta.y / zoom) * Config::PixelToMeter );
            if (type == GizmoType::TRANSLATE)
            {
                if (selected) {
                    if (state.GetActiveAxis() == GizmoAxis::X) selected->transform.SetPosition(selected->transform.position + Vec2(worldDelta.x, 0));
                    else if (state.GetActiveAxis() == GizmoAxis::Y) selected->transform.SetPosition(selected->transform.position + Vec2(0, worldDelta.y));
                    else if (state.GetActiveAxis() == GizmoAxis::BOTH) selected->transform.SetPosition(selected->transform.position + worldDelta);
                } else if (genDef) {
                    if (state.GetActiveAxis() == GizmoAxis::X) genDef->startX += worldDelta.x;
                    else if (state.GetActiveAxis() == GizmoAxis::Y) genDef->startY += worldDelta.y;
                    else if (state.GetActiveAxis() == GizmoAxis::BOTH) { genDef->startX += worldDelta.x; genDef->startY += worldDelta.y; }
                    world.RegenerateGenerator(selectedGroup);
                }
            }
            else if (type == GizmoType::ROTATE && selected)
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
                    if (diff > PI) diff -= 2.0f * PI;
                    if (diff < -PI) diff += 2.0f * PI;
                    selected->transform.SetRotation(selected->transform.rotation + diff);
                }
            }
        }
    }
}
