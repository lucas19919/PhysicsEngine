#include "main/utility/InputHandler.h"
#include "main/components/Controller.h"
#include "main/physics/Config.h"
#include "raylib.h"

void InputHandler::Update(World& world, EditorCamera& camera, const std::string& filePath, int screenWidth, int screenHeight, float dt)
{
    // Update mouse position in physics world
    physicsMousePos = camera.ScreenToWorldMeters(GetMousePosition());

    // Camera Controls
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
    {
        camera.Pan(GetMouseDelta());
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0)
    {
        camera.Zoom(wheel, GetMousePosition());
    }

    if (IsKeyPressed(KEY_SPACE))
    {
        world.isPaused = !world.isPaused;
    }
    if (IsKeyPressed(KEY_R))
    {
        world.isPaused = true;
        world.Clear();
        LoadScene::Load(filePath, world, screenWidth, screenHeight);
    }

    for (const auto& c : world.GetControllers())
    {
        if (c->active)
            c->Update(dt); 
    }
}
