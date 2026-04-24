#include "main/editor/Editor.h"
#include "main/editor/PerformancePanel.h"
#include "main/editor/ScenePanel.h"
#include "main/editor/ViewportPanel.h"
#include "main/editor/HierarchyPanel.h"
#include "main/editor/InspectorPanel.h"
#include "main/editor/DebugPanel.h"
#include "external/imgui/imgui.h"
#include "external/imgui/imgui_internal.h"
#include "external/imgui/rlImGui.h"
#include "main/physics/Config.h"

namespace Editor {

Editor::Editor(World& world, EditorCamera& camera, InputHandler& input) {
    panels.push_back(std::make_unique<ViewportPanel>(camera));
    panels.push_back(std::make_unique<PerformancePanel>(camera, input));
    panels.push_back(std::make_unique<HierarchyPanel>());
    panels.push_back(std::make_unique<InspectorPanel>());
    panels.push_back(std::make_unique<DebugPanel>());
    panels.push_back(std::make_unique<ScenePanel>(Config::screenWidth, Config::screenHeight));
}

Editor::~Editor() {
}

void Editor::Update(World& world) {
    rlImGuiBegin();

    static bool opt_fullscreen = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport) {
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
        }
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("MainDockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    
    static bool resetLayout = false;
    
    // Only build layout if it doesn't exist (no .ini file) or user requested a reset
    // Ensure viewport size is valid before building
    if ((!ImGui::DockBuilderGetNode(dockspace_id) || resetLayout) && ImGui::GetMainViewport()->WorkSize.x > 0) {
        resetLayout = false;

        ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
        ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

        // Split Left (0.2)
        ImGuiID dock_id_main = dockspace_id;
        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.20f, nullptr, &dock_id_main);
        // Split Right (0.25)
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.25f, nullptr, &dock_id_main);
        // Split Bottom Center
        ImGuiID dock_id_bottom_center = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Down, 0.25f, nullptr, &dock_id_main);
        
        // Split Bottom Left
        ImGuiID dock_id_bottom_left = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.30f, nullptr, &dock_id_left);
        // Split Bottom Right
        ImGuiID dock_id_bottom_right = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.50f, nullptr, &dock_id_right);

        // Assign panels to slots
        ImGui::DockBuilderDockWindow("Viewport", dock_id_main);
        ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
        ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
        ImGui::DockBuilderDockWindow("Performance & Viewport", dock_id_bottom_left);
        ImGui::DockBuilderDockWindow("Debug Settings", dock_id_bottom_right);
        ImGui::DockBuilderDockWindow("Scene Manager", dock_id_bottom_center);
        
        ImGui::DockBuilderFinish(dockspace_id);
    }

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit")) { /* Handle exit */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window"))
        {
            if (ImGui::MenuItem("Reset Layout"))
            {
                resetLayout = true;
            }
            ImGui::Separator();
            for (auto& panel : panels)
            {
                ImGui::MenuItem(panel->GetName(), nullptr, &panel->isOpen);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    for (auto& panel : panels) {
        panel->OnImGui(world);
    }

    ImGui::End(); // End DockSpace window

    rlImGuiEnd();
}

} // namespace Editor
