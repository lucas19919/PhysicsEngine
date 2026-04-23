#include "main/utility/InputHandler.h"
#include "main/components/Controller.h"
#include "main/physics/Config.h"
#include "raylib.h"
#include "external/imgui/imgui.h"
#include "main/components/Collider.h"

void InputHandler::Update(World& world, const std::string& filePath, int viewportWidth, int viewportHeight, float dt, Camera2D camera)
{
    if (IsKeyPressed(KEY_SPACE))
    {
        world.isPaused = !world.isPaused;
    }
    if (IsKeyPressed(KEY_R))
    {
        world.isPaused = true;
        world.Clear();
        LoadScene::Load(filePath, world, viewportWidth, viewportHeight);
    }

    // Selection Logic
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !ImGui::GetIO().WantCaptureMouse)
    {
        Vector2 mousePos = GetMousePosition();
        Vector2 worldMouse = GetScreenToWorld2D(mousePos, camera);
        Vec2 point(worldMouse.x, worldMouse.y); 
        
        world.selectedObject = nullptr;
        for (auto& objPtr : world.GetGameObjects())
        {
            if (objPtr->c)
            {
                // Force update cache for accurate point testing even when paused
                objPtr->c->UpdateCache(objPtr->transform);
                if (objPtr->c->IsPointInside(point))
                {
                    world.selectedObject = objPtr.get();
                    break;
                }
            }
        }
    }

    for (const auto& c : world.GetControllers())
    {
        if (c->active)
            c->Update(dt); 
    }
}
