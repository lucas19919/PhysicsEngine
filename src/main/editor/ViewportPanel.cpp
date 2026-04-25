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

    // scene
    BeginTextureMode(target);
        ClearBackground(GRAY);
        camera.Begin();
            Render(world, camera);
        camera.End();
        GizmoRender(world, camera);
        GizmoUpdate(world, camera);
        if (Config::drawFPS) DrawFPS(10, 10);
    EndTextureMode();

    // blit
    rlImGuiImageRenderTextureFit(&target, true);

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace Editor
