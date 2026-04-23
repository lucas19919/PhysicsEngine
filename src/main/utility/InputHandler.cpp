#include "main/utility/InputHandler.h"
#include "main/components/Controller.h"
#include "main/physics/Config.h"
#include "raylib.h"

void InputHandler::Update(World& world, const std::string& filePath, int screenWidth, int screenHeight, float dt)
{
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
