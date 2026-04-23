#include "raylib.h"
#include "external/imgui/imgui.h"
#include "external/imgui/rlImGui.h"
#include "main/World.h"
#include "main/GameObject.h"
#include "main/scenes/LoadScene.h"
#include "main/utility/Draw.h"
#include "main/utility/InputHandler.h"
#include "main/physics/Config.h"
#include "main/components/RigidBody.h"
#include "main/components/Renderer.h"
#include <string>
#include <vector>
#include <algorithm>

enum class SortMode { ID, Mass, Type };

int main() {
    // Layout configuration
    const int sidebarWidth = 300;
    const int viewportWidth = Config::screenWidth;
    const int viewportHeight = Config::screenHeight;
    const int totalWidth = viewportWidth + (sidebarWidth * 2);
    const int totalHeight = viewportHeight;

    InitWindow(totalWidth, totalHeight, "Halliday2D - Editor");
    SetTargetFPS(Config::targetFPS);    

    rlImGuiSetup(true);

    World world;
    InputHandler input;

    // Editor Camera to shift the view into the center "clear" area
    Camera2D camera = { 0 };
    camera.offset = { (float)sidebarWidth, 0 };
    camera.target = { 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    std::string selectedFile = "../assets/examples/PrattTruss.json";
    char filePathBuffer[256] = "../assets/examples/";

    LoadScene::Load(selectedFile, world, viewportWidth, viewportHeight);

    bool FPS = Config::drawFPS;
    static SortMode sortMode = SortMode::ID;

    const float dt = 1.0f / 60.0f;
    while (!WindowShouldClose()) {
        input.Update(world, selectedFile, viewportWidth, viewportHeight, dt);
        world.Step(dt);

        BeginDrawing();
            ClearBackground({ 30, 30, 30, 255 }); // Dark editor background

            // --- VIEWPORT RENDERING ---
            // Draw a background for the actual game area
            DrawRectangle(sidebarWidth, 0, viewportWidth, viewportHeight, RAYWHITE);
            
            BeginMode2D(camera);
                Render(world); 
            EndMode2D();

            rlImGuiBegin();
            
            // --- LEFT SIDEBAR: PHYSICS TOOLS ---
            ImGui::SetNextWindowPos({ 0, 0 });
            ImGui::SetNextWindowSize({ (float)sidebarWidth, (float)totalHeight });
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
                    ImGui::Checkbox("Draw FPS", &FPS);
                    Config::drawFPS = FPS;
                }
            }
            ImGui::End();

            // --- RIGHT SIDEBAR: SCENE & INSPECTOR ---
            ImGui::SetNextWindowPos({ (float)(totalWidth - sidebarWidth), 0 });
            ImGui::SetNextWindowSize({ (float)sidebarWidth, (float)totalHeight });
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

                        std::sort(sortedObjs.begin(), sortedObjs.end(), [](GameObject* a, GameObject* b) {
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
                            ImGui::DragFloat2("Pos", &obj->transform.position.x, 1.0f);
                            float rot = obj->transform.rotation * RAD2DEG;
                            if (ImGui::DragFloat("Rot", &rot, 1.0f)) obj->transform.rotation = rot * DEG2RAD;
                            obj->transform.isDirty = true;
                        }

                        if (obj->rb && ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
                            float m = obj->rb->GetMass();
                            if (ImGui::DragFloat("Mass", &m, 0.1f)) obj->rb->SetMass(m);
                            float f = obj->rb->GetFriction();
                            if (ImGui::SliderFloat("Friction", &f, 0.0f, 1.0f)) obj->rb->SetFriction(f);
                            Vec2 v = obj->rb->GetVelocity();
                            if (ImGui::DragFloat2("Velocity", &v.x, 1.0f)) obj->rb->SetVelocity(v);
                            
                            bool useGrav = obj->rb->IsGravityEnabled();
                            if (ImGui::Checkbox("Gravity", &useGrav)) obj->rb->SetGravity(useGrav);
                        }

                        if (Renderer* ren = obj->GetComponent<Renderer>()) {
                            if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
                                Shape s = ren->GetShape();
                                float color[4] = { s.color.r/255.f, s.color.g/255.f, s.color.b/255.f, s.color.a/255.f };
                                if (ImGui::ColorEdit4("Color", color)) {
                                    ren->SetColor({(unsigned char)(color[0]*255), (unsigned char)(color[1]*255), (unsigned char)(color[2]*255), (unsigned char)(color[3]*255)});
                                }
                            }
                        }
                    } else {
                        ImGui::Text("Select an object to inspect.");
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();

            rlImGuiEnd();
            
            if (FPS) DrawFPS(sidebarWidth + 10, 10);
        
        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}
