#include "raylib.h"
#include "external/imgui/imgui.h"
#include "external/imgui/rlImGui.h"
#include "main/World.h"
#include "main/GameObject.h"
#include "main/scenes/LoadScene.h"
#include "main/utility/Draw.h"
#include "main/utility/InputHandler.h"
#include "main/physics/Config.h"
#include "main/editor/EditorCamera.h"
#include "main/editor/EditorState.h"
#include "main/editor/Editor.h"
#include <string>

int main() {
    const int screenWidth = Config::screenWidth;
    const int screenHeight = Config::screenHeight;

    InitWindow(screenWidth, screenHeight, "Halliday2D");
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    SetTargetFPS(Config::targetFPS);    

    rlImGuiSetup(true);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    World world;
    InputHandler input;
    EditorCamera camera((float)screenWidth, (float)screenHeight);
    Editor::Editor editor(world, camera, input);

    LoadScene::Load(EditorState::Get().GetActiveScenePath(), world, screenWidth, screenHeight);

    //draw fps?
    bool FPS = Config::drawFPS;

    const float dt = 1.0f / 60.0f;
    while (!WindowShouldClose()) {
        input.Update(world, camera, EditorState::Get().GetActiveScenePath(), screenWidth, screenHeight, dt);
        
        world.Step(dt);

        BeginDrawing();
            ClearBackground(DARKGRAY);

            editor.Update(world);
        
        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}