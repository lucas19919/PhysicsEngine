#include "main/editor/ViewportPanel.h"
#include "external/imgui/imgui.h"
#include "external/imgui/rlImGui.h"
#include "main/utility/Draw.h"
#include "main/editor/EditorState.h"
#include "main/scenes/LoadScene.h"
#include "external/imgui/extras/IconsFontAwesome6.h"
#include "main/physics/Config.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Joint.h"

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
    if (viewportPanelSize.x != target.texture.width || viewportPanelSize.y != target.texture.height) {
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
    bool overToolbar = (mousePos.x >= min.x + 10 && mousePos.x <= min.x + 10 + 76 &&
                        mousePos.y >= min.y + 10 && mousePos.y <= min.y + 10 + 38);

    // Selection Logic (Fresh ImGui State)
    if (state.IsViewportHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !state.IsGizmoHovered() && !state.IsGizmoActive() && !overToolbar) {
        GameObject* hit = nullptr;
        Vec2 physicsMousePos = camera.ScreenToWorldMeters(state.GetViewportMousePos());
        const auto& objects = world.GetGameObjects();
        
        for (auto it = objects.rbegin(); it != objects.rend(); ++it) {
            GameObject* obj = it->get();
            if (obj->GetGroupName() == "INTERNAL_WALL") continue;
            if (obj->c && obj->c->TestPoint(physicsMousePos)) {
                hit = obj;
                break;
            }
        }

        if (state.GetPickingMode() == EditorState::PickingMode::CONSTRAINT_TARGET) {
            if (hit && hit != state.GetSelected()) {
                GameObject* anchor = state.GetSelected();
                GameObject* attached = hit;
                
                if (state.GetPendingConstraintType() == ConstraintType::DISTANCE) {
                    float dist = (attached->transform.position - anchor->transform.position).Mag();
                    world.AddConstraint(std::make_unique<DistanceConstraint>(anchor, attached, dist, Vec2(), Vec2()));
                } else if (state.GetPendingConstraintType() == ConstraintType::JOINT) {
                    world.AddConstraint(std::make_unique<JointConstraint>(
                        std::vector<JointAttachment>{{anchor, 0.0f, 0.0f}, {attached, 0.0f, 0.0f}}, 
                        physicsMousePos, false));
                }
                state.ClearPickingMode();
            } else if (!hit) {
                state.ClearPickingMode();
            }
        } else {
            if (hit) {
                GeneratorDef* gen = world.GetGenerator(hit->GetGroupName());
                if (gen) state.SetSelectedGroup(gen->name);
                else state.SetSelected(hit);
            } else {
                state.ClearSelection();
            }
        }
    }

    // scene
    BeginTextureMode(target);
        ClearBackground(EditorState::Get().GetThemeColors().viewportBg);
        camera.Begin();
            Render(world, camera);
        camera.End();
        GizmoRender(world, camera);
        
        if (state.GetPickingMode() == EditorState::PickingMode::CONSTRAINT_TARGET) {
            ImGui::SetTooltip("Picking Target... (Click object in viewport)");
        }

        if (Config::drawFPS) DrawFPS(10, 10);
    EndTextureMode();

    // blit
    ImGui::SetCursorScreenPos(min);
    rlImGuiImageRenderTextureFit(&target, false);

    // Toolbar overlay
    ImGui::SetCursorScreenPos(ImVec2(min.x + 10, min.y + 10));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    ImGui::BeginChild("ViewportToolbar", ImVec2(76, 38), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    
    float btnSize = 30.0f;
    ImGui::SetCursorPos(ImVec2(6, 4)); // Fine-tuned centering

    ImGui::PushStyleColor(ImGuiCol_Button, state.GetGizmoType() == GizmoType::TRANSLATE ? ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] : ImGui::GetStyle().Colors[ImGuiCol_Button]);
    if (ImGui::Button(ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, ImVec2(btnSize, btnSize))) {
        state.SetGizmoType(GizmoType::TRANSLATE);
    }
    ImGui::PopStyleColor();
    
    ImGui::SameLine(0, 4);
    
    ImGui::PushStyleColor(ImGuiCol_Button, state.GetGizmoType() == GizmoType::ROTATE ? ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] : ImGui::GetStyle().Colors[ImGuiCol_Button]);
    if (ImGui::Button(ICON_FA_ROTATE, ImVec2(btnSize, btnSize))) {
        state.SetGizmoType(GizmoType::ROTATE);
    }
    ImGui::PopStyleColor();
    
    ImGui::EndChild();
    ImGui::PopStyleVar(2);

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace Editor
