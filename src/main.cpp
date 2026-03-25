#include "raylib.h"
#include "main/World.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/scenes/LoadScene.h"
#include <vector>
#include "main/utility/Draw.h"
#include "main/utility/Instantiate.h"

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 960;

    World world;
    LoadScene::Load("assets/testrotation.json", world, screenWidth, screenHeight);
    
    InitWindow(screenWidth, screenHeight, "Engine 1.0");
    SetTargetFPS(60);    

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        world.Step(dt);
        world.CheckCollisons(screenWidth, screenHeight);

        BeginDrawing();
            ClearBackground(RAYWHITE);
            for (GameObject* obj : world.GetGameObjects())
            {
                Render(obj);
            }
        EndDrawing();

        DrawFPS(10, 10);
    }

    CloseWindow();
    return 0;
}