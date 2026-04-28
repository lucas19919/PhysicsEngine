#include "main/editor/ViewportPanel.h"

#include "external/imgui/extras/IconsFontAwesome6.h"
#include "external/imgui/imgui.h"
#include "external/imgui/rlImGui.h"

#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/Renderer.h"
#include "main/components/RigidBody.h"
#include "main/editor/EditorState.h"
#include "main/physics/Config.h"
#include "main/scenes/LoadScene.h"
#include "main/utility/Draw.h"
#include "main/utility/Instantiate.h"
#include "math/RotationMatrix.h"

namespace Editor {

ViewportPanel::ViewportPanel(EditorCamera& camera) : camera(camera) {
    target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
}

ViewportPanel::~ViewportPanel() {
    UnloadRenderTexture(target);
}

void ViewportPanel::RefreshRenderTexture(float width, float height) {
    if (width <= 0 || height <= 0) return;
    UnloadRenderTexture(target);
    target = LoadRenderTexture((int)width, (int)height);
}

void ViewportPanel::OnImGui(World& world) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin(GetName(), &isOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    
    // resize
    if (viewportPanelSize.x != (float)target.texture.width || viewportPanelSize.y != (float)target.texture.height) {
        RefreshRenderTexture(viewportPanelSize.x, viewportPanelSize.y);
        camera.GetRaylibCamera().offset = { viewportPanelSize.x / 2.0f, viewportPanelSize.y / 2.0f };
    }

    // state
    EditorState& state = EditorState::Get();
    state.SetViewportSize(Vec2(viewportPanelSize.x, viewportPanelSize.y));
    state.SetViewportHovered(ImGui::IsWindowHovered());
    state.SetViewportFocused(ImGui::IsWindowFocused());
    
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 min = ImGui::GetCursorScreenPos();
    state.SetViewportMousePos(Vec2(mousePos.x - min.x, mousePos.y - min.y));

    // Exclude toolbar area from selection
    bool overToolbar = (mousePos.x >= min.x + 10 && mousePos.x <= min.x + 10 + 144 &&
                        mousePos.y >= min.y + 10 && mousePos.y <= min.y + 10 + 38);

    // Selection Logic
    if (state.IsViewportHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !state.IsGizmoHovered() && !state.IsGizmoActive() && !overToolbar) {
        size_t hitID = (size_t)-1;
        std::string hitGroupName = "";
        Vec2 physicsMousePos = camera.ScreenToWorldMeters(state.GetViewportMousePos());
        const auto& objects = world.GetGameObjects();
        
        for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
            GameObject* obj = it->get();
            if (obj->GetGroupName() == "INTERNAL_WALL") continue;
            if (obj->c && obj->c->TestPoint(physicsMousePos)) {
                hitID = obj->GetID();
                hitGroupName = obj->GetGroupName();
                break;
            }
        }

        if (state.GetPickingMode() == EditorState::PickingMode::CONSTRAINT_TARGET) {
            // Constraint picking (simplified for now)
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
                state.SetBoxSelection({ physicsMousePos, physicsMousePos, true });
            }
        }
    }

    if (state.GetBoxSelection().active) {
        Vec2 physicsMousePos = camera.ScreenToWorldMeters(state.GetViewportMousePos());
        EditorState::BoxSelection box = state.GetBoxSelection();
        box.end = physicsMousePos;
        state.SetBoxSelection(box);

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            if (!ImGui::GetIO().KeyShift && !ImGui::GetIO().KeyCtrl) state.ClearSelection();
            
            float minX = std::min(box.start.x, box.end.x);
            float maxX = std::max(box.start.x, box.end.x);
            float minY = std::min(box.start.y, box.end.y);
            float maxY = std::max(box.start.y, box.end.y);

            // Avoid selecting everything if box is tiny (just a click)
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

    // scene
    BeginTextureMode(target);
        ClearBackground(EditorState::Get().GetThemeColors().viewportBg);
        camera.Begin();
            Render(world, camera);

            if (state.GetBoxSelection().active) {
                EditorState::BoxSelection box = state.GetBoxSelection();
                float minX = std::min(box.start.x, box.end.x);
                float maxX = std::max(box.start.x, box.end.x);
                float minY = std::min(box.start.y, box.end.y);
                float maxY = std::max(box.start.y, box.end.y);
                DrawRectangleLinesEx({minX * Config::MeterToPixel, minY * Config::MeterToPixel, (maxX - minX) * Config::MeterToPixel, (maxY - minY) * Config::MeterToPixel}, 1.0f / camera.GetRaylibCamera().zoom, Fade(SKYBLUE, 0.5f));
                DrawRectangleRec({minX * Config::MeterToPixel, minY * Config::MeterToPixel, (maxX - minX) * Config::MeterToPixel, (maxY - minY) * Config::MeterToPixel}, Fade(SKYBLUE, 0.1f));
            }
        camera.End();
        GizmoRender(world, camera);
        if (Config::drawFPS) DrawFPS(140, 20);
    EndTextureMode();

    ImGui::SetCursorScreenPos(min);
    rlImGuiImageRenderTextureFit(&target, false);

    // Toolbar overlay
    ImGui::SetCursorScreenPos(ImVec2(min.x + 10, min.y + 10));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    ImGui::BeginChild("ViewportToolbar", ImVec2(144, 38), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    
    float btnSize = 30.0f;
    ImGui::SetCursorPos(ImVec2(6, 4));

    auto gizmoButton = [&](GizmoType gType, const char* icon) {
        ImGui::PushStyleColor(ImGuiCol_Button, state.GetGizmoType() == gType ? ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] : ImGui::GetStyle().Colors[ImGuiCol_Button]);
        if (ImGui::Button(icon, ImVec2(btnSize, btnSize))) state.SetGizmoType(gType);
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 4);
    };

    gizmoButton(GizmoType::TRANSLATE, ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT);
    gizmoButton(GizmoType::ROTATE, ICON_FA_ROTATE);
    gizmoButton(GizmoType::SCALE, ICON_FA_MAXIMIZE);
    
    if (ImGui::Button(ICON_FA_PLUS, ImVec2(btnSize, btnSize))) ImGui::OpenPopup("AddObjectPopup");

    if (ImGui::BeginPopup("AddObjectPopup")) {
        if (ImGui::MenuItem("Box")) {
            Instantiate().WithTransform(Vec2(0,0), 0).WithCollider(ColliderType::BOX, Vec2(1.0f, 1.0f)).WithRenderer(Shape{RenderShape::R_BOX, WHITE, Vec2(1.0f, 1.0f)}).WithRigidBody(Properties{1.0f, 0.5f, 0.5f, 0.5f}, LinearState{Vec2(0,0), Vec2(0,0), Vec2(0,0)}, AngularState{0, 0, 0}, Settings{true}).Create(world, -1)->SetName("New Box");
        }
        if (ImGui::MenuItem("Circle")) {
            Instantiate().WithTransform(Vec2(0,0), 0).WithCollider(ColliderType::CIRCLE, 0.5f).WithRenderer(Shape{RenderShape::R_CIRCLE, WHITE, 0.5f}).WithRigidBody(Properties{1.0f, 0.5f, 0.5f, 0.5f}, LinearState{Vec2(0,0), Vec2(0,0), Vec2(0,0)}, AngularState{0, 0, 0}, Settings{true}).Create(world, -1)->SetName("New Circle");
        }
        if (ImGui::MenuItem("Empty Object")) {
            Instantiate().WithTransform(Vec2(0,0), 0).Create(world, -1)->SetName("New GameObject");
        }
        ImGui::EndPopup();
    }
    
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace Editor
