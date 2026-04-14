#include "raylib.h"
#include "external/imgui/imgui.h"
#include "external/imgui/rlImGui.h"
#include "main/World.h"
#include "main/GameObject.h"
#include "main/scenes/LoadScene.h"
#include "main/utility/Draw.h"
#include "main/utility/InputHandler.h"
#include "main/physics/Config.h"
#include <string>

int main() {
    const int screenWidth = Config::screenWidth;
    const int screenHeight = Config::screenHeight;

    InitWindow(screenWidth, screenHeight, "Halliday2D");
    SetTargetFPS(Config::targetFPS);    

    rlImGuiSetup(true);

    World world;
    InputHandler input;

    std::string selectedFile = "../assets/examples/Car.json";
    char filePathBuffer[256] = "";

    LoadScene::Load(selectedFile, world, screenWidth, screenHeight);

    //draw fps?
    bool FPS = Config::drawFPS;

    const float dt = 1.0f / 60.0f;
    while (!WindowShouldClose()) {
        input.Update(world, selectedFile, screenWidth, screenHeight, dt);
        world.Step(dt);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            Render(world); 
            
            rlImGuiBegin();
            ImGui::Begin("Physics Tools");
            
            ImGui::InputText("Level Path", filePathBuffer, 256);
            
            if (ImGui::Button("Load Level")) {
                selectedFile = filePathBuffer;
                world.isPaused = true;
                world.Clear();
                
                LoadScene::Load(selectedFile, world, screenWidth, screenHeight);  
          }

            if (!selectedFile.empty()) {
                ImGui::Text("Currently active file: %s", selectedFile.c_str());
            }

            ImGui::End();

            rlImGuiEnd();
            
            if (FPS) {
                DrawFPS(10, 10);
            }
        
        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}