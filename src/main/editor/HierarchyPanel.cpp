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

    // 1. Populate the groups map with all known group names (Manual + Generators)
    std::map<std::string, std::vector<GameObject*>> groups;
    for (const auto& groupName : world.GetGroups()) {
        groups[groupName] = {};
    }
    for (const auto& gen : world.GetGenerators()) {
        groups[gen.name] = {};
    }

    // 2. Assign objects to their respective groups
    std::vector<GameObject*> ungrouped;
    for (const auto& objPtr : objects) {
        GameObject* obj = objPtr.get();
        if (obj->GetGroupName().empty()) {
            ungrouped.push_back(obj);
        } else {
            // This also handles groups that weren't explicitly added to World::groups
            groups[obj->GetGroupName()].push_back(obj);
        }
    }

    size_t idToRemove = (size_t)-1;
    std::string groupToRemove = "";
    static bool openCreateGroupPopup = false;

    auto renderNode = [&](GameObject* obj) {
        std::string objName = obj->GetName();
        if (objName.empty()) {
            objName = "GameObject " + std::to_string(obj->GetID());
        }
        
        ImGui::PushID((int)obj->GetID());
        ImGuiTreeNodeFlags flags = ((selected == obj) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool opened = ImGui::TreeNodeEx(objName.c_str(), flags);
        
        if (ImGui::IsItemClicked()) {
            EditorState::Get().SetSelected(obj);
        }

        if (ImGui::BeginDragDropSource()) {
            size_t id = obj->GetID();
            ImGui::SetDragDropPayload("HIERARCHY_OBJ", &id, sizeof(size_t));
            ImGui::Text("Moving %s", objName.c_str());
            ImGui::EndDragDropSource();
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
        ImGui::PopID();
    };

    // 3. Render Groups (Folders)
    for (auto& pair : groups) {
        ImGui::PushID(pair.first.c_str());
        ImGuiTreeNodeFlags folderFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (EditorState::Get().GetSelectedGroup() == pair.first) folderFlags |= ImGuiTreeNodeFlags_Selected;

        bool opened = ImGui::TreeNodeEx(pair.first.c_str(), folderFlags);
        
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            EditorState::Get().SetSelectedGroup(pair.first);
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_OBJ")) {
                size_t id = *(const size_t*)payload->Data;
                for (auto& objPtr : world.GetGameObjects()) {
                    if (objPtr->GetID() == id) {
                        objPtr->SetGroupName(pair.first);
                        break;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginPopupContextItem()) {
            static char renameBuf[64];
            if (ImGui::MenuItem("Rename Group")) {
                strncpy(renameBuf, pair.first.c_str(), 63);
                ImGui::OpenPopup("RenameGroupPopup");
            }

            if (ImGui::BeginPopup("RenameGroupPopup")) {
                ImGui::InputText("##newname", renameBuf, 63);
                if (ImGui::Button("Apply")) {
                    std::string newName = renameBuf;
                    if (!newName.empty()) {
                        world.RenameGenerator(pair.first, newName);
                        for (GameObject* obj : pair.second) {
                            obj->SetGroupName(newName);
                        }
                        EditorState::Get().SetSelectedGroup(newName);
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if (ImGui::MenuItem("Save as Prefab")) {
                std::string path = "../assets/prefabs/" + pair.first + ".json";
                SaveScene::SaveCollection(path, world, pair.first);
            }

            if (ImGui::MenuItem("Delete Group")) {
                groupToRemove = pair.first;
            }

            ImGui::EndPopup();
        }

        if (opened) {
            for (GameObject* obj : pair.second) {
                renderNode(obj);
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    // 4. Render Ungrouped (Only if there are ungrouped objects)
    if (!ungrouped.empty()) {
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0,0,0,0));
        bool ungroupedNode = ImGui::TreeNodeEx("Ungrouped", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf);
        ImGui::PopStyleColor(2);

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_OBJ")) {
                size_t id = *(const size_t*)payload->Data;
                for (auto& objPtr : world.GetGameObjects()) {
                    if (objPtr->GetID() == id) {
                        objPtr->SetGroupName("");
                        break;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ungroupedNode) {
            for (GameObject* obj : ungrouped) {
                renderNode(obj);
            }
            ImGui::TreePop();
        }
    }

    // 5. Cleanup and Popups
    if (idToRemove != (size_t)-1) {
        if (selected && selected->GetID() == idToRemove) EditorState::Get().SetSelected(nullptr);
        world.RemoveGameObject(idToRemove);
    }

    if (!groupToRemove.empty()) {
        world.RemoveGroup(groupToRemove);
    }

    if (openCreateGroupPopup) {
        ImGui::OpenPopup("CreateGroupPopup");
        openCreateGroupPopup = false;
    }

    if (ImGui::BeginPopup("CreateGroupPopup")) {
        static char groupNameBuf[64] = "New Group";
        ImGui::InputText("Group Name", groupNameBuf, 63);
        if (ImGui::Button("Create") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            std::string name = groupNameBuf;
            // Ensure unique name
            int counter = 1;
            while (std::find(world.GetGroups().begin(), world.GetGroups().end(), name) != world.GetGroups().end()) {
                name = std::string(groupNameBuf) + " " + std::to_string(counter++);
            }
            world.AddGroup(name);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Global Context Menu
    if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::BeginMenu("Create...")) {
            if (ImGui::MenuItem("Group")) {
                openCreateGroupPopup = true;
            }
            ImGui::Separator();
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

                                    std::string uniqueGroupName = name;
                                    int counter = 1;
                                    bool exists = false;
                                    for (const auto& obj : world.GetGameObjects()) if (obj->GetGroupName() == uniqueGroupName) { exists = true; break; }
                                    while (exists) {
                                        uniqueGroupName = name + "_" + std::to_string(counter++);
                                        exists = false;
                                        for (const auto& obj : world.GetGameObjects()) if (obj->GetGroupName() == uniqueGroupName) { exists = true; break; }
                                    }

                                    if (data.contains("objects")) {
                                        LoadScene::LoadCollection(data, world, Vec2(0,0), uniqueGroupName);
                                        EditorState::Get().SetSelectedGroup(uniqueGroupName);
                                    } else {
                                        GameObject* obj = LoadScene::LoadObject(data, world);
                                        if (obj) {
                                            obj->transform.SetPosition(Vec2(0,0));
                                            obj->SetGroupName(uniqueGroupName);
                                            EditorState::Get().SetSelectedGroup(uniqueGroupName);
                                        }
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
