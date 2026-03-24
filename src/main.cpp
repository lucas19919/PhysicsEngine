#include "raylib.h"
#include "main/World.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include <vector>
#include "main/utility/Draw.h"
#include "main/utility/Instantiate.h"

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 960;

    World world;

    bool wallCollisions = true;
    if (wallCollisions)
    {
        GameObject* floor = Instantiate()
            .WithTransform(Vec2(screenWidth / 2.0f, screenHeight + 25.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(screenWidth, 50.0f))
            .Create(world);

        GameObject* ceiling = Instantiate()
            .WithTransform(Vec2(screenWidth / 2.0f, -25.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(screenWidth, 50.0f))
            .Create(world);

        GameObject* leftWall = Instantiate()
            .WithTransform(Vec2(-25.0f, screenHeight / 2.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(50.0f, screenHeight))
            .Create(world);

        GameObject* rightWall = Instantiate()
            .WithTransform(Vec2(screenWidth + 25.0f, screenHeight / 2.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(50.0f, screenHeight))
            .Create(world);
    }

    Properties properties = { 1.0f, 0.5f, 100.0f };//, 1.0f };
    LinearState linearState = { Vec2(100.0f, 0.0f), Vec2(), Vec2() };
    AngularState angularState = { 1.0f, 0.0f, 0.0f };
    
    GameObject* box = Instantiate()
        .WithTransform(Vec2(screenWidth / 2, 50.0f), 0.0f)
        .WithCollider(ColliderType::BOX, Vec2(30.0f, 50.0f))
        .WithRenderer({ RenderShape::R_BOX, BLUE, Vec2(30.0f, 50.0f) })
        .WithRigidBody(properties, linearState, angularState)
        .Create(world);

    GameObject* circle = Instantiate()
        .WithTransform(Vec2(400.0f, 50.0f), 0.0f)
        .WithCollider(ColliderType::CIRCLE, 25.0f)
        .WithRenderer({ RenderShape::R_CIRCLE, RED, 25.0f})
        .WithRigidBody(properties, linearState, angularState)
        .Create(world);

    std::vector<Vec2> polyVerts = { 
        Vec2(-20.0f, -20.0f), // Top Left
        Vec2(-20.0f,  20.0f), // Bottom Left
        Vec2( 20.0f,  20.0f), // Bottom Right
        Vec2( 30.0f,  0.0f),  // Pointy Right Edge
        Vec2( 20.0f, -20.0f)  // Top Right
    };

    GameObject* poly = Instantiate()
        .WithTransform(Vec2(200.0f, 100.0f), 0.0f)
        .WithCollider(ColliderType::POLYGON, polyVerts)
        .WithRenderer({ RenderShape::R_POLYGON, GREEN, polyVerts })
        .WithRigidBody(properties, linearState, angularState)
        .Create(world);

        std::vector<Vec2> polyVerts2 = { 
            Vec2(0.0f, 0.0f),  // Top Left     
            Vec2(0.0f,  50.0f),  // Bottom Left   
            Vec2( 30.0f,  10.0f),  // Bottom Right
            Vec2( 00.0f, 10.0f),  // Top Right
        };


    GameObject* poly2 = Instantiate()
        .WithTransform(Vec2(300.0f, 150.0f), 0.0f)
        .WithCollider(ColliderType::POLYGON, polyVerts2)
        .WithRenderer({ RenderShape::R_POLYGON, PURPLE, polyVerts2 })
        .WithRigidBody(properties, linearState, angularState)
        .Create(world);

    InitWindow(screenWidth, screenHeight, "Engine 1.0");
    SetTargetFPS(60);    

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        world.Step(dt);
        world.CheckCollisons(screenWidth, screenHeight);

        BeginDrawing();
            ClearBackground(RAYWHITE);
            Render(box);
            Render(circle);
            Render(poly);
            Render(poly2);
        EndDrawing();

        DrawFPS(10, 10);
    }

    CloseWindow();
    return 0;
}