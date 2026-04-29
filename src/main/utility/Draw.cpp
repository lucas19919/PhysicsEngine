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
        bool isSelected = EditorState::Get().GetSelectedConstraintID() == c->GetID();
        Color constrColor = isSelected ? SKYBLUE : DARKGRAY;

        if (c->GetType() == ConstraintType::DISTANCE) {
            DistanceConstraint* dc = static_cast<DistanceConstraint*>(c.get());
            DrawLineEx(ToScreen(dc->anchor->transform.position + RotMatrix(dc->anchor->transform.rotation).Rotate(dc->anchorOffset)), ToScreen(dc->attached->transform.position + RotMatrix(dc->attached->transform.rotation).Rotate(dc->attachedOffset)), 2.0f / zoom, constrColor);
        }
        else if (c->GetType() == ConstraintType::PIN) {
            PinConstraint* pc = static_cast<PinConstraint*>(c.get());
            Vector2 pos = ToScreen(pc->position); 
            float baseSize = 25.0f / zoom;
            
            // Highlight selected pin
            Color pinColor = isSelected ? SKYBLUE : BLACK;

            // Draw support triangle
            DrawTriangle( pos, { pos.x - baseSize, pos.y + baseSize }, { pos.x + baseSize, pos.y + baseSize }, pinColor);
            DrawCircleV(pos, 5.0f / zoom, pinColor);

            if (pc->fixedX && pc->fixedY) {
                // Festlager notation: Hash marks at the bottom
                DrawLineEx({pos.x - baseSize, pos.y + baseSize}, {pos.x + baseSize, pos.y + baseSize}, 2.0f / zoom, pinColor);
                for (float offset = -baseSize; offset <= baseSize; offset += 8.0f / zoom) {
                    DrawLineEx({pos.x + offset, pos.y + baseSize}, {pos.x + offset - 5.0f / zoom, pos.y + baseSize + 5.0f / zoom}, 1.0f / zoom, pinColor);
                }
            } else {
                // Loslager notation: Gap and line
                float gap = 5.0f / zoom;
                DrawLineEx({pos.x - baseSize, pos.y + baseSize + gap}, {pos.x + baseSize, pos.y + baseSize + gap}, 2.0f / zoom, pinColor);
            }
        }
        else if (c->GetType() == ConstraintType::JOINT) {
            JointConstraint* jc = static_cast<JointConstraint*>(c.get());
            DrawCircleV(ToScreen(jc->position), 5.0f / zoom, constrColor);
        }
        else if (c->GetType() == ConstraintType::MOTOR) {
            MotorConstraint* mc = static_cast<MotorConstraint*>(c.get());
            DrawCircleV(ToScreen(mc->rotor->transform.position + RotMatrix(mc->rotor->transform.rotation).Rotate(mc->localPosition)), 10.0f / zoom, isSelected ? SKYBLUE : RED);
        }
    }
}
