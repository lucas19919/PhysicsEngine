#pragma once
#include "raylib.h"

#include "main/editor/EditorCamera.h"
#include "main/editor/Panel.h"

namespace Editor {

class ViewportPanel : public Panel {
public:
    ViewportPanel(EditorCamera& camera);
    ~ViewportPanel();
    void OnImGui(World& world) override;
    const char* GetName() const override { return "Viewport"; }

private:
    EditorCamera& camera;
    RenderTexture2D target;
    
    void RefreshRenderTexture(float width, float height);
};

} // namespace Editor
