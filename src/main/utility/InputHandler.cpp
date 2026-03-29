#include "main/utility/InputHandler.h"
#include "raylib.h"


void InputHandler::Update(World& world, const std::string& filePath, int screenWidth, int screenHeight)
{
    if (IsKeyPressed(KEY_SPACE))
    {
        world.isPaused = world.isPaused ? false : true;
    }
    if (IsKeyPressed(KEY_R))
    {
        world.Clear();
        world.isPaused = true;
        LoadScene::Load(filePath, world, screenWidth, screenHeight);
    }
}