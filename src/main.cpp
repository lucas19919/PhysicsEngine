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
    const std::string& filepath = "../assets/gentest.json";
    LoadScene::Load(filepath, world, screenWidth, screenHeight);
    
    InitWindow(screenWidth, screenHeight, "Engine 1.0");
    SetTargetFPS(Config().targetFPS);    

    //draw fps?
    bool FPS = true;

    int displaySleepCount = 0;
    float uiTimer = 0.0f;
    float uiUpdateInterval = 0.3f; 

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        input.Update(world, filepath, screenWidth, screenHeight);
        world.Step(dt);

        /*uiTimer += dt;
        if (uiTimer >= uiUpdateInterval) 
        {
            displaySleepCount = world.sleepCounter;
            uiTimer = 0.0f;
        }*/

        BeginDrawing();
            ClearBackground(RAYWHITE);
            for (const auto& objPtr : world.GetGameObjects()) 
            {
                Render(objPtr.get());
            }

            if (FPS) 
                DrawFPS(10, 10);    

            //DrawText("Sleeping Objects: ", 10, 40, 20, DARKGRAY);
            //DrawText(std::to_string(displaySleepCount).c_str(), 190, 40, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}