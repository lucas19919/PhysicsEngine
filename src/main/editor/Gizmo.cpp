#include "main/editor/Gizmo.h"

#include "external/imgui/imgui.h"
#include "raylib.h"
#include "raymath.h"

#include "main/editor/HistoryManager.h"
#include "main/physics/Config.h"
#include "math/RotationMatrix.h"

namespace Editor {

Gizmo::Gizmo() : isDragging(false) {}

bool Gizmo::Update(World& world, const EditorCamera& camera) {
    EditorState& state = EditorState::Get();
    const std::vector<size_t>& selectedIDs = state.GetSelectedObjectIDs();
    std::string selectedGroup = state.GetSelectedGroup();
    
    Vec2 gizmoWorldPos;
    GeneratorDef* genDef = nullptr;
    bool hasActiveGizmo = false;

    // Calculate gizmo position (center of selection)
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
        genDef = world.GetGenerator(selectedGroup);
        if (genDef) {
            gizmoWorldPos = Vec2(genDef->startX, genDef->startY);
            hasActiveGizmo = true;
        } else {
            Vec2 center(0, 0);
            int count = 0;
            for (const auto& obj : world.GetGameObjects()) {
                if (obj->GetGroupName() == selectedGroup) {
                    center += obj->transform.position;
                    count++;
                }
            }
            if (count > 0) {
                gizmoWorldPos = center * (1.0f / (float)count);
                hasActiveGizmo = true;
            }
        }
    }

    if (!hasActiveGizmo) {
        state.SetHoveredAxis(GizmoAxis::NONE);
        return false;
    }

    GizmoType type = state.GetGizmoType();
    GizmoAxis hovered = HitTest(camera, gizmoWorldPos, type);
    state.SetHoveredAxis(hovered);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && state.IsViewportHovered() && hovered != GizmoAxis::NONE) {
        state.SetActiveAxis(hovered);
        isDragging = true;
        initialMouseWorldPos = camera.ScreenToWorldMeters(state.GetViewportMousePos());
    }

    if (isDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        Vector2 d = GetMouseDelta();
        Vec2 delta(d.x, d.y);
        
        if (delta.x != 0 || delta.y != 0) {
            float zoom = camera.GetRaylibCamera().zoom;
            Vec2 worldDelta((delta.x / zoom) * Config::PixelToMeter, (delta.y / zoom) * Config::PixelToMeter);
            Vec2 currentMouseWorld = camera.ScreenToWorldMeters(state.GetViewportMousePos());

            if (type == GizmoType::TRANSLATE) {
                Vec2 move(0, 0);
                if (state.GetActiveAxis() == GizmoAxis::X) move.x = worldDelta.x;
                else if (state.GetActiveAxis() == GizmoAxis::Y) move.y = worldDelta.y;
                else if (state.GetActiveAxis() == GizmoAxis::BOTH) move = worldDelta;

                if (!selectedIDs.empty()) {
                    for (size_t id : selectedIDs) {
                        for (auto& obj : world.GetGameObjects()) if (obj->GetID() == id) {
                            obj->transform.SetPosition(obj->transform.position + move);
                            break;
                        }
                    }
                } else if (!selectedGroup.empty()) {
                    if (genDef) {
                        genDef->startX += move.x;
                        genDef->startY += move.y;
                        world.RegenerateGenerator(selectedGroup);
                    } else {
                        for (auto& obj : world.GetGameObjects()) {
                            if (obj->GetGroupName() == selectedGroup) {
                                obj->transform.SetPosition(obj->transform.position + move);
                            }
                        }
                    }
                }
            } else if (type == GizmoType::ROTATE) {
                Vec2 originScreen = camera.WorldToScreenPixels(gizmoWorldPos);
                Vector2 oPos = { originScreen.x, originScreen.y };
                Vec2 mouseScreen = state.GetViewportMousePos();
                Vector2 mPos = { mouseScreen.x, mouseScreen.y };
                Vector2 prevPos = { mouseScreen.x - delta.x, mouseScreen.y - delta.y };
                
                Vector2 currentDir = Vector2Subtract(mPos, oPos);
                Vector2 prevDir = Vector2Subtract(prevPos, oPos);
                
                if (Vector2Length(currentDir) > 0.1f && Vector2Length(prevDir) > 0.1f) {
                    float angle1 = atan2f(currentDir.y, currentDir.x);
                    float angle2 = atan2f(prevDir.y, prevDir.x);
                    float diff = angle1 - angle2;
                    if (diff > PI) diff -= 2.0f * PI;
                    if (diff < -PI) diff += 2.0f * PI;

                    for (size_t id : selectedIDs) {
                        for (auto& obj : world.GetGameObjects()) if (obj->GetID() == id) {
                            if (selectedIDs.size() > 1) {
                                Vec2 relative = obj->transform.position - gizmoWorldPos;
                                float c = cos(diff);
                                float s = sin(diff);
                                Vec2 rotated(relative.x * c - relative.y * s, relative.x * s + relative.y * c);
                                obj->transform.SetPosition(gizmoWorldPos + rotated);
                            }
                            obj->transform.SetRotation(obj->transform.rotation + diff);
                            break;
                        }
                    }
                }
            } else if (type == GizmoType::SCALE) {
                float sx = 1.0f;
                float sy = 1.0f;
                float sensitivity = 0.5f;
                if (state.GetActiveAxis() == GizmoAxis::X) sx += worldDelta.x * sensitivity;
                else if (state.GetActiveAxis() == GizmoAxis::Y) sy += worldDelta.y * sensitivity;
                else if (state.GetActiveAxis() == GizmoAxis::BOTH) {
                    float s = 1.0f + (worldDelta.x + worldDelta.y) * 0.5f * sensitivity;
                    sx = sy = s;
                }

                if (sx > 0.001f && sy > 0.001f) {
                    if (!selectedIDs.empty()) {
                        for (size_t id : selectedIDs) {
                            for (auto& obj : world.GetGameObjects()) if (obj->GetID() == id) {
                                obj->Scale(sx, sy);
                                break;
                            }
                        }
                    } else if (!selectedGroup.empty()) {
                        if (genDef) {
                            if (state.GetActiveAxis() == GizmoAxis::X || state.GetActiveAxis() == GizmoAxis::BOTH) genDef->spacingX *= sx;
                            if (state.GetActiveAxis() == GizmoAxis::Y || state.GetActiveAxis() == GizmoAxis::BOTH) genDef->spacingY *= sy;
                            if (genDef->templateObject) genDef->templateObject->Scale(sx, sy);
                            world.RegenerateGenerator(selectedGroup);
                        } else {
                            for (auto& obj : world.GetGameObjects()) {
                                if (obj->GetGroupName() == selectedGroup) {
                                    obj->Scale(sx, sy);
                                }
                            }
                        }
                    }
                }
            }
        }
        return true;
    }

    if (isDragging && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        isDragging = false;
        state.SetActiveAxis(GizmoAxis::NONE);
        HistoryManager::Get().RecordState(world);
        return true;
    }

    return isDragging;
}

void Gizmo::Render(World& world, const EditorCamera& camera) {
    EditorState& state = EditorState::Get();
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
        } else {
            Vec2 center(0, 0);
            int count = 0;
            for (const auto& obj : world.GetGameObjects()) {
                if (obj->GetGroupName() == selectedGroup) {
                    center += obj->transform.position;
                    count++;
                }
            }
            if (count > 0) {
                gizmoWorldPos = center * (1.0f / (float)count);
                hasActiveGizmo = true;
            }
        }
    }

    if (!hasActiveGizmo) return;

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
}

GizmoAxis Gizmo::HitTest(const EditorCamera& camera, Vec2 gizmoWorldPos, GizmoType type) {
    EditorState& state = EditorState::Get();
    Vec2 origin = camera.WorldToScreenPixels(gizmoWorldPos);
    Vec2 mousePos = state.GetViewportMousePos();
    Vector2 mPos = { mousePos.x, mousePos.y };
    Vector2 oPos = { origin.x, origin.y };
    float handleLength = 100.0f;

    if (type == GizmoType::TRANSLATE || type == GizmoType::SCALE) {
        if (CheckCollisionPointRec(mPos, { oPos.x - 15, oPos.y - 15, 30, 30 })) return GizmoAxis::BOTH;
        else if (CheckCollisionPointRec(mPos, { oPos.x, oPos.y - 15, handleLength + 40, 30 })) return GizmoAxis::X;
        else if (CheckCollisionPointRec(mPos, { oPos.x - 15, oPos.y, 30, handleLength + 40 })) return GizmoAxis::Y;
    } else if (type == GizmoType::ROTATE) {
        float radius = 80.0f;
        float dist = Vector2Distance(mPos, oPos);
        if (std::abs(dist - radius) < 10.0f || dist < 10.0f) return GizmoAxis::BOTH;
    }
    return GizmoAxis::NONE;
}

} // namespace Editor
