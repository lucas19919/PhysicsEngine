#include "main/utility/InputHandler.h"
#include "main/components/Controller.h"
#include "main/physics/Config.h"
#include "raylib.h"
#include "external/imgui/imgui.h"
#include "main/components/Collider.h"

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

    // Selection Logic
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !ImGui::GetIO().WantCaptureMouse)
    {
        Vector2 mousePos = GetMousePosition();
        // Subtract 300px sidebar offset to get world-space coordinates
        Vec2 point(mousePos.x - 300.0f, mousePos.y); 
        
        world.selectedObject = nullptr;
        for (auto& objPtr : world.GetGameObjects())
        {
            if (objPtr->c && objPtr->c->IsPointInside(point))
            {
                world.selectedObject = objPtr.get();
                break;
            }
        }
    }

    for (const auto& c : world.GetControllers())
    {
        if (c->active)
            c->Update(dt); 
    }
}
