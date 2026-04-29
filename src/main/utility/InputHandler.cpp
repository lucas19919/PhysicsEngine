#include "main/utility/InputHandler.h"

#include "external/imgui/imgui.h"
#include "raylib.h"

#include "main/components/Controller.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Motor.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/editor/EditorState.h"
#include "main/editor/HistoryManager.h"
#include "main/physics/Config.h"
#include "main/scenes/LoadScene.h"
#include "main/scenes/SaveScene.h"
#include "math/RotationMatrix.h"
#include "raymath.h"
#include <external/nlohmann/json.hpp>

using json = nlohmann::json;

void InputHandler::Update(World& world, EditorCamera& camera, const std::string& filePath, int screenWidth, int screenHeight, float dt)
{
    EditorState& state = EditorState::Get();

    // Ignore input if ImGui is capturing, UNLESS it's the viewport
    if (ImGui::GetCurrentContext() != nullptr && (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) && !state.IsViewportHovered()) return;

    // 1. Update Mouse Pos
    physicsMousePos = camera.ScreenToWorldMeters(state.GetViewportMousePos());

    // 2. Gizmo Interaction
    bool gizmoActive = gizmo.Update(world, camera);

    // 3. Anchor Interaction
    if (!gizmoActive) {
        HandleAnchorInteraction(world, camera);
    }

    // 4. Selection Logic (Only if gizmo or anchors aren't being dragged)
    if (!gizmoActive && state.GetActiveAnchor().constraintID == (size_t)-1) {
        HandleSelection(world, camera);
    }

    // 5. Camera Controls
    HandleCameraControls(camera);

    // 6. Shortcuts
    HandleShortcuts(world, screenWidth, screenHeight);

    // 7. Controllers
    for (const auto& c : world.GetControllers()) if (c->active) c->Update(dt); 
}

void InputHandler::HandleCameraControls(EditorCamera& camera) {
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta();
        camera.Pan(Vec2(delta.x, delta.y));
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector2 m = GetMousePosition();
        camera.Zoom(wheel, Vec2(m.x, m.y));
    }
}

void InputHandler::HandleShortcuts(World& world, int screenWidth, int screenHeight) {
    EditorState& state = EditorState::Get();

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

    if (IsKeyPressed(KEY_G) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) state.SetGizmoType(GizmoType::TRANSLATE);
    if (IsKeyPressed(KEY_E) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) state.SetGizmoType(GizmoType::ROTATE);
    if (IsKeyPressed(KEY_S) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) state.SetGizmoType(GizmoType::SCALE);
    
    if (IsKeyPressed(KEY_DELETE)) {
        size_t selID = state.GetSelectedID();
        if (selID != (size_t)-1) {
            Editor::HistoryManager::Get().RecordState(world);
            world.RemoveGameObject(selID);
            state.ClearSelection();
            Editor::HistoryManager::Get().RecordState(world);
        }
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        if (IsKeyPressed(KEY_Z)) Editor::HistoryManager::Get().Undo(world);
        if (IsKeyPressed(KEY_Y)) Editor::HistoryManager::Get().Redo(world);
        
        if (IsKeyPressed(KEY_D)) {
            GameObject* sel = state.GetSelected(world);
            std::string selGroup = state.GetSelectedGroup();
            
            if (sel) {
                Editor::HistoryManager::Get().RecordState(world);
                nlohmann::json objJson = SaveScene::SerializeObject(sel);
                nlohmann::json wrapper;
                wrapper["objects"] = { objJson };
                LoadScene::LoadCollection(wrapper, world, Vec2(0.5f, 0.5f), "");
                Editor::HistoryManager::Get().RecordState(world);
            } else if (!selGroup.empty()) {
                Editor::HistoryManager::Get().RecordState(world);
                nlohmann::json groupData;
                json objects = json::array();
                for (const auto& obj : world.GetGameObjects()) {
                    if (obj->GetGroupName() == selGroup) {
                        objects.push_back(SaveScene::SerializeObject(obj.get()));
                    }
                }
                groupData["objects"] = objects;
                LoadScene::LoadCollection(groupData, world, Vec2(1.0f, 1.0f), selGroup + " Copy");
                Editor::HistoryManager::Get().RecordState(world);
            }
        }
    }
}

void InputHandler::HandleAnchorInteraction(World& world, const EditorCamera& camera) {
    EditorState& state = EditorState::Get();

    if (state.IsViewportHovered() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (state.GetHoveredAnchor().isHovered) {
            state.SetActiveAnchor(state.GetHoveredAnchor());
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (state.GetActiveAnchor().constraintID != (size_t)-1) {
             Editor::HistoryManager::Get().RecordState(world);
        }
        state.SetActiveAnchor({(size_t)-1, -1, false});
    }

    if (state.GetActiveAnchor().constraintID != (size_t)-1) {
        size_t cID = state.GetActiveAnchor().constraintID;
        Constraint* c = nullptr;
        for (auto& constr : world.GetConstraints()) if (constr->GetID() == cID) { c = constr.get(); break; }
        
        if (!c) {
            state.SetActiveAnchor({(size_t)-1, -1, false});
            return;
        }

        int idx = state.GetActiveAnchor().index;
        Vec2 mouseWorld = camera.ScreenToWorldMeters(state.GetViewportMousePos());

        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
            float snap = 0.1f;
            mouseWorld.x = std::round(mouseWorld.x / snap) * snap;
            mouseWorld.y = std::round(mouseWorld.y / snap) * snap;
        }

        if (c->GetType() == ConstraintType::DISTANCE) {
            DistanceConstraint* dc = static_cast<DistanceConstraint*>(c);
            if (dc->anchor->rb) { dc->anchor->rb->SetVelocity({0,0}); dc->anchor->rb->SetAngularVelocity(0); }
            if (dc->attached->rb) { dc->attached->rb->SetVelocity({0,0}); dc->attached->rb->SetAngularVelocity(0); }
            if (idx == 0) dc->anchorOffset = RotMatrix(-dc->anchor->transform.rotation).Rotate(mouseWorld - dc->anchor->transform.position);
            else if (idx == 1) dc->attachedOffset = RotMatrix(-dc->attached->transform.rotation).Rotate(mouseWorld - dc->attached->transform.position);
        } else if (c->GetType() == ConstraintType::PIN) {
            PinConstraint* pc = static_cast<PinConstraint*>(c);
            for (auto& att : pc->attachments) if (att.obj->rb) { att.obj->rb->SetVelocity({0,0}); att.obj->rb->SetAngularVelocity(0); }
            bool moveObjects = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
            if (idx == 0) {
                if (moveObjects) {
                    Vec2 delta = mouseWorld - pc->position;
                    for (auto& att : pc->attachments) att.obj->transform.SetPosition(att.obj->transform.position + delta);
                } else {
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
            for (auto& att : jc->attachments) if (att.obj->rb) { att.obj->rb->SetVelocity({0,0}); att.obj->rb->SetAngularVelocity(0); }
            bool moveObjects = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
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
    }
}

void InputHandler::HandleSelection(World& world, const EditorCamera& camera) {
    EditorState& state = EditorState::Get();
    Vec2 mouseScreen = state.GetViewportMousePos();
    
    // Toolbar overlay exclusion (Magic numbers from ViewportPanel)
    bool overToolbar = (mouseScreen.x >= 10 && mouseScreen.x <= 154 &&
                        mouseScreen.y >= 10 && mouseScreen.y <= 48);

    if (state.IsViewportHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !overToolbar) {
        size_t hitID = (size_t)-1;
        std::string hitGroupName = "";
        Vec2 worldMouse = camera.ScreenToWorldMeters(mouseScreen);

        // 1. Check for Constraint selection
        size_t hitConstraintID = (size_t)-1;
        float bestDist = 0.5f; // Selection radius in meters
        for (const auto& c : world.GetConstraints()) {
            Vec2 cPos;
            if (c->GetType() == ConstraintType::DISTANCE) {
                DistanceConstraint* dc = static_cast<DistanceConstraint*>(c.get());
                cPos = (dc->anchor->transform.position + dc->attached->transform.position) * 0.5f;
            } else if (c->GetType() == ConstraintType::PIN) {
                cPos = static_cast<PinConstraint*>(c.get())->position;
            } else if (c->GetType() == ConstraintType::JOINT) {
                cPos = static_cast<JointConstraint*>(c.get())->position;
            } else if (c->GetType() == ConstraintType::MOTOR) {
                MotorConstraint* mc = static_cast<MotorConstraint*>(c.get());
                cPos = mc->rotor->transform.position + RotMatrix(mc->rotor->transform.rotation).Rotate(mc->localPosition);
            }

            float d = (worldMouse - cPos).Mag();
            if (d < bestDist) {
                bestDist = d;
                hitConstraintID = c->GetID();
            }
        }

        if (hitConstraintID != (size_t)-1) {
            state.ClearSelection();
            state.SetSelectedConstraint(hitConstraintID);
            return;
        }

        // 2. Check for GameObject selection
        const auto& objects = world.GetGameObjects();
        
        for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
            GameObject* obj = it->get();
            if (obj->GetGroupName() == "INTERNAL_WALL") continue;
            if (obj->c && obj->c->TestPoint(worldMouse)) {
                hitID = obj->GetID();
                hitGroupName = obj->GetGroupName();
                break;
            }
        }

        if (state.GetPickingMode() == EditorState::PickingMode::CONSTRAINT_TARGET) {
            if (hitID != (size_t)-1) {
                size_t targetID = state.GetTargetConstraintID();
                ConstraintType type = state.GetPendingConstraintType();
                
                if (type == ConstraintType::DISTANCE) {
                    GameObject* anchor = state.GetSelected(world);
                    GameObject* attached = nullptr;
                    for (auto& obj : world.GetGameObjects()) if (obj->GetID() == hitID) { attached = obj.get(); break; }
                    
                    if (anchor && attached && anchor != attached) {
                        Editor::HistoryManager::Get().RecordState(world);
                        auto dc = std::make_unique<DistanceConstraint>(anchor, attached, (anchor->transform.position - attached->transform.position).Mag(), Vec2(0,0), Vec2(0,0));
                        world.AddConstraint(std::move(dc));
                        Editor::HistoryManager::Get().RecordState(world);
                    }
                } else if (type == ConstraintType::PIN || type == ConstraintType::JOINT) {
                    Constraint* c = nullptr;
                    for (auto& constr : world.GetConstraints()) if (constr->GetID() == targetID) { c = constr.get(); break; }
                    
                    if (c) {
                        GameObject* targetObj = nullptr;
                        for (auto& obj : world.GetGameObjects()) if (obj->GetID() == hitID) { targetObj = obj.get(); break; }
                        
                        if (targetObj) {
                            Editor::HistoryManager::Get().RecordState(world);
                            if (type == ConstraintType::PIN) {
                                PinConstraint* pc = static_cast<PinConstraint*>(c);
                                Vec2 local = RotMatrix(-targetObj->transform.rotation).Rotate(pc->position - targetObj->transform.position);
                                pc->attachments.push_back({ targetObj, local.x, local.y });
                            } else {
                                JointConstraint* jc = static_cast<JointConstraint*>(c);
                                Vec2 local = RotMatrix(-targetObj->transform.rotation).Rotate(jc->position - targetObj->transform.position);
                                jc->attachments.push_back({ targetObj, local.x, local.y });
                            }
                            Editor::HistoryManager::Get().RecordState(world);
                        }
                    }
                }
                state.ClearPickingMode();
            } else {
                state.ClearPickingMode();
            }
            return;
        } else {
            if (hitID != (size_t)-1) {
                GeneratorDef* gen = world.GetGenerator(hitGroupName);
                if (gen) state.SetSelectedGroup(gen->name);
                else {
                    if (ImGui::GetIO().KeyShift || ImGui::GetIO().KeyCtrl) {
                        if (state.IsSelected(hitID)) state.RemoveSelectedObject(hitID);
                        else state.AddSelectedObject(hitID);
                    } else {
                        state.SetSelectedObject(hitID);
                    }
                }
            } else {
                if (!ImGui::GetIO().KeyShift && !ImGui::GetIO().KeyCtrl) {
                    state.ClearSelection();
                }
                state.SetBoxSelection({ worldMouse, worldMouse, true });
            }
        }
    }

    if (state.GetBoxSelection().active) {
        Vec2 worldMouse = camera.ScreenToWorldMeters(mouseScreen);
        EditorState::BoxSelection box = state.GetBoxSelection();
        box.end = worldMouse;
        state.SetBoxSelection(box);

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            if (!ImGui::GetIO().KeyShift && !ImGui::GetIO().KeyCtrl) state.ClearSelection();
            
            float minX = std::min(box.start.x, box.end.x);
            float maxX = std::max(box.start.x, box.end.x);
            float minY = std::min(box.start.y, box.end.y);
            float maxY = std::max(box.start.y, box.end.y);

            if (std::abs(maxX - minX) > 0.01f || std::abs(maxY - minY) > 0.01f) {
                for (const auto& objPtr : world.GetGameObjects()) {
                    GameObject* obj = objPtr.get();
                    if (obj->GetGroupName() == "INTERNAL_WALL") continue;
                    Vec2 p = obj->transform.position;
                    if (p.x >= minX && p.x <= maxX && p.y >= minY && p.y <= maxY) {
                        state.AddSelectedObject(obj->GetID());
                    }
                }
            }
            state.SetBoxSelection({ {0,0}, {0,0}, false });
        }
    }
}
