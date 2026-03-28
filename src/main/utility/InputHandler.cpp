#include "main/utility/InputHandler.h"
#include "raylib.h"

void InputHandler::Update(World& world)
{
    if (IsKeyPressed(KEY_SPACE))
    {
        world.isPaused = world.isPaused ? false : true;
    }
}