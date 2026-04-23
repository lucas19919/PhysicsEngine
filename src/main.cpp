#include "raylib.h"
#include "external/imgui/rlImGui.h"
#include "main/World.h"
#include "main/GameObject.h"
#include "main/scenes/LoadScene.h"
#include "main/utility/Draw.h"
#include "main/utility/InputHandler.h"
#include "main/physics/Config.h"
#include "main/UI/EditorUI.h"
#include <string>

int main() {
    const int sidebarWidth = 300;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE); 
    InitWindow(Config::screenWidth + (sidebarWidth * 2), Config::screenHeight, "Halliday2D - Editor");
    SetTargetFPS(Config::targetFPS);    

    rlImGuiSetup(true);

    World world;
    InputHandler input;
    EditorUI editorUI;

    Camera2D camera = { 0 };
    camera.offset = { (float)sidebarWidth, 0 };
    camera.target = { 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    std::string selectedFile = "../assets/examples/PrattTruss.json";

    LoadScene::Load(selectedFile, world, Config::screenWidth, Config::screenHeight);

    const float dt = 1.0f / 60.0f;
    while (!WindowShouldClose()) {
        const int currentWidth = GetScreenWidth();
        const int currentHeight = GetScreenHeight();
        const int viewportWidth = currentWidth - (sidebarWidth * 2);
        const int viewportHeight = currentHeight;

        Config::screenWidth = viewportWidth;
        Config::screenHeight = viewportHeight;

        input.Update(world, selectedFile, viewportWidth, viewportHeight, dt, camera);
        world.Step(dt);

        BeginDrawing();
            ClearBackground({ 30, 30, 30, 255 }); // Dark editor background

            // background for viewport
            DrawRectangle(sidebarWidth, 0, viewportWidth, viewportHeight, RAYWHITE);
            
            BeginMode2D(camera);
                Render(world); 
            EndMode2D();

            editorUI.Draw(world, selectedFile, viewportWidth, viewportHeight, currentWidth, currentHeight);
        
        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}