#include "raylib.h"
#include "physics/world.h"
#include "physics/RigidBody.h"
#include "math/Vec2.h"
#include <vector>

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 960;

    InitWindow(screenWidth, screenHeight, "Engine 1.0");
    SetTargetFPS(60);

    World world;
    int ballCount = 100;
    for (int i = 0; i < ballCount; i++) {
        float posX = 100 + (i * 50) % (screenWidth - 200);
        float posY = 100 + (i * 40) % (screenHeight / 2);

        float velX = (float)GetRandomValue(-200, 200);
        float velY = (float)GetRandomValue(-200, 200);

        float mass = (i % 5 == 0) ? 10.0f : 1.0f;
        float radius = (mass > 5.0f) ? 30.0f : 15.0f;

        RigidBody* b = new RigidBody(mass, Vec2(posX, posY), Vec2(velX, velY), Vec2(), Vec2(), radius, 0.95f);
        
        world.AddBody(b);
    }

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        world.Step(dt);
        world.CheckCollisons(screenWidth, screenHeight);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            for (RigidBody* rb : world.bodies)
            {
                DrawCircle(rb->position.x, rb->position.y, rb->GetRadius(), DARKBLUE);
            }
        EndDrawing();

        DrawFPS(10, 10);
    }

    CloseWindow();
    return 0;
}