#include "raylib.h"
#include "main/World.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include <vector>

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 960;

    InitWindow(screenWidth, screenHeight, "Engine 1.0");
    SetTargetFPS(60);

    World world;
    GameObject* obj = new GameObject();

    Collider* col = new CircleCollider(15.0f);
    obj->SetCollider(col);
    Renderer* rend = new Renderer(BLUE);
    obj->SetRenderer(rend);
    RigidBody* rb = new RigidBody(1.0f, 0.5f, Vec2(), Vec2(), Vec2());
    obj->SetRigidBody(rb);

    world.AddGameObject(obj);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        world.Step(dt);
        world.CheckCollisons(screenWidth, screenHeight);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            CircleCollider* c = static_cast<CircleCollider*>(obj->GetCollider());

            DrawCircle(obj->transform.position.x, obj->transform.position.y, c->radius, obj->GetRenderer()->color);
        EndDrawing();

        DrawFPS(10, 10);
    }

    CloseWindow();
    return 0;
}