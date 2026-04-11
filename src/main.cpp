#include "raylib.h"
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

    World world;
    InputHandler input;

    //all levels under ../assets/( ... ).json
    const std::string& filepath = "../assets/demos/Joints.json";
    LoadScene::Load(filepath, world, screenWidth, screenHeight);
    
    InitWindow(screenWidth, screenHeight, "Halliday2D");
    SetTargetFPS(Config::targetFPS);    

    //draw fps?
    bool FPS = Config::drawFPS;

    const float dt = 1.0f / 60.0f;
    while (!WindowShouldClose()) {
        input.Update(world, filepath, screenWidth, screenHeight);
        world.Step(dt);

        BeginDrawing();
            ClearBackground(RAYWHITE);
            Render(world);
            
            if (FPS)
                DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}