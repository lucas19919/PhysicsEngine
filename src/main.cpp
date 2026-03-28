#include "raylib.h"
#include "main/World.h"
#include "main/GameObject.h"
#include "main/scenes/LoadScene.h"
#include "main/utility/Draw.h"
#include "main/utility/InputHandler.h"

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 960;

    World world;
    InputHandler input;

    //all levels under assets/( ... ).json
    LoadScene::Load("assets/plinko.json", world, screenWidth, screenHeight);
    
    InitWindow(screenWidth, screenHeight, "Engine 1.0");
    SetTargetFPS(60);    

    //draw fps?
    bool FPS = false;

    while (!WindowShouldClose()) {
        input.Update(world);

        float dt = GetFrameTime();
        world.Step(dt);
        world.CheckCollisions();

        BeginDrawing();
            ClearBackground(RAYWHITE);
            for (GameObject* obj : world.GetGameObjects())
            {
                Render(obj);
            }
        EndDrawing();

        if (FPS)
            DrawFPS(10, 10);            
    }

    CloseWindow();
    return 0;
}