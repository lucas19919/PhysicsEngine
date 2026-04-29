#pragma once

#include "main/World.h"
#include "main/editor/EditorCamera.h"
#include "main/editor/EditorState.h"
#include "math/Vec2.h"

namespace Editor {

class Gizmo {
public:
    Gizmo();

    // Returns true if the gizmo handled the interaction
    bool Update(World& world, const EditorCamera& camera);
    void Render(World& world, const EditorCamera& camera);

private:
    GizmoAxis HitTest(const EditorCamera& camera, Vec2 gizmoWorldPos, GizmoType type);
    
    // Internal state for dragging
    Vec2 initialMouseWorldPos;
    bool isDragging = false;
};

} // namespace Editor
