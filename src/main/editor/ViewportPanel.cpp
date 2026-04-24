#include "main/editor/ViewportPanel.h"
#include "external/imgui/imgui.h"
#include "external/imgui/rlImGui.h"
#include "main/utility/Draw.h"
#include "main/editor/EditorState.h"
#include "main/scenes/LoadScene.h"

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
    
    // Resize render texture if panel size changed
    if (viewportPanelSize.x != target.texture.width || viewportPanelSize.y != target.texture.height) {
        RefreshRenderTexture(viewportPanelSize.x, viewportPanelSize.y);
        camera.GetRaylibCamera().offset = { viewportPanelSize.x / 2.0f, viewportPanelSize.y / 2.0f };
    }

    // Update EditorState
    EditorState& state = EditorState::Get();
    state.SetViewportSize(Vec2(viewportPanelSize.x, viewportPanelSize.y));
    state.SetViewportHovered(ImGui::IsWindowHovered());
    state.SetViewportFocused(ImGui::IsWindowFocused());
    
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 min = ImGui::GetCursorScreenPos();
    state.SetViewportMousePos(Vec2(mousePos.x - min.x, mousePos.y - min.y));

    // Render physics world to texture
    BeginTextureMode(target);
        ClearBackground(GRAY);
        camera.Begin();
            Render(world, camera);
        camera.End();
        GizmoRender(camera);
        GizmoUpdate(camera);
    EndTextureMode();

    // Draw the texture into the ImGui window
    rlImGuiImageRenderTextureFit(&target, true);

    // --- Floating Playback Toolbar ---
    float titleBarHeight = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().FramePadding.y * 2.0f;
    float buttonWidth = 30.0f;
    float buttonHeight = 22.0f;
    float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
    
    // Calculate exact width for 3 buttons
    float toolbarWidth = (buttonWidth * 3.0f) + (itemSpacing * 2.0f) + 10.0f; // +10 for internal padding
    
    // Center the entire toolbar container in the viewport
    ImGui::SetCursorPos(ImVec2((viewportPanelSize.x - toolbarWidth) * 0.5f, titleBarHeight + 10));
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.6f));
    
    if (ImGui::BeginChild("PlaybackToolbar", ImVec2(toolbarWidth, 32), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        // Start buttons at a fixed offset to ensure the middle button is truly central
        ImGui::SetCursorPosX(5);
        ImGui::SetCursorPosY((32.0f - buttonHeight) * 0.5f); 
        
        // Play/Pause button
        if (world.isPaused) {
            if (ImGui::Button(">", ImVec2(buttonWidth, buttonHeight))) world.isPaused = false;
        } else {
            // Using a more symmetrical pause icon
            if (ImGui::Button("||", ImVec2(buttonWidth, buttonHeight))) world.isPaused = true;
        }
        
        ImGui::SameLine();
        
        // Step button
        if (ImGui::Button("|>", ImVec2(buttonWidth, buttonHeight))) {
            world.isPaused = false; 
            world.Step(1.0f / 60.0f);
            world.isPaused = true;
        }

        ImGui::SameLine();

        // Reset/Reload button
        if (ImGui::Button("R", ImVec2(buttonWidth, buttonHeight))) {
            if (!EditorState::Get().GetActiveScenePath().empty()) {
                world.isPaused = true;
                world.Clear();
                LoadScene::Load(EditorState::Get().GetActiveScenePath(), world, (int)viewportPanelSize.x, (int)viewportPanelSize.y);
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace Editor
