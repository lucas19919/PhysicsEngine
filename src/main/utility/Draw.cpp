#include "main/utility/Draw.h"

#include <cmath>

#include "external/imgui/imgui.h"

#include "main/World.h"
#include "main/components/Collider.h"
#include "main/components/Renderer.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Motor.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/editor/EditorState.h"
#include "main/physics/Config.h"
#include "math/RotationMatrix.h"
#include "raymath.h"

inline Vector2 ToScreen(Vec2 pos) { return { pos.x * Config::MeterToPixel, pos.y * Config::MeterToPixel }; }
inline float ToScreen(float val) { return val * Config::MeterToPixel; }

void Render(World& world, const EditorCamera& camera)
{
    const std::vector<size_t>& selectedObjectIDs = EditorState::Get().GetSelectedObjectIDs();
    std::string selectedGroup = EditorState::Get().GetSelectedGroup();
    const auto& theme = EditorState::Get().GetThemeColors();
    float zoom = camera.GetRaylibCamera().zoom;
    float thickness = 4.0f / zoom; 

    Vec2 worldSize = world.GetWorldSize();
    Vector2 screenWorldSize = ToScreen(worldSize);
    
    // background
    DrawRectangleV({ -screenWorldSize.x / 2.0f, -screenWorldSize.y / 2.0f }, screenWorldSize, WHITE);
    
    // spatial hash
    if (Config::spatialHashMode != Config::SpatialHashMode::None) {
        float cellSize = Config::spatialHashCellSize;
        float halfW = worldSize.x * 0.5f;
        float halfH = worldSize.y * 0.5f;
        
        float screenCellSize = cellSize * Config::MeterToPixel * zoom;
        
        if (screenCellSize > 4.0f) {
            float lineThickness = std::max(1.0f / zoom, 0.5f);
            Color gridColor = theme.gridColor;
            
            if (screenCellSize < 20.0f) {
                float alpha = (screenCellSize - 4.0f) / 16.0f;
                gridColor.a = (unsigned char)(gridColor.a * alpha);
            }

            for (float x = std::floor(-halfW / cellSize) * cellSize; x <= std::ceil(halfW / cellSize) * cellSize; x += cellSize)
                DrawLineEx(ToScreen(Vec2(x, -halfH)), ToScreen(Vec2(x, halfH)), lineThickness, gridColor);
            for (float y = std::floor(-halfH / cellSize) * cellSize; y <= std::ceil(halfH / cellSize) * cellSize; y += cellSize)
                DrawLineEx(ToScreen(Vec2(-halfW, y)), ToScreen(Vec2(halfW, y)), lineThickness, gridColor);
        }
    }

    // border
    float borderThickness = 10.0f;
    DrawRectangleLinesEx({(-screenWorldSize.x / 2.0f) - borderThickness / 2.0f, (-screenWorldSize.y / 2.0f) - borderThickness / 2.0f, screenWorldSize.x + borderThickness, screenWorldSize.y + borderThickness}, borderThickness, theme.borderColor);

    for (const auto& objPtr : world.GetGameObjects())
    {
        GameObject* obj = objPtr.get();
        Renderer *r = obj->GetComponent<Renderer>();
        if (!r) continue;

        bool isSelected = EditorState::Get().IsSelected(obj->GetID());
        bool isGroupSelected = (!selectedGroup.empty() && obj->GetGroupName() == selectedGroup);

        if ((isSelected || isGroupSelected) && obj->c) {
            if (obj->c->GetType() == ColliderType::CIRCLE) {
                float radius = ToScreen(static_cast<CircleCollider*>(obj->c)->radius);
                DrawRing(ToScreen(obj->transform.position), radius, radius + thickness, 0.0f, 360.0f, 36, theme.selectionColor);
            } else {
                const Array<20>& vertices = obj->c->GetVertices();
                for (size_t i = 0; i < vertices.Size(); i++)
                    DrawLineEx(ToScreen(vertices[i]), ToScreen(vertices[(i + 1) % vertices.Size()]), thickness, theme.selectionColor);
            }
        }

        Shape shape = r->GetShape();
        if (shape.form == RenderShape::R_CIRCLE) {
            float radius = ToScreen(std::get<float>(shape.scale));
            Vector2 pos = ToScreen(obj->transform.position);
            DrawCircleV(pos, radius, shape.color);
            DrawRing(pos, radius - 2.0f, radius, 0.0f, 360.0f, 36, BLACK);
            DrawLineEx(pos, {pos.x + std::cos(obj->transform.rotation) * radius, pos.y + std::sin(obj->transform.rotation) * radius}, 2.0f / zoom, BLACK);
        } else if (shape.form == RenderShape::R_BOX) {
            Vec2 size = std::get<Vec2>(shape.scale);
            Vector2 screenPos = ToScreen(obj->transform.position);
            Vector2 screenSize = ToScreen(size);
            DrawRectanglePro(Rectangle{ screenPos.x, screenPos.y, screenSize.x, screenSize.y }, { screenSize.x / 2.0f, screenSize.y / 2.0f }, obj->transform.rotation * RAD2DEG, shape.color);        
            Array<20> vertices = r->UpdateWorldCoordinates(obj->transform.position, obj->transform.rotation);
            for (size_t i = 0; i < vertices.Size(); i++) DrawLineEx(ToScreen(vertices[i]), ToScreen(vertices[(i + 1) % vertices.Size()]), 2.0f, BLACK);
        } else if (shape.form == RenderShape::R_POLYGON) {
            Array<20> vertices = r->UpdateWorldCoordinates(obj->transform.position, obj->transform.rotation);
            if (vertices.Size() >= 3) {
                std::vector<Vector2> rv(vertices.Size());
                for (size_t i = 0; i < vertices.Size(); i++) rv[i] = ToScreen(vertices[i]);
                for (size_t i = 1; i < vertices.Size() - 1; i++) DrawTriangle(rv[0], rv[i+1], rv[i], shape.color);
                for (size_t i = 0; i < vertices.Size(); i++) DrawLineEx(rv[i], rv[(i+1)%vertices.Size()], 2.0f, BLACK);
            }
        }
    }

    const auto& contacts = world.GetContacts();
    for (const auto& contact : contacts) {
        if (Config::drawNormals) for (int i = 0; i < contact.pointCount; i++) DrawLineEx(ToScreen(contact.points[i]), ToScreen(contact.points[i] + contact.normal * 0.2f), 1.0f / zoom, YELLOW);
        if (Config::drawContactPoints) for (int i = 0; i < contact.pointCount; i++) DrawCircleV(ToScreen(contact.points[i]), 4.0f / zoom, RED);
    }

    for (auto& c : world.GetConstraints()) {
        if (c->GetType() == ConstraintType::DISTANCE) {
            DistanceConstraint* dc = static_cast<DistanceConstraint*>(c.get());
            DrawLineEx(ToScreen(dc->anchor->transform.position + RotMatrix(dc->anchor->transform.rotation).Rotate(dc->anchorOffset)), ToScreen(dc->attached->transform.position + RotMatrix(dc->attached->transform.rotation).Rotate(dc->attachedOffset)), 2.0f / zoom, DARKGRAY);
        }
        else if (c->GetType() == ConstraintType::PIN) {
            PinConstraint* pc = static_cast<PinConstraint*>(c.get());
            Vector2 pos = ToScreen(pc->position); 
            float baseSize = 25.0f / zoom;
            DrawTriangle( pos, { pos.x - baseSize, pos.y + baseSize }, { pos.x + baseSize, pos.y + baseSize }, BLACK);
            DrawCircleV(pos, 5.0f / zoom, BLACK);
        }
        else if (c->GetType() == ConstraintType::JOINT) {
            JointConstraint* jc = static_cast<JointConstraint*>(c.get());
            DrawCircleV(ToScreen(jc->position), 5.0f / zoom, DARKGRAY);
        }
        else if (c->GetType() == ConstraintType::MOTOR) {
            MotorConstraint* mc = static_cast<MotorConstraint*>(c.get());
            DrawCircleV(ToScreen(mc->rotor->transform.position + RotMatrix(mc->rotor->transform.rotation).Rotate(mc->localPosition)), 10.0f / zoom, RED);
        }
    }
}

void GizmoRender(World& world, const EditorCamera& camera)
{
    EditorState& state = EditorState::Get();
    const auto& theme = state.GetThemeColors();
    const std::vector<size_t>& selectedIDs = state.GetSelectedObjectIDs();
    std::string selectedGroup = state.GetSelectedGroup();
    
    Vec2 gizmoWorldPos;
    bool hasActiveGizmo = false;

    if (!selectedIDs.empty()) {
        Vec2 center(0, 0);
        int count = 0;
        for (size_t id : selectedIDs) {
            for (auto& obj : world.GetGameObjects()) if (obj->GetID() == id) {
                center += obj->transform.position;
                count++;
                break;
            }
        }
        if (count > 0) {
            gizmoWorldPos = center * (1.0f / (float)count);
            hasActiveGizmo = true;
        }
    } else if (!selectedGroup.empty()) {
        GeneratorDef* gen = world.GetGenerator(selectedGroup);
        if (gen) {
            gizmoWorldPos = Vec2(gen->startX, gen->startY);
            hasActiveGizmo = true;
        }
    }

    if (!hasActiveGizmo) {
        state.SetHoveredAxis(GizmoAxis::NONE);
        state.SetHoveredAnchor({ (size_t)-1, -1, false });
        return;
    }

    Vec2 origin = camera.WorldToScreenPixels(gizmoWorldPos);
    Vector2 originV = { origin.x, origin.y };
    GizmoType type = state.GetGizmoType();
    GizmoAxis hovered = state.GetHoveredAxis();
    GizmoAxis active = state.GetActiveAxis();
    float handleLength = 100.0f;
    float thickness = 4.0f;

    switch (type) {
        case GizmoType::TRANSLATE: {
            Color cx = (hovered == GizmoAxis::X || active == GizmoAxis::X) ? YELLOW : RED;
            Color cy = (hovered == GizmoAxis::Y || active == GizmoAxis::Y) ? YELLOW : GREEN;
            Color cb = (hovered == GizmoAxis::BOTH || active == GizmoAxis::BOTH) ? YELLOW : BLUE;
            DrawLineEx(originV, {originV.x + handleLength, originV.y}, thickness, cx);
            DrawTriangle({originV.x + handleLength + 15, originV.y}, {originV.x + handleLength, originV.y - 7}, {originV.x + handleLength, originV.y + 7}, cx);
            DrawLineEx(originV, {originV.x, originV.y + handleLength}, thickness, cy);
            DrawTriangle({originV.x, originV.y + handleLength + 15}, {originV.x + 7, originV.y + handleLength}, {originV.x - 7, originV.y + handleLength}, cy);
            DrawRectangleV({originV.x - 8, originV.y - 8}, {16, 16}, cb);
            break;
        }
        case GizmoType::ROTATE: {
            float r = 80.0f;
            Color cb = (hovered == GizmoAxis::BOTH || active == GizmoAxis::BOTH) ? YELLOW : BLUE;
            DrawCircleLinesV(originV, r, cb);
            DrawCircleV(originV, 5.0f, cb);
            break;
        }
        case GizmoType::SCALE: {
            Color cx = (hovered == GizmoAxis::X || active == GizmoAxis::X) ? YELLOW : RED;
            Color cy = (hovered == GizmoAxis::Y || active == GizmoAxis::Y) ? YELLOW : GREEN;
            Color cb = (hovered == GizmoAxis::BOTH || active == GizmoAxis::BOTH) ? YELLOW : BLUE;
            DrawLineEx(originV, {originV.x + handleLength, originV.y}, thickness, cx);
            DrawRectangleV({originV.x + handleLength - 5, originV.y - 5}, {10, 10}, cx);
            DrawLineEx(originV, {originV.x, originV.y + handleLength}, thickness, cy);
            DrawRectangleV({originV.x - 5, originV.y + handleLength - 5}, {10, 10}, cy);
            DrawRectangleV({originV.x - 8, originV.y - 8}, {16, 16}, cb);
            break;
        }
        default: break;
    }

    EditorState::SelectedAnchor hoveredAnchor = { (size_t)-1, -1, false };
    if (!selectedIDs.empty() && !world.GetGameObjects().empty()) {
        GameObject* selected = world.GetGameObjects()[0].get(); // fallback
        for (auto& obj : world.GetGameObjects()) if (obj->GetID() == selectedIDs[0]) { selected = obj.get(); break; }
        
        auto constraints = world.GetConstraintsForObject(selected);
        Vector2 mousePos = GetMousePosition();
        float dotRadius = 6.0f;

        for (auto* c : constraints) {
            std::vector<Vec2> worldAnchors;
            if (c->GetType() == ConstraintType::DISTANCE) {
                DistanceConstraint* dc = static_cast<DistanceConstraint*>(c);
                worldAnchors.push_back(dc->anchor->transform.position + RotMatrix(dc->anchor->transform.rotation).Rotate(dc->anchorOffset));
                worldAnchors.push_back(dc->attached->transform.position + RotMatrix(dc->attached->transform.rotation).Rotate(dc->attachedOffset));
            } else if (c->GetType() == ConstraintType::PIN) {
                PinConstraint* pc = static_cast<PinConstraint*>(c);
                worldAnchors.push_back(pc->position);
                for (auto& att : pc->attachments) worldAnchors.push_back(att.obj->transform.position + RotMatrix(att.obj->transform.rotation).Rotate(Vec2(att.localX, att.localY)));
            } else if (c->GetType() == ConstraintType::JOINT) {
                JointConstraint* jc = static_cast<JointConstraint*>(c);
                worldAnchors.push_back(jc->position);
                for (auto& att : jc->attachments) worldAnchors.push_back(att.obj->transform.position + RotMatrix(att.obj->transform.rotation).Rotate(Vec2(att.localX, att.localY)));
            } else if (c->GetType() == ConstraintType::MOTOR) {
                MotorConstraint* mc = static_cast<MotorConstraint*>(c);
                worldAnchors.push_back(mc->rotor->transform.position + RotMatrix(mc->rotor->transform.rotation).Rotate(mc->localPosition));
            }

            for (int i = (int)worldAnchors.size() - 1; i >= 0; i--) {
                Vec2 s = camera.WorldToScreenPixels(worldAnchors[i]);
                Vector2 sp = { s.x, s.y };
                float currentRadius = (i == 0 && (c->GetType() == ConstraintType::PIN || c->GetType() == ConstraintType::JOINT)) ? dotRadius * 1.5f : dotRadius * 0.8f;
                bool isHovered = CheckCollisionPointCircle(mousePos, sp, currentRadius + 2.0f);
                if (isHovered && (i == 0 || !hoveredAnchor.isHovered)) hoveredAnchor = { c->GetID(), (int)i, true };
                bool isActive = (state.GetActiveAnchor().constraintID == c->GetID() && state.GetActiveAnchor().index == (int)i);
                Color color = (isHovered || isActive) ? YELLOW : (i == 0 ? ORANGE : LIME);
                DrawCircleV(sp, currentRadius, color);
                DrawCircleLinesV(sp, currentRadius, BLACK);
                if (isActive) DrawCircleLinesV(sp, currentRadius + 3.0f, YELLOW);
            }
        }
    }
    state.SetHoveredAnchor(hoveredAnchor);
}
