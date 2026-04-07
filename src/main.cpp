#include "raylib.h"
#include "main/World.h"
#include "main/GameObject.h"
#include "main/scenes/LoadScene.h"
#include "main/utility/Draw.h"
#include "main/utility/InputHandler.h"
#include "main/physics/Config.h"
#include <string>

int main() {
    const int screenWidth = Config().screenWidth;
    const int screenHeight = Config().screenHeight;

    World world;
    InputHandler input;

    //all levels under ../assets/( ... ).json
    const std::string& filepath = "../assets/demos/DoublePendulum.json";
    LoadScene::Load(filepath, world, screenWidth, screenHeight);
    
    InitWindow(screenWidth, screenHeight, "Engine 1.0");
    SetTargetFPS(Config().targetFPS);    

    //draw fps?
    bool FPS = Config().drawFPS;

    int uiTimer = 0;
    int uiToggleDelay = 60;
    int sleepingObjects = world.sleepCounter;

    const float dt = 1.0f / 60.0f;
    while (!WindowShouldClose()) {
        input.Update(world, filepath, screenWidth, screenHeight);
        world.Step(dt);

        BeginDrawing();
            ClearBackground(RAYWHITE);
            Render(world);
            
            uiTimer++;
            if (uiTimer >= uiToggleDelay) {
                sleepingObjects = world.sleepCounter;
                uiTimer = 0;
            }

            if (Config().debugSleep)
                DrawText(std::to_string(sleepingObjects).c_str(), 10, 50, 30, BLACK);

            if (FPS)
                DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}