#include "raylib.h"
#include "main/World.h"
#include "main/GameObject.h"
#include "main/scenes/LoadScene.h"
#include "main/utility/Draw.h"
#include "main/utility/InputHandler.h"
#include <string>

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 960;

    World world;
    InputHandler input;

    //all levels under ../assets/( ... ).json
    const std::string& filepath = "../assets/gentest.json";
    LoadScene::Load(filepath, world, screenWidth, screenHeight);
    
    InitWindow(screenWidth, screenHeight, "Engine 1.0");
    SetTargetFPS(60);    

    //draw fps?
    bool FPS = true;

    while (!WindowShouldClose()) {
        input.Update(world, filepath, screenWidth, screenHeight);

        float dt = GetFrameTime();
        int subTicks = 8; 
        for (int i = 0; i < subTicks; i++)
        {
            world.Step(dt / subTicks);
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            for (const auto& objPtr : world.GetGameObjects())
            {
                Render(objPtr.get());
            }

            if (FPS)
                DrawFPS(10, 10);    
        EndDrawing();
    }

    CloseWindow();
    return 0;
}