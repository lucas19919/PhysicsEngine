#include "main/editor/DebugPanel.h"
#include "external/imgui/imgui.h"
#include "main/physics/Config.h"

namespace Editor {

void DebugPanel::OnImGui(World& world) {
    if (!isOpen) return;

    ImGui::Begin(GetName(), &isOpen);

    if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Paused", &world.isPaused);
        if (ImGui::Button("Step Frame")) {
            world.Step(1.0f / 60.0f);
        }
    }

    if (ImGui::CollapsingHeader("Physics Config", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat2("Gravity", &Config::gravity.x, 0.1f);
        ImGui::SliderInt("Sub-Ticks", &Config::pipelineSubTicks, 1, 32);
        ImGui::SliderInt("Impulse Iterations", &Config::impulseIterations, 1, 50);
        ImGui::SliderInt("Position Iterations", &Config::positionIterations, 1, 20);
        ImGui::Checkbox("Warm Starting", &Config::warmStart);
    }

    if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Draw FPS", &Config::drawFPS);
        ImGui::Checkbox("Draw AABBs", &Config::drawAABB);
        ImGui::Checkbox("Draw Normals", &Config::drawNormals);
        ImGui::Checkbox("Draw Contact Points", &Config::drawContactPoints);
        ImGui::Checkbox("Draw Velocity", &Config::drawVelocity);
        ImGui::Checkbox("Draw Acceleration", &Config::drawAcceleration);
    }

    ImGui::End();
}

} // namespace Editor
