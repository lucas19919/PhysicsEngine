#include <string>

#include "external/imgui/imgui.h"
#include "external/imgui/rlImGui.h"
#include "raylib.h"

#include "main/World.h"
#include "main/editor/Editor.h"
#include "main/editor/EditorCamera.h"
#include "main/editor/EditorState.h"
#include "main/physics/Config.h"
#include "main/scenes/LoadScene.h"
#include "main/utility/InputHandler.h"

int main() {
    const int screenWidth = Config::screenWidth;
    const int screenHeight = Config::screenHeight;

    InitWindow(screenWidth, screenHeight, "Halliday2D");
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    SetTargetFPS(Config::targetFPS);    

    Image icon = LoadImage("icon.ico"); 
    SetWindowIcon(icon);
    UnloadImage(icon);

    rlImGuiSetup(true);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    World world;
    InputHandler input;
    EditorCamera camera((float)screenWidth, (float)screenHeight);
    Editor::Editor editor(world, camera, input);

    LoadScene::Load(EditorState::Get().GetActiveScenePath(), world, screenWidth, screenHeight);

    const float dt = 1.0f / 60.0f;
    while (!WindowShouldClose()) {
        int currentScreenWidth = GetScreenWidth();
        int currentScreenHeight = GetScreenHeight();
        
        input.Update(world, camera, EditorState::Get().GetActiveScenePath(), currentScreenWidth, currentScreenHeight, dt);
        
        if (world.isPaused) world.UpdateCaches();
        world.Step(dt);

        BeginDrawing();
            ClearBackground(EditorState::Get().GetThemeColors().viewportBg);

            editor.Update(world);
        
        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}