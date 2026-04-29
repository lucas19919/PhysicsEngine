#include "main/editor/ViewportPanel.h"

#include "external/imgui/extras/IconsFontAwesome6.h"
#include "external/imgui/imgui.h"
#include "external/imgui/rlImGui.h"

#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/components/constrainttypes/Motor.h"
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

ViewportPanel::ViewportPanel(EditorCamera& camera, InputHandler& input) : camera(camera), input(input) {
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
        
        // Gizmo Rendering
        input.GetGizmo().Render(world, camera);

        // Anchor Rendering (Formerly in GizmoRender)
        const std::vector<size_t>& selectedIDs = state.GetSelectedObjectIDs();
        size_t selectedConstraintID = state.GetSelectedConstraintID();
        EditorState::SelectedAnchor hoveredAnchor = { (size_t)-1, -1, false };
        
        std::vector<Constraint*> constraintsToShow;
        if (selectedConstraintID != (size_t)-1) {
            Constraint* sc = state.GetSelectedConstraint(world);
            if (sc) constraintsToShow.push_back(sc);
        } else if (!selectedIDs.empty() && !world.GetGameObjects().empty()) {
            GameObject* selected = nullptr;
            for (auto& obj : world.GetGameObjects()) if (obj->GetID() == selectedIDs[0]) { selected = obj.get(); break; }
            if (selected) constraintsToShow = world.GetConstraintsForObject(selected);
        }

        if (!constraintsToShow.empty()) {
            Vector2 mPos = GetMousePosition();
            float dotRadius = 6.0f;

            for (auto* c : constraintsToShow) {
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
                    bool isHovered = CheckCollisionPointCircle(mPos, sp, currentRadius + 2.0f);
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
