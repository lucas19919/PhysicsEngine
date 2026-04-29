#pragma once
#include "main/editor/EditorCamera.h"
#include "main/editor/Panel.h"
#include "main/utility/InputHandler.h"

namespace Editor {

class PerformancePanel : public Panel {
public:
    PerformancePanel(EditorCamera& camera, InputHandler& input);
    void OnImGui(World& world) override;
    const char* GetName() const override { return "Performance & Viewport"; }

private:
    EditorCamera& camera;
    InputHandler& input;

    float broadphaseHistory[100] = {0};
    float solverHistory[100] = {0};
    int historyOffset = 0;
};

} // namespace Editor
