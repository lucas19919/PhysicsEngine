#include "main/editor/Editor.h"
#include "main/editor/PerformancePanel.h"
#include "main/editor/ScenePanel.h"
#include "main/editor/ViewportPanel.h"
#include "main/editor/HierarchyPanel.h"
#include "main/editor/InspectorPanel.h"
#include "main/editor/DebugPanel.h"
#include "main/editor/ThemeManager.h"
#include "main/editor/FileDialog.h"
#include "external/imgui/imgui.h"
#include "external/imgui/imgui_internal.h"
#include "external/imgui/rlImGui.h"
#include "main/physics/Config.h"
#include "main/scenes/LoadScene.h"
#include "main/scenes/SaveScene.h"
#include "main/scenes/BoundarySystem.h"
#include "main/editor/EditorState.h"

namespace Editor {

Editor::Editor(World& world, EditorCamera& camera, InputHandler& input) {
    panels.push_back(std::make_unique<ViewportPanel>(camera));
    panels.push_back(std::make_unique<PerformancePanel>(camera, input));
    panels.push_back(std::make_unique<HierarchyPanel>());
    panels.push_back(std::make_unique<InspectorPanel>());
    panels.push_back(std::make_unique<DebugPanel>());
    panels.push_back(std::make_unique<ScenePanel>(Config::screenWidth, Config::screenHeight));

    ThemeManager::ApplyTheme(EditorTheme::Retro);
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
        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.20f, nullptr, &dock_id_main);
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.25f, nullptr, &dock_id_main);
        ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Down, 0.25f, nullptr, &dock_id_main);
        
        ImGuiID dock_id_bottom_left = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.30f, nullptr, &dock_id_left);
        ImGuiID dock_id_bottom_right = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.40f, nullptr, &dock_id_right);

        ImGui::DockBuilderDockWindow("Viewport", dock_id_main);
        ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
        ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
        ImGui::DockBuilderDockWindow("Performance & Viewport", dock_id_bottom_left);
        ImGui::DockBuilderDockWindow("Debug Settings", dock_id_bottom_right);
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
        EditorState::Get().SetSelected(nullptr);
        EditorState::Get().SetActiveScenePath("");
        world.isPaused = true;
    }

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                LoadScene::Load("", world, Config::screenWidth, Config::screenHeight);
                EditorState::Get().SetSelected(nullptr);
                EditorState::Get().SetActiveScenePath("");
                world.isPaused = true;
            }
            if (ImGui::MenuItem("Load")) {
                std::string path = ShowFileDialog(false);
                if (!path.empty()) {
                    EditorState::Get().SetSelected(nullptr);
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
        if (ImGui::BeginMenu("Theme"))
        {
            if (ImGui::MenuItem("Retro")) ThemeManager::ApplyTheme(EditorTheme::Retro);
            if (ImGui::MenuItem("Dark"))  ThemeManager::ApplyTheme(EditorTheme::Dark);
            if (ImGui::MenuItem("Light")) ThemeManager::ApplyTheme(EditorTheme::Light);
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

        // playbar
        float barWidth = ImGui::GetWindowWidth();
        float btnW = 30.0f;
        float groupW = (btnW * 3.0f) + (ImGui::GetStyle().ItemSpacing.x * 2.0f);
        ImGui::SetCursorPosX((barWidth - groupW) * 0.5f);
        
        if (world.isPaused) {
            if (ImGui::Button(">", ImVec2(btnW, 0))) {
                if (!EditorState::Get().HasInitialState()) {
                    EditorState::Get().CaptureInitialState(SaveScene::SerializeScene(world));
                }
                world.isPaused = false;
            }
        } else {
            if (ImGui::Button("||", ImVec2(btnW, 0))) world.isPaused = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("|>", ImVec2(btnW, 0))) {
            if (!EditorState::Get().HasInitialState()) {
                EditorState::Get().CaptureInitialState(SaveScene::SerializeScene(world));
            }
            world.isPaused = false;
            world.Step(1.0f / 60.0f);
            world.isPaused = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("R", ImVec2(btnW, 0))) {
            if (EditorState::Get().HasInitialState()) {
                EditorState::Get().SetSelected(nullptr);
                world.isPaused = true;
                LoadScene::LoadFromJSON(EditorState::Get().GetInitialState(), world, Config::screenWidth, Config::screenHeight);
                EditorState::Get().ClearInitialState();
            }
        }
        ImGui::EndMainMenuBar();
    }

    for (auto& panel : panels) {
        panel->OnImGui(world);
    }

    rlImGuiEnd();
}

} // namespace Editor
