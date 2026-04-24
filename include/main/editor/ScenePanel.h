#pragma once
#include "main/editor/Panel.h"
#include <string>

namespace Editor {

class ScenePanel : public Panel {
public:
    ScenePanel(int screenWidth, int screenHeight);
    void OnImGui(World& world) override;
    const char* GetName() const override { return "Scene Manager"; }

private:
    int screenWidth;
    int screenHeight;
    char filePathBuffer[256];
    std::string currentDirectory;
};

} // namespace Editor
