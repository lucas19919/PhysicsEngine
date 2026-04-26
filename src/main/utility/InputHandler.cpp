#include "main/utility/InputHandler.h"
#include "main/components/Controller.h"
#include "main/physics/Config.h"
#include "main/editor/EditorState.h"
#include "raylib.h"
#include "raymath.h"
#include "external/imgui/imgui.h"
#include "main/scenes/LoadScene.h"
#include "main/scenes/SaveScene.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Motor.h"
#include "math/RotationMatrix.h"

void InputHandler::Update(World& world, EditorCamera& camera, const std::string& filePath, int screenWidth, int screenHeight, float dt)
{
    EditorState& state = EditorState::Get();

    // Ignore input if ImGui is capturing, UNLESS it's the viewport
    // (ViewportPanel sets IsViewportHovered when the Viewport window is hovered)
    if (ImGui::GetCurrentContext() != nullptr && (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) && !state.IsViewportHovered()) return;

    // 1. Update Viewport Position
    physicsMousePos = camera.ScreenToWorldMeters(state.GetViewportMousePos());

    // 2. Handle Anchor/Gizmo Interaction
    if (state.IsViewportHovered() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (state.GetHoveredAnchor().isHovered) {
            state.SetActiveAnchor(state.GetHoveredAnchor());
        } else if (state.GetHoveredAxis() != GizmoAxis::NONE) {
            state.SetActiveAxis(state.GetHoveredAxis());
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        state.SetActiveAxis(GizmoAxis::NONE);
        state.SetActiveAnchor({nullptr, -1, false});
    }

    if (state.GetActiveAnchor().constraint != nullptr) {
        Constraint* c = state.GetActiveAnchor().constraint;
        int idx = state.GetActiveAnchor().index;
        Vec2 mouseWorld = camera.ScreenToWorldMeters(state.GetViewportMousePos());

        // Grid snapping if CTRL is held
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
            float snap = 0.1f; // 10cm grid
            mouseWorld.x = std::round(mouseWorld.x / snap) * snap;
            mouseWorld.y = std::round(mouseWorld.y / snap) * snap;
        }

        // Freeze objects involved in the constraint while dragging its anchor
        // ... (freeze logic remains the same)
        if (c->GetType() == ConstraintType::DISTANCE) {
            DistanceConstraint* dc = static_cast<DistanceConstraint*>(c);
            if (dc->anchor->rb) { dc->anchor->rb->SetVelocity({0,0}); dc->anchor->rb->SetAngularVelocity(0); }
            if (dc->attached->rb) { dc->attached->rb->SetVelocity({0,0}); dc->attached->rb->SetAngularVelocity(0); }
        } else if (c->GetType() == ConstraintType::PIN) {
            PinConstraint* pc = static_cast<PinConstraint*>(c);
            for (auto& att : pc->attachments) if (att.obj->rb) { att.obj->rb->SetVelocity({0,0}); att.obj->rb->SetAngularVelocity(0); }
        } else if (c->GetType() == ConstraintType::JOINT) {
            JointConstraint* jc = static_cast<JointConstraint*>(c);
            for (auto& att : jc->attachments) if (att.obj->rb) { att.obj->rb->SetVelocity({0,0}); att.obj->rb->SetAngularVelocity(0); }
        }

        bool moveObjects = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        if (c->GetType() == ConstraintType::DISTANCE) {
            DistanceConstraint* dc = static_cast<DistanceConstraint*>(c);
            if (idx == 0) dc->anchorOffset = RotMatrix(-dc->anchor->transform.rotation).Rotate(mouseWorld - dc->anchor->transform.position);
            else if (idx == 1) dc->attachedOffset = RotMatrix(-dc->attached->transform.rotation).Rotate(mouseWorld - dc->attached->transform.position);
        } else if (c->GetType() == ConstraintType::PIN) {
            PinConstraint* pc = static_cast<PinConstraint*>(c);
            if (idx == 0) {
                if (moveObjects) {
                    Vec2 delta = mouseWorld - pc->position;
                    for (auto& att : pc->attachments) att.obj->transform.SetPosition(att.obj->transform.position + delta);
                } else {
                    // Default: update local anchors so objects stay put
                    for (auto& att : pc->attachments) {
                        Vec2 local = RotMatrix(-att.obj->transform.rotation).Rotate(mouseWorld - att.obj->transform.position);
                        att.localX = local.x; att.localY = local.y;
                    }
                }
                pc->position = mouseWorld;
            }
            else {
                PinAttachment& att = pc->attachments[idx - 1];
                Vec2 local = RotMatrix(-att.obj->transform.rotation).Rotate(mouseWorld - att.obj->transform.position);
                att.localX = local.x; att.localY = local.y;
            }
        } else if (c->GetType() == ConstraintType::JOINT) {
            JointConstraint* jc = static_cast<JointConstraint*>(c);
            if (idx == 0) {
                if (moveObjects) {
                    Vec2 delta = mouseWorld - jc->position;
                    for (auto& att : jc->attachments) att.obj->transform.SetPosition(att.obj->transform.position + delta);
                } else {
                    for (auto& att : jc->attachments) {
                        Vec2 local = RotMatrix(-att.obj->transform.rotation).Rotate(mouseWorld - att.obj->transform.position);
                        att.localX = local.x; att.localY = local.y;
                    }
                }
                jc->position = mouseWorld;
            }
            else {
                JointAttachment& att = jc->attachments[idx - 1];
                Vec2 local = RotMatrix(-att.obj->transform.rotation).Rotate(mouseWorld - att.obj->transform.position);
                att.localX = local.x; att.localY = local.y;
            }
        } else if (c->GetType() == ConstraintType::MOTOR) {
            MotorConstraint* mc = static_cast<MotorConstraint*>(c);
            mc->localPosition = RotMatrix(-mc->rotor->transform.rotation).Rotate(mouseWorld - mc->rotor->transform.position);
        }
        return; // Skip normal gizmo logic if dragging anchor
    }

    // 2. Handle Gizmo Logic (Update state and interactions)
    GameObject* selected = state.GetSelected();
    std::string selectedGroup = state.GetSelectedGroup();
    
    Vec2 gizmoWorldPos;
    GeneratorDef* genDef = nullptr;
    bool hasActiveGizmo = false;

    if (selected) {
        gizmoWorldPos = selected->transform.position;
        hasActiveGizmo = true;
    } else if (!selectedGroup.empty()) {
        genDef = world.GetGenerator(selectedGroup);
        if (genDef) {
            gizmoWorldPos = Vec2(genDef->startX, genDef->startY);
            hasActiveGizmo = true;
        } else {
            // Calculate center of group for gizmo placement
            Vec2 center(0, 0);
            int count = 0;
            for (const auto& obj : world.GetGameObjects()) {
                if (obj->GetGroupName() == selectedGroup) {
                    center += obj->transform.position;
                    count++;
                }
            }
            if (count > 0) {
                gizmoWorldPos = center * (1.0f / count);
                hasActiveGizmo = true;
            }
        }
    }

    if (hasActiveGizmo) {
        Vec2 gizmoScreenPos = camera.WorldToScreenPixels(gizmoWorldPos);
        float handleLength = 100.0f;
        GizmoType type = state.GetGizmoType();
        Vec2 mousePos = state.GetViewportMousePos();
        
        GizmoAxis hovered = GizmoAxis::NONE;
        if (type == GizmoType::TRANSLATE) {
            Vector2 mPos = { mousePos.x, mousePos.y };
            Vector2 oPos = { gizmoScreenPos.x, gizmoScreenPos.y };
            if (CheckCollisionPointRec(mPos, { oPos.x - 15, oPos.y - 15, 30, 30 })) hovered = GizmoAxis::BOTH;
            else if (CheckCollisionPointRec(mPos, { oPos.x, oPos.y - 15, handleLength + 40, 30 })) hovered = GizmoAxis::X;
            else if (CheckCollisionPointRec(mPos, { oPos.x - 15, oPos.y, 30, handleLength + 40 })) hovered = GizmoAxis::Y;
        } else if (type == GizmoType::ROTATE && selected) {
            Vector2 mPos = { mousePos.x, mousePos.y };
            Vector2 oPos = { gizmoScreenPos.x, gizmoScreenPos.y };
            float radius = 80.0f;
            float dist = Vector2Distance(mPos, oPos);
            if (std::abs(dist - radius) < 10.0f || dist < 10.0f) hovered = GizmoAxis::BOTH;
        }
        
        state.SetHoveredAxis(hovered);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && state.IsViewportHovered()) state.SetActiveAxis(hovered);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) state.SetActiveAxis(GizmoAxis::NONE);

        // Gizmo Dragging
        if (state.GetActiveAxis() != GizmoAxis::NONE && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 d = GetMouseDelta();
            Vec2 delta(d.x, d.y);
            if (delta.x != 0 || delta.y != 0) {
                float zoom = camera.GetRaylibCamera().zoom;
                Vec2 worldDelta( (delta.x / zoom) * Config::PixelToMeter, (delta.y / zoom) * Config::PixelToMeter );
                if (type == GizmoType::TRANSLATE) {
                    if (selected) {
                        if (state.GetActiveAxis() == GizmoAxis::X) selected->transform.SetPosition(selected->transform.position + Vec2(worldDelta.x, 0));
                        else if (state.GetActiveAxis() == GizmoAxis::Y) selected->transform.SetPosition(selected->transform.position + Vec2(0, worldDelta.y));
                        else if (state.GetActiveAxis() == GizmoAxis::BOTH) selected->transform.SetPosition(selected->transform.position + worldDelta);
                    } else if (!selectedGroup.empty()) {
                        if (genDef) {
                            if (state.GetActiveAxis() == GizmoAxis::X) genDef->startX += worldDelta.x;
                            else if (state.GetActiveAxis() == GizmoAxis::Y) genDef->startY += worldDelta.y;
                            else if (state.GetActiveAxis() == GizmoAxis::BOTH) { genDef->startX += worldDelta.x; genDef->startY += worldDelta.y; }
                            world.RegenerateGenerator(selectedGroup);
                        } else {
                            for (auto& obj : world.GetGameObjects()) {
                                if (obj->GetGroupName() == selectedGroup) {
                                    if (state.GetActiveAxis() == GizmoAxis::X) obj->transform.SetPosition(obj->transform.position + Vec2(worldDelta.x, 0));
                                    else if (state.GetActiveAxis() == GizmoAxis::Y) obj->transform.SetPosition(obj->transform.position + Vec2(0, worldDelta.y));
                                    else if (state.GetActiveAxis() == GizmoAxis::BOTH) obj->transform.SetPosition(obj->transform.position + worldDelta);
                                }
                            }
                        }
                    }
                } else if (type == GizmoType::ROTATE && selected) {
                    Vector2 mPos = { mousePos.x, mousePos.y };
                    Vector2 oPos = { gizmoScreenPos.x, gizmoScreenPos.y };
                    Vector2 currentDir = Vector2Subtract(mPos, oPos);
                    Vector2 prevPos = { mousePos.x - delta.x, mousePos.y - delta.y };
                    Vector2 prevDir = Vector2Subtract(prevPos, oPos);
                    if (Vector2Length(currentDir) > 0.1f && Vector2Length(prevDir) > 0.1f) {
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
    } else {
        state.SetHoveredAxis(GizmoAxis::NONE);
        state.SetActiveAxis(GizmoAxis::NONE);
    }

    // 3. Selection Logic moved to ViewportPanel to ensure fresh ImGui state
    // ...

    // 4. Camera Controls
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta();
        camera.Pan(Vec2(delta.x, delta.y));
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector2 m = GetMousePosition();
        camera.Zoom(wheel, Vec2(m.x, m.y));
    }

    // 5. Shortcuts and Simulation
    if (IsKeyPressed(KEY_SPACE)) {
        if (world.isPaused && !state.HasInitialState()) {
            state.CaptureInitialState(SaveScene::SerializeScene(world));
        }
        world.isPaused = !world.isPaused;
    }

    if (IsKeyPressed(KEY_R)) {
        if (state.HasInitialState()) {
            state.ClearSelection();
            world.isPaused = true;
            LoadScene::LoadFromJSON(state.GetInitialState(), world, screenWidth, screenHeight);
            state.ClearInitialState();
        }
    }

    if (IsKeyPressed(KEY_G) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) {
        state.SetGizmoType(GizmoType::TRANSLATE);
    }
    if (IsKeyPressed(KEY_E) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) {
        state.SetGizmoType(GizmoType::ROTATE);
    }
    if (IsKeyPressed(KEY_S) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) {
        state.SetGizmoType(GizmoType::SCALE);
    }
    
    if (IsKeyPressed(KEY_DELETE)) {
        GameObject* sel = state.GetSelected();
        if (sel) {
            world.RemoveGameObject(sel->GetID());
            state.ClearSelection();
        }
    }

    for (const auto& c : world.GetControllers()) {
        if (c->active) c->Update(dt); 
    }
}
