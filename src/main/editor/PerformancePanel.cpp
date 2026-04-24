#include "main/editor/PerformancePanel.h"
#include "external/imgui/imgui.h"

namespace Editor {

    PerformancePanel::PerformancePanel(EditorCamera& camera, InputHandler& input) 
        : camera(camera), input(input) {}

    void PerformancePanel::OnImGui(World& world) {
        if (!isOpen) return;

        ImGui::Begin(GetName(), &isOpen);
        
        if (Config::drawFPS) {
            ImGui::Text("FPS: %d", GetFPS());
        }

        ImGui::Text("Viewport Info:");
        ImGui::Text("Zoom: %.2fx", camera.GetRaylibCamera().zoom);
        Vec2 mousePos = input.GetMouseWorldPos();
        ImGui::Text("Mouse World (m): %.2f, %.2f", mousePos.x, mousePos.y);

        ImGui::Separator();
        
        ImGui::Text("Timing:");
        ImGui::Text("Integrate Velocity Time: %.2f ms", world.integrateVelocityTime);
        ImGui::Text("Integrate Position Time: %.2f ms", world.integratePositionTime);
        ImGui::Text("Broadphase Time: %.2f ms", world.broadphaseTime);
        ImGui::Text("Solver Time: %.2f ms", world.solverTime);

        ImGui::End();
    }
}
