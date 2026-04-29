#include "main/editor/Editor.h"

#include "external/imgui/imgui.h"
#include "external/imgui/imgui_internal.h"
#include "external/imgui/rlImGui.h"

#include "main/editor/ConstraintPanel.h"
#include "main/editor/DebugPanel.h"
#include "main/editor/EditorState.h"
#include "main/editor/FileDialog.h"
#include "main/editor/HierarchyPanel.h"
#include "main/editor/HistoryManager.h"
#include "main/editor/InspectorPanel.h"
#include "main/editor/PerformancePanel.h"
#include "main/editor/ScenePanel.h"
#include "main/editor/ThemeManager.h"
#include "main/editor/ViewportPanel.h"
#include "main/physics/Config.h"
#include "main/scenes/BoundarySystem.h"
#include "main/scenes/LoadScene.h"
#include "main/scenes/SaveScene.h"

namespace Editor {

Editor::Editor(World& world, EditorCamera& camera, InputHandler& input) {
    panels.push_back(std::make_unique<ViewportPanel>(camera, input));
    panels.push_back(std::make_unique<PerformancePanel>(camera, input));
    panels.push_back(std::make_unique<HierarchyPanel>());
    panels.push_back(std::make_unique<InspectorPanel>());
    panels.push_back(std::make_unique<ConstraintPanel>());
    panels.push_back(std::make_unique<DebugPanel>());
    panels.push_back(std::make_unique<ScenePanel>(Config::screenWidth, Config::screenHeight));

    ThemeManager::ApplyTheme(EditorTheme::Retro);

    HistoryManager::Get().RecordState(world);
}

Editor::~Editor() {
}

void Editor::Update(World& world) {
    rlImGuiBegin();

    static bool resetLayout = false;
    static bool firstFrame = true;
    ImGuiID dockspace_id = ImGui::GetID("EditorDockSpace");

    if ((firstFrame || resetLayout)) {
        firstFrame = false;
        resetLayout = false;

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiID dock_id_main = dockspace_id;
        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.18f, nullptr, &dock_id_main);
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.22f, nullptr, &dock_id_main);
        ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Down, 0.35f, nullptr, &dock_id_main);
        
        // Split main center area: Viewport (left) and Inspector (right)
        ImGuiID dock_id_inspector = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.30f, nullptr, &dock_id_main);

        // Split left column: Hierarchy (top) and Performance (bottom - taller)
        ImGuiID dock_id_perf = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.45f, nullptr, &dock_id_left);

        // Split right column: Debug (top) and Constraints (bottom)
        ImGuiID dock_id_constraints = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.50f, nullptr, &dock_id_right);

        ImGui::DockBuilderDockWindow("Viewport", dock_id_main);
        ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
        ImGui::DockBuilderDockWindow("Inspector", dock_id_inspector);
        ImGui::DockBuilderDockWindow("Debug", dock_id_right);
        ImGui::DockBuilderDockWindow("Constraints", dock_id_constraints);
        ImGui::DockBuilderDockWindow("Performance & Viewport", dock_id_perf);
        ImGui::DockBuilderDockWindow("Scene Manager", dock_id_bottom);
        
        ImGui::DockBuilderFinish(dockspace_id);
    }

    // root dockspace
    ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport());

    // shortcuts
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        if (!EditorState::Get().GetActiveScenePath().empty()) {
            SaveScene::Save(EditorState::Get().GetActiveScenePath(), world);
        } else {
            std::string path = ShowFileDialog(true);
            if (!path.empty()) {
                EditorState::Get().SetActiveScenePath(path);
                SaveScene::Save(path, world);
            }
        }
    }
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N)) {
        LoadScene::Load("", world, Config::screenWidth, Config::screenHeight);
        EditorState::Get().ClearSelection();
        EditorState::Get().SetActiveScenePath("");
        world.isPaused = true;
    }

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                LoadScene::Load("", world, Config::screenWidth, Config::screenHeight);
                EditorState::Get().ClearSelection();
                EditorState::Get().SetActiveScenePath("");
                world.isPaused = true;
            }
            if (ImGui::MenuItem("Load")) {
                std::string path = ShowFileDialog(false);
                if (!path.empty()) {
                    EditorState::Get().ClearSelection();
                    EditorState::Get().SetActiveScenePath(path);
                    world.isPaused = true;
                    LoadScene::Load(path, world, Config::screenWidth, Config::screenHeight);
                }
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                if (!EditorState::Get().GetActiveScenePath().empty()) {
                    SaveScene::Save(EditorState::Get().GetActiveScenePath(), world);
                } else {
                    std::string path = ShowFileDialog(true);
                    if (!path.empty()) {
                        EditorState::Get().SetActiveScenePath(path);
                        SaveScene::Save(path, world);
                    }
                }
            }
            if (ImGui::MenuItem("Save As")) {
                std::string path = ShowFileDialog(true);
                if (!path.empty()) {
                    EditorState::Get().SetActiveScenePath(path);
                    SaveScene::Save(path, world);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) { /* ... */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, HistoryManager::Get().CanUndo())) HistoryManager::Get().Undo(world);
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, HistoryManager::Get().CanRedo())) HistoryManager::Get().Redo(world);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Theme"))
        {
            if (ImGui::MenuItem("Modern Dark")) ThemeManager::ApplyTheme(EditorTheme::ModernDark);
            if (ImGui::MenuItem("Modern Light")) ThemeManager::ApplyTheme(EditorTheme::ModernLight);
            if (ImGui::MenuItem("Retro"))  ThemeManager::ApplyTheme(EditorTheme::Retro);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window"))
        {
            if (ImGui::MenuItem("Reset Layout")) resetLayout = true;
            ImGui::Separator();
            for (auto& panel : panels)
                ImGui::MenuItem(panel->GetName(), nullptr, &panel->isOpen);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("World"))
        {
            ImGui::TextDisabled("Physics World Settings");
            ImGui::Separator();
            
            if (ImGui::Checkbox("Use Boundary Walls", &Config::useWalls)) {
                BoundarySystem::UpdateBoundaries(world);
            }
            ImGui::Separator();

            ImGui::DragFloat2("Gravity (m/s^2)", &Config::gravity.x, 0.1f);
            ImGui::DragInt("Substeps", &Config::pipelineSubTicks, 1, 1, 100);
            ImGui::DragInt("Solver Iterations", &Config::impulseIterations, 1, 1, 100);

            ImGui::Separator();

            bool changed = false;
            changed |= ImGui::DragInt("Screen Width", &Config::screenWidth, 1, 100, 4000);
            changed |= ImGui::DragInt("Screen Height", &Config::screenHeight, 1, 100, 4000);
            
            if (changed) {
                world.SetWorldSize(Vec2(Config::screenWidth * Config::PixelToMeter, Config::screenHeight * Config::PixelToMeter));
                if (Config::useWalls) BoundarySystem::UpdateBoundaries(world);
            }

            ImGui::Separator();
            Vec2 size = world.GetWorldSize();
            if (ImGui::DragFloat2("World Size (m)", &size.x, 0.1f, 1.0f, 1000.0f)) {
                world.SetWorldSize(size);
                Config::screenWidth = (int)(size.x * Config::MeterToPixel);
                Config::screenHeight = (int)(size.y * Config::MeterToPixel);
                if (Config::useWalls) BoundarySystem::UpdateBoundaries(world);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    for (auto& panel : panels) {
        panel->OnImGui(world);
    }

    rlImGuiEnd();
}

} // namespace Editor
