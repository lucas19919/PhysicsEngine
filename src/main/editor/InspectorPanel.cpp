#include "main/editor/InspectorPanel.h"
#include "external/imgui/imgui.h"
#include "main/editor/EditorState.h"
#include "main/scenes/LoadScene.h"
#include "main/scenes/SaveScene.h"
#include "main/components/TransformComponent.h"
#include "main/components/RigidBody.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/Renderer.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Motor.h"
#include <string>
#include <cstring>
#include <fstream>
#include <iomanip>

namespace Editor {

void InspectorPanel::OnImGui(World& world) {
    if (!isOpen) return;

    ImGui::Begin(GetName(), &isOpen);

    std::string selectedGroup = EditorState::Get().GetSelectedGroup();
    GameObject* selected = EditorState::Get().GetSelected();

    if (!selectedGroup.empty()) {
        GeneratorDef* genDef = world.GetGenerator(selectedGroup);
        if (genDef) {
            // Name editing for generator at the very top
            std::string oldName = genDef->name;
            char nameBuf[128];
            strncpy(nameBuf, oldName.c_str(), sizeof(nameBuf));
            if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::IsItemDeactivatedAfterEdit()) {
                std::string newName = nameBuf;
                if (!newName.empty() && newName != oldName) {
                    world.RenameGenerator(oldName, newName);
                    EditorState::Get().SetSelectedGroup(newName);
                    selectedGroup = newName; // Update local variable for consistency
                }
            }

            ImGui::Separator();
            ImGui::TextDisabled("Generator Settings");

            bool changed = false;
            changed |= ImGui::DragInt("Rows", &genDef->rows, 1, 1, 100);
            changed |= ImGui::DragInt("Columns", &genDef->columns, 1, 1, 100);
            changed |= ImGui::DragFloat2("Start Pos", &genDef->startX, 0.1f);
            changed |= ImGui::DragFloat2("Spacing", &genDef->spacingX, 0.1f);

            if (changed) {
                world.RegenerateGenerator(selectedGroup);
            }

            ImGui::Separator();
            ImGui::TextDisabled("Template Object Properties");
            ImGui::PushID("TemplateObject");
            
            bool templateChanged = false;
            // Inspect template components
            if (genDef->templateObject) {
                const auto& components = genDef->templateObject->GetComponents();
                Component* componentToRemove = nullptr;

                for (size_t i = 0; i < components.size(); i++) {
                    ImGui::PushID((int)i);
                    bool open = ImGui::CollapsingHeader(components[i]->GetName(), ImGuiTreeNodeFlags_DefaultOpen);
                    
                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Delete Component")) {
                            componentToRemove = components[i].get();
                        }
                        ImGui::EndPopup();
                    }

                    if (open) {
                        if (components[i]->OnInspectorGui(&world)) {
                            templateChanged = true;
                        }
                    }
                    ImGui::PopID();
                }

                if (componentToRemove) {
                    genDef->templateObject->RemoveComponent(componentToRemove);
                    templateChanged = true;
                }

                ImGui::Separator();
        
                // Add Component Button for Template
                if (ImGui::Button("Add Component...", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                    ImGui::OpenPopup("AddTemplateComponentPopup");
                }

                if (ImGui::BeginPopup("AddTemplateComponentPopup")) {
                    if (genDef->templateObject->rb == nullptr) {
                        if (ImGui::MenuItem("RigidBody")) {
                            genDef->templateObject->AddComponent(std::make_unique<RigidBody>(
                                Properties{1.0f, 0.5f, 0.5f, 0.5f},
                                LinearState{Vec2(0,0), Vec2(0,0), Vec2(0,0)},
                                AngularState{0, 0, 0},
                                Settings{true}
                            ));
                            templateChanged = true;
                            ImGui::CloseCurrentPopup();
                        }
                    }

                    if (genDef->templateObject->c == nullptr) {
                        if (ImGui::BeginMenu("Collider")) {
                            if (ImGui::MenuItem("Box Collider")) {
                                genDef->templateObject->AddComponent(std::make_unique<BoxCollider>(Vec2(1.0f, 1.0f)));
                                templateChanged = true;
                                ImGui::CloseCurrentPopup();
                            }
                            if (ImGui::MenuItem("Circle Collider")) {
                                genDef->templateObject->AddComponent(std::make_unique<CircleCollider>(0.5f));
                                templateChanged = true;
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::EndMenu();
                        }
                    }

                    if (genDef->templateObject->GetComponent<Renderer>() == nullptr) {
                        if (ImGui::MenuItem("Renderer")) {
                            genDef->templateObject->AddComponent(std::make_unique<Renderer>(Shape{RenderShape::R_BOX, WHITE, Vec2(1.0f, 1.0f)}));
                            templateChanged = true;
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    
                    ImGui::EndPopup();
                }
            }

            if (templateChanged) {
                world.RegenerateGenerator(selectedGroup);
            }

            ImGui::PopID(); // TemplateObject
        } else {
            ImGui::Text("Group: %s", selectedGroup.c_str());
            ImGui::TextDisabled("(No generator definition found)");
        }
    }
    else if (selected) {
        ImGui::PushID((int)selected->GetID());

        // Name Editing
        char nameBuf[128];
        strncpy(nameBuf, selected->GetName().c_str(), sizeof(nameBuf));
        nameBuf[sizeof(nameBuf) - 1] = '\0';
        if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
            selected->SetName(nameBuf);
        }

        ImGui::SameLine();
        if (ImGui::Button("Save Prefab")) {
            std::string path = "../assets/prefabs/" + selected->GetName() + ".json";
            std::ofstream file(path);
            if (file.is_open()) {
                file << std::setw(4) << SaveScene::SerializeObject(selected) << std::endl;
            }
        }

        ImGui::Text("ID: %zu", selected->GetID());
        ImGui::Separator();

        // Transform is a special component (inline struct)
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            selected->transform.OnInspectorGui(&world);
        }

        // Iterate through all other components
        const auto& components = selected->GetComponents();
        Component* componentToRemove = nullptr;

        for (size_t i = 0; i < components.size(); i++) {
            ImGui::PushID((int)i);
            
            bool open = ImGui::CollapsingHeader(components[i]->GetName(), ImGuiTreeNodeFlags_DefaultOpen);
            
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete Component")) {
                    componentToRemove = components[i].get();
                }
                ImGui::EndPopup();
            }

            if (open) {
                components[i]->OnInspectorGui(&world);
            }
            ImGui::PopID();
        }

        if (componentToRemove) {
            selected->RemoveComponent(componentToRemove);
        }

        // Constraints Section
        ImGui::Separator();
        ImGui::TextDisabled("Constraints");

        auto constraints = world.GetConstraintsForObject(selected);
        Constraint* constraintToRemove = nullptr;

        for (size_t i = 0; i < constraints.size(); i++) {
            ImGui::PushID((int)(i + 1000));
            bool open = ImGui::CollapsingHeader(constraints[i]->GetName(), ImGuiTreeNodeFlags_DefaultOpen);
            
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete Constraint")) {
                    constraintToRemove = constraints[i];
                }
                ImGui::EndPopup();
            }

            if (open) {
                constraints[i]->OnInspectorGui(&world);
            }
            ImGui::PopID();
        }

        if (constraintToRemove) {
            world.RemoveConstraint(constraintToRemove->GetID());
        }

        ImGui::Separator();

        // Add Constraint Button
        if (ImGui::Button("Add Constraint...", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            ImGui::OpenPopup("AddConstraintPopup");
        }

        if (ImGui::BeginPopup("AddConstraintPopup")) {
            bool inPickingMode = (EditorState::Get().GetPickingMode() == EditorState::PickingMode::CONSTRAINT_TARGET);
            if (inPickingMode) {
                ImGui::Text("Click another object in viewport...");
                if (ImGui::Button("Cancel Picking")) EditorState::Get().ClearPickingMode();
            } else {
                if (ImGui::MenuItem("Distance Constraint")) {
                    EditorState::Get().SetPickingMode(EditorState::PickingMode::CONSTRAINT_TARGET, ConstraintType::DISTANCE);
                }
                if (ImGui::MenuItem("Joint Constraint")) {
                    EditorState::Get().SetPickingMode(EditorState::PickingMode::CONSTRAINT_TARGET, ConstraintType::JOINT);
                }
                if (ImGui::MenuItem("Pin Constraint")) {
                    world.AddConstraint(std::make_unique<PinConstraint>(
                        std::vector<PinAttachment>{{selected, 0.0f, 0.0f}}, 
                        selected->transform.position, true, true));
                }
                if (ImGui::MenuItem("Motor Constraint")) {
                    world.AddConstraint(std::make_unique<MotorConstraint>(selected, Vec2(), 10.0f));
                }
            }
            ImGui::EndPopup();
        }

        ImGui::Separator();
        
        // Add Component Button
        if (ImGui::Button("Add Component...", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup")) {
            // RigidBody check
            if (selected->rb == nullptr) {
                if (ImGui::MenuItem("RigidBody")) {
                    selected->AddComponent(std::make_unique<RigidBody>(
                        Properties{1.0f, 0.5f, 0.5f, 0.5f},
                        LinearState{Vec2(0,0), Vec2(0,0), Vec2(0,0)},
                        AngularState{0, 0, 0},
                        Settings{true}
                    ));
                    ImGui::CloseCurrentPopup();
                }
            }

            // Collider check (only one collider allowed for now)
            if (selected->c == nullptr) {
                if (ImGui::BeginMenu("Collider")) {
                    if (ImGui::MenuItem("Box Collider")) {
                        selected->AddComponent(std::make_unique<BoxCollider>(Vec2(1.0f, 1.0f)));
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Circle Collider")) {
                        selected->AddComponent(std::make_unique<CircleCollider>(0.5f));
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndMenu();
                }
            }

            // Renderer check
            if (selected->GetComponent<Renderer>() == nullptr) {
                if (ImGui::MenuItem("Renderer")) {
                    selected->AddComponent(std::make_unique<Renderer>(Shape{RenderShape::R_BOX, WHITE, Vec2(1.0f, 1.0f)}));
                    ImGui::CloseCurrentPopup();
                }
            }
            
            ImGui::EndPopup();
        }

        ImGui::PopID(); // selected object ID
    } else {
        ImGui::TextDisabled("Select an object to inspect");
    }

    ImGui::End();
}

} // namespace Editor
