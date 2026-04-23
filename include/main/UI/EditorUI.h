#pragma once
#include "main/World.h"
#include <string>

enum class SortMode { ID, Mass, Type };

class EditorUI {
public:
    EditorUI();
    void Draw(World& world, std::string& selectedFile, int viewportWidth, int viewportHeight, int currentWidth, int currentHeight);

private:
    char filePathBuffer[256] = "../assets/examples/";
    SortMode sortMode = SortMode::ID;
    bool showFPS = true;
    const int sidebarWidth = 300;

    void DrawPhysicsTools(World& world, std::string& selectedFile, int viewportWidth, int viewportHeight, int currentHeight);
    void DrawInspectorAndScene(World& world, int currentWidth, int currentHeight);
};