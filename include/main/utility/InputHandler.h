#pragma once
#include <string>

#include "main/World.h"
#include "main/editor/EditorCamera.h"
#include "main/editor/Gizmo.h"

class InputHandler
{
    public:
        void Update(World& world, EditorCamera& camera, const std::string& filePath, int screenWidth, int screenHeight, float dt);
        
        Vec2 GetMouseWorldPos() const { return physicsMousePos; }
        Editor::Gizmo& GetGizmo() { return gizmo; }

    private:
        Vec2 physicsMousePos;
        Editor::Gizmo gizmo;

        void HandleCameraControls(EditorCamera& camera);
        void HandleShortcuts(World& world, int screenWidth, int screenHeight);
        void HandleAnchorInteraction(World& world, const EditorCamera& camera);
        void HandleSelection(World& world, const EditorCamera& camera);
};
