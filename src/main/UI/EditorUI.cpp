#include "main/UI/EditorUI.h"
#include "external/imgui/imgui.h"
#include "external/imgui/rlImGui.h"
#include "main/physics/Config.h"
#include "main/scenes/LoadScene.h"
#include "main/components/RigidBody.h"
#include "main/components/Renderer.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/collidertypes/PolygonCollider.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Motor.h"
#include <vector>
#include <algorithm>
#include <cstdio>

EditorUI::EditorUI() {
    showFPS = Config::drawFPS;
}

void EditorUI::Draw(World& world, std::string& selectedFile, int viewportWidth, int viewportHeight, int currentWidth, int currentHeight) {
    rlImGuiBegin();

    DrawPhysicsTools(world, selectedFile, viewportWidth, viewportHeight, currentHeight);
    DrawInspectorAndScene(world, currentWidth, currentHeight);

    rlImGuiEnd();

    if (showFPS) DrawFPS(sidebarWidth + 10, 10);
}

void EditorUI::DrawPhysicsTools(World& world, std::string& selectedFile, int viewportWidth, int viewportHeight, int currentHeight) {
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ (float)sidebarWidth, (float)currentHeight });
    ImGui::Begin("Physics Tools", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    
    if (ImGui::CollapsingHeader("Level Loading", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputText("Path", filePathBuffer, 256);
        if (ImGui::Button("Load Level", ImVec2(-1, 0))) {
            selectedFile = filePathBuffer;
            world.isPaused = true;
            world.Clear();
            LoadScene::Load(selectedFile, world, viewportWidth, viewportHeight);  
        }
        if (!selectedFile.empty()) ImGui::TextWrapped("Active: %s", selectedFile.c_str());
    }
    
    ImGui::Separator();
    
    if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Paused (Space)", &world.isPaused);
        ImGui::DragFloat2("Gravity", &Config::gravity.x, 1.0f);
        ImGui::SliderInt("Sub-ticks", &Config::pipelineSubTicks, 1, 32);
        ImGui::SliderInt("Impulse Iter", &Config::impulseIterations, 1, 20);
        ImGui::SliderInt("Position Iter", &Config::positionIterations, 1, 20);
        ImGui::Checkbox("Warm Starting", &Config::warmStart);
    }
    
    ImGui::Separator();
    
    if (ImGui::CollapsingHeader("Visualization", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable Debug Draw", &Config::debugDraw);
        if (Config::debugDraw) {
            ImGui::Checkbox("Draw AABBs", &Config::drawAABB);
            ImGui::Checkbox("Draw Contact Points", &Config::drawContactPoints);
            ImGui::Checkbox("Draw Contact Normals", &Config::drawContactNormals);
            ImGui::Checkbox("Draw Velocities", &Config::drawVelocities);
            ImGui::Checkbox("Draw Accelerations", &Config::drawAccelerations);
            ImGui::Checkbox("Draw Spatial Grid", &Config::drawGrid);
            ImGui::Checkbox("Draw FPS", &showFPS);
            Config::drawFPS = showFPS;
        }
    }
    ImGui::End();
}

void EditorUI::DrawInspectorAndScene(World& world, int currentWidth, int currentHeight) {
    ImGui::SetNextWindowPos({ (float)(currentWidth - sidebarWidth), 0 });
    ImGui::SetNextWindowSize({ (float)sidebarWidth, (float)currentHeight });
    ImGui::Begin("Inspector & Scene", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    
    if (ImGui::BeginTabBar("RightTabs")) {
        if (ImGui::BeginTabItem("Scene Tree")) {
            ImGui::Text("Sort:"); ImGui::SameLine();
            if (ImGui::RadioButton("ID", sortMode == SortMode::ID)) sortMode = SortMode::ID; ImGui::SameLine();
            if (ImGui::RadioButton("Mass", sortMode == SortMode::Mass)) sortMode = SortMode::Mass; ImGui::SameLine();
            if (ImGui::RadioButton("Type", sortMode == SortMode::Type)) sortMode = SortMode::Type;

            ImGui::Separator();
            if (ImGui::BeginChild("TreeList")) {
                auto& gameObjects = world.GetGameObjects();
                std::vector<GameObject*> sortedObjs;
                for (auto& obj : gameObjects) sortedObjs.push_back(obj.get());

                std::sort(sortedObjs.begin(), sortedObjs.end(), [this](GameObject* a, GameObject* b) {
                    if (sortMode == SortMode::ID) return a->GetID() < b->GetID();
                    if (sortMode == SortMode::Mass) return (a->rb ? a->rb->GetMass() : 0) > (b->rb ? b->rb->GetMass() : 0);
                    if (sortMode == SortMode::Type) {
                        int typeA = a->c ? (int)a->c->GetType() : -1;
                        int typeB = b->c ? (int)b->c->GetType() : -1;
                        return typeA < typeB;
                    }
                    return false;
                });

                for (auto obj : sortedObjs) {
                    char label[64];
                    const char* typeStr = "Empty";
                    if (obj->c) {
                        if (obj->c->GetType() == ColliderType::CIRCLE) typeStr = "Circle";
                        else if (obj->c->GetType() == ColliderType::BOX) typeStr = "Box";
                        else typeStr = "Polygon";
                    }
                    sprintf(label, "[%zu] %s##%p", obj->GetID(), typeStr, (void*)obj);
                    if (ImGui::Selectable(label, world.selectedObject == obj)) world.selectedObject = obj;
                }
                ImGui::EndChild();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Inspector")) {
            if (world.selectedObject) {
                GameObject* obj = world.selectedObject;
                ImGui::Text("ID: %zu", obj->GetID());
                
                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (ImGui::DragFloat2("Position", &obj->transform.position.x, 1.0f)) obj->transform.isDirty = true;
                    float rot = obj->transform.rotation * RAD2DEG;
                    if (ImGui::DragFloat("Rotation", &rot, 1.0f)) {
                        obj->transform.rotation = rot * DEG2RAD;
                        obj->transform.isDirty = true;
                    }
                }

                if (obj->rb && ImGui::CollapsingHeader("RigidBody", ImGuiTreeNodeFlags_DefaultOpen)) {
                    float m = obj->rb->GetMass();
                    if (ImGui::DragFloat("Mass", &m, 0.1f, 0.001f, 10000.0f)) obj->rb->SetMass(m);
                    
                    float r = obj->rb->GetRestitution();
                    if (ImGui::SliderFloat("Restitution", &r, 0.0f, 1.0f)) obj->rb->SetRestitution(r);
                    
                    float f = obj->rb->GetFriction();
                    if (ImGui::SliderFloat("Friction", &f, 0.0f, 1.0f)) obj->rb->SetFriction(f);
                    
                    float i = obj->rb->GetInertia();
                    if (ImGui::DragFloat("Inertia", &i, 1.0f, 0.1f, 1000000.0f)) obj->rb->SetInertia(i);

                    Vec2 v = obj->rb->GetVelocity();
                    if (ImGui::DragFloat2("Velocity", &v.x, 1.0f)) obj->rb->SetVelocity(v);
                    
                    float av = obj->rb->GetAngularVelocity();
                    if (ImGui::DragFloat("Angular Vel", &av, 0.1f)) obj->rb->SetAngularVelocity(av);

                    bool useGrav = obj->rb->IsGravityEnabled();
                    if (ImGui::Checkbox("Use Gravity", &useGrav)) obj->rb->SetGravity(useGrav);
                }

                if (obj->c && ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Checkbox("Is Active", &obj->c->isActive);
                    if (obj->c->GetType() == ColliderType::CIRCLE) {
                        CircleCollider* cc = static_cast<CircleCollider*>(obj->c);
                        if (ImGui::DragFloat("Radius", &cc->radius, 1.0f, 0.1f, 1000.0f)) obj->transform.isDirty = true;
                    } else if (obj->c->GetType() == ColliderType::BOX) {
                        BoxCollider* bc = static_cast<BoxCollider*>(obj->c);
                        if (ImGui::DragFloat2("Size", &bc->size.x, 1.0f, 0.1f, 2000.0f)) obj->transform.isDirty = true;
                    } else if (obj->c->GetType() == ColliderType::POLYGON) {
                        PolygonCollider* pc = static_cast<PolygonCollider*>(obj->c);
                        ImGui::Text("Vertices: %zu", pc->vertices.Size());
                        for (size_t i = 0; i < pc->vertices.Size(); i++) {
                            char label[32]; sprintf(label, "V%zu", i);
                            if (ImGui::DragFloat2(label, &pc->vertices[i].x, 1.0f)) obj->transform.isDirty = true;
                        }
                    }
                }

                if (Renderer* ren = obj->GetComponent<Renderer>()) {
                    if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
                        Shape s = ren->GetShape();
                        float color[4] = { s.color.r/255.f, s.color.g/255.f, s.color.b/255.f, s.color.a/255.f };
                        if (ImGui::ColorEdit4("Color", color)) {
                            ren->SetColor({(unsigned char)(color[0]*255), (unsigned char)(color[1]*255), (unsigned char)(color[2]*255), (unsigned char)(color[3]*255)});
                        }
                        
                        if (s.form == RenderShape::R_CIRCLE) {
                            float rad = std::get<float>(s.scale);
                            if (ImGui::DragFloat("Scale Radius", &rad, 1.0f)) ren->SetScale(rad);
                        } else if (s.form == RenderShape::R_BOX) {
                            Vec2 size = std::get<Vec2>(s.scale);
                            if (ImGui::DragFloat2("Scale Size", &size.x, 1.0f)) ren->SetScale(size);
                        } else if (s.form == RenderShape::R_POLYGON) {
                            Array<20> verts = std::get<Array<20>>(s.scale);
                            bool changed = false;
                            for (size_t i = 0; i < verts.Size(); i++) {
                                char label[32]; sprintf(label, "SV%zu", i);
                                if (ImGui::DragFloat2(label, &verts[i].x, 1.0f)) changed = true;
                            }
                            if (changed) ren->SetScale(verts);
                        }
                    }
                }
            } else {
                ImGui::Text("Select an object to inspect.");
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Constraints")) {
            if (ImGui::BeginChild("ConstraintList")) {
                auto& constraints = world.GetConstraints();
                for (size_t i = 0; i < constraints.size(); i++) {
                    Constraint* c = constraints[i].get();
                    char label[64];
                    const char* typeStr = "Unknown";
                    if (c->GetType() == ConstraintType::DISTANCE) typeStr = "Distance";
                    else if (c->GetType() == ConstraintType::PIN) typeStr = "Pin";
                    else if (c->GetType() == ConstraintType::JOINT) typeStr = "Joint";
                    else if (c->GetType() == ConstraintType::MOTOR) typeStr = "Motor";
                    
                    sprintf(label, "[%zu] %s##%p", c->GetID(), typeStr, (void*)c);
                    if (ImGui::CollapsingHeader(label)) {
                        if (c->GetType() == ConstraintType::DISTANCE) {
                            DistanceConstraint* dc = static_cast<DistanceConstraint*>(c);
                            ImGui::Text("Anchor ID: %zu", dc->anchor->GetID());
                            ImGui::Text("Attached ID: %zu", dc->attached->GetID());
                            ImGui::DragFloat("Length", &dc->length, 1.0f);
                            ImGui::DragFloat2("Anchor Offset", &dc->anchorOffset.x, 1.0f);
                            ImGui::DragFloat2("Attached Offset", &dc->attachedOffset.x, 1.0f);
                        } else if (c->GetType() == ConstraintType::PIN) {
                            PinConstraint* pc = static_cast<PinConstraint*>(c);
                            ImGui::DragFloat2("World Position", &pc->position.x, 1.0f);
                            ImGui::Checkbox("Fixed X", &pc->fixedX);
                            ImGui::Checkbox("Fixed Y", &pc->fixedY);
                            for (size_t j = 0; j < pc->attachments.size(); j++) {
                                ImGui::BulletText("Obj ID: %zu", pc->attachments[j].obj->GetID());
                                char olabel[32]; sprintf(olabel, "Local Anchor##%zu", j);
                                ImGui::DragFloat2(olabel, &pc->attachments[j].localAnchor.x, 1.0f);
                            }
                        } else if (c->GetType() == ConstraintType::JOINT) {
                            JointConstraint* jc = static_cast<JointConstraint*>(c);
                            ImGui::DragFloat2("World Position", &jc->position.x, 1.0f);
                            ImGui::Checkbox("Collisions", &jc->collisions);
                            for (size_t j = 0; j < jc->attachments.size(); j++) {
                                ImGui::BulletText("Obj ID: %zu", jc->attachments[j].obj->GetID());
                                char olabel[32]; sprintf(olabel, "Local Anchor##%zu", j);
                                ImGui::DragFloat2(olabel, &jc->attachments[j].localAnchor.x, 1.0f);
                            }
                        } else if (c->GetType() == ConstraintType::MOTOR) {
                            MotorConstraint* mc = static_cast<MotorConstraint*>(c);
                            ImGui::Text("Rotor ID: %zu", mc->rotor->GetID());
                            ImGui::DragFloat("Torque", &mc->torque, 10.0f);
                            ImGui::DragFloat2("Local Position", &mc->localPosition.x, 1.0f);
                        }
                    }
                }
                ImGui::EndChild();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}