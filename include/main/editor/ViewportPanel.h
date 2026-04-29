#pragma once
#include "raylib.h"

#include "main/editor/EditorCamera.h"
#include "main/editor/Panel.h"
#include "main/utility/InputHandler.h"

namespace Editor {

class ViewportPanel : public Panel {
public:
    ViewportPanel(EditorCamera& camera, InputHandler& input);
    ~ViewportPanel();
    void OnImGui(World& world) override;
    const char* GetName() const override { return "Viewport"; }

private:
    EditorCamera& camera;
    InputHandler& input;
    RenderTexture2D target;
    
    void RefreshRenderTexture(float width, float height);
};

} // namespace Editor
