#include "main/utility/InputHandler.h"
#include "main/components/Controller.h"
#include "main/physics/Config.h"
#include "main/utility/EditorState.h"
#include "raylib.h"
#include "external/imgui/imgui.h"

void InputHandler::Update(World& world, EditorCamera& camera, const std::string& filePath, int screenWidth, int screenHeight, float dt)
{
    // Ignore input if ImGui is capturing
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse) return;

    // Update mouse position in physics world
    physicsMousePos = camera.ScreenToWorldMeters(GetMousePosition());

    // Selection logic
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        // Don't change selection if clicking on a gizmo
        if (!EditorState::Get().IsGizmoHovered() && !EditorState::Get().IsGizmoActive())
        {
            GameObject* hit = nullptr;
            // Iterate backwards to select "top" objects first if they overlap
            const auto& objects = world.GetGameObjects();
            for (auto it = objects.rbegin(); it != objects.rend(); ++it)
            {
                GameObject* obj = it->get();
                if (obj->c && obj->c->TestPoint(physicsMousePos))
                {
                    hit = obj;
                    break;
                }
            }
            EditorState::Get().SetSelected(hit);
        }
    }

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

    if (IsKeyPressed(KEY_W))
    {
        EditorState::Get().SetGizmoType(GizmoType::TRANSLATE);
    }
    if (IsKeyPressed(KEY_E))
    {
        EditorState::Get().SetGizmoType(GizmoType::ROTATE);
    }

    for (const auto& c : world.GetControllers())
    {
        if (c->active)
            c->Update(dt); 
    }
}
