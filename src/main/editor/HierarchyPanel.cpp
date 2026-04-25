#include "main/editor/HierarchyPanel.h"
#include "external/imgui/imgui.h"
#include "main/editor/EditorState.h"
#include "main/utility/Instantiate.h"
#include "main/scenes/LoadScene.h"
#include "main/scenes/SaveScene.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/Renderer.h"
#include "main/components/RigidBody.h"
#include <string>
#include <map>
#include <vector>
#include <filesystem>
#include <fstream>

namespace Editor {

void HierarchyPanel::OnImGui(World& world) {
    if (!isOpen) return;

    ImGui::Begin(GetName(), &isOpen);

    const auto& objects = world.GetGameObjects();
    GameObject* selected = EditorState::Get().GetSelected();

    // Group objects by groupName
    std::map<std::string, std::vector<GameObject*>> groups;
    std::vector<GameObject*> ungrouped;

    for (const auto& objPtr : objects) {
        GameObject* obj = objPtr.get();
        if (obj->GetGroupName().empty()) {
            ungrouped.push_back(obj);
        } else {
            groups[obj->GetGroupName()].push_back(obj);
        }
    }

    size_t idToRemove = (size_t)-1;

    auto renderNode = [&](GameObject* obj) {
        std::string objName = obj->GetName();
        if (objName.empty()) {
            objName = "GameObject " + std::to_string(obj->GetID());
        }
        
        std::string label = objName;
        ImGuiTreeNodeFlags flags = ((selected == obj) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool opened = ImGui::TreeNodeEx((void*)(intptr_t)obj->GetID(), flags, "%s", label.c_str());
        
        if (ImGui::IsItemClicked()) {
            EditorState::Get().SetSelected(obj);
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
                idToRemove = obj->GetID();
            }
            ImGui::EndPopup();
        }

        if (opened) {
            ImGui::TreePop();
        }
    };

    // Render Groups (Folders)
    for (auto& pair : groups) {
        ImGuiTreeNodeFlags folderFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (EditorState::Get().GetSelectedGroup() == pair.first) folderFlags |= ImGuiTreeNodeFlags_Selected;

        bool opened = ImGui::TreeNodeEx(pair.first.c_str(), folderFlags);
        
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            EditorState::Get().SetSelectedGroup(pair.first);
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Save as Prefab")) {
                std::string path = "../assets/prefabs/" + pair.first + ".json";
                SaveScene::SaveCollection(path, world, pair.first);
            }
            ImGui::EndPopup();
        }

        if (opened) {
            for (GameObject* obj : pair.second) {
                renderNode(obj);
            }
            ImGui::TreePop();
        }
    }

    // Render Ungrouped
    for (GameObject* obj : ungrouped) {
        renderNode(obj);
    }

    if (idToRemove != (size_t)-1) {
        if (selected && selected->GetID() == idToRemove) {
            EditorState::Get().SetSelected(nullptr);
        }
        world.RemoveGameObject(idToRemove);
    }

    // Context menu for the whole panel
    if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::BeginMenu("Create...")) {
            if (ImGui::MenuItem("Box")) {
                Instantiate()
                    .WithTransform(Vec2(0,0), 0)
                    .WithCollider(ColliderType::BOX, Vec2(1.0f, 1.0f))
                    .WithRenderer(Shape{RenderShape::R_BOX, WHITE, Vec2(1.0f, 1.0f)})
                    .WithRigidBody(
                        Properties{1.0f, 0.5f, 0.5f, 0.5f},
                        LinearState{Vec2(0,0), Vec2(0,0), Vec2(0,0)},
                        AngularState{0, 0, 0},
                        Settings{true}
                    )
                    .Create(world, -1)->SetName("New Box");
            }
            if (ImGui::MenuItem("Circle")) {
                Instantiate()
                    .WithTransform(Vec2(0,0), 0)
                    .WithCollider(ColliderType::CIRCLE, 0.5f)
                    .WithRenderer(Shape{RenderShape::R_CIRCLE, WHITE, 0.5f})
                    .WithRigidBody(
                        Properties{1.0f, 0.5f, 0.5f, 0.5f},
                        LinearState{Vec2(0,0), Vec2(0,0), Vec2(0,0)},
                        AngularState{0, 0, 0},
                        Settings{true}
                    )
                    .Create(world, -1)->SetName("New Circle");
            }
            if (ImGui::MenuItem("Empty Object")) {
                Instantiate()
                    .WithTransform(Vec2(0,0), 0)
                    .Create(world, -1)->SetName("New GameObject");
            }

            if (ImGui::BeginMenu("Prefab")) {
                std::string prefabDir = "../assets/prefabs";
                if (std::filesystem::exists(prefabDir)) {
                    for (const auto& entry : std::filesystem::directory_iterator(prefabDir)) {
                        if (entry.path().extension() == ".json") {
                            std::string name = entry.path().stem().string();
                            if (ImGui::MenuItem(name.c_str())) {
                                std::ifstream file(entry.path());
                                if (file.is_open()) {
                                    nlohmann::json data;
                                    file >> data;
                                    if (data.contains("objects")) {
                                        LoadScene::LoadCollection(data, world, Vec2(0,0));
                                    } else {
                                        GameObject* obj = LoadScene::LoadObject(data, world);
                                        if (obj) obj->transform.SetPosition(Vec2(0,0));
                                    }
                                }
                            }
                        }
                    }
                } else {
                    ImGui::TextDisabled("No prefabs directory");
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Generator")) {
                std::string baseName = "New Generator";
                std::string uniqueName = baseName;
                int counter = 1;
                while (world.GetGenerator(uniqueName) != nullptr) {
                    uniqueName = baseName + " " + std::to_string(counter++);
                }

                GeneratorDef def;
                def.name = uniqueName;
                def.rows = 2;
                def.columns = 2;
                def.startX = 0;
                def.startY = 0;
                def.spacingX = 2.0f;
                def.spacingY = 2.0f;
                
                // Default template is a simple box
                def.templateObject = std::make_unique<GameObject>();
                def.templateObject->AddComponent(std::make_unique<BoxCollider>(Vec2(1.0f, 1.0f)));
                def.templateObject->AddComponent(std::make_unique<Renderer>(Shape{RenderShape::R_BOX, WHITE, Vec2(1.0f, 1.0f)}));
                def.templateObject->AddComponent(std::make_unique<RigidBody>(
                    Properties{1.0f, 0.5f, 0.5f, 0.5f},
                    LinearState{Vec2(0,0), Vec2(0,0), Vec2(0,0)},
                    AngularState{0, 0, 0},
                    Settings{true}
                ));

                world.AddGenerator(std::move(def));
                // Initial generation
                world.RegenerateGenerator(uniqueName);
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

} // namespace Editor
