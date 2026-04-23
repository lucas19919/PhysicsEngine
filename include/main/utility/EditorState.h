#pragma once
#include "main/GameObject.h"
#include "math/Vec2.h"

enum class GizmoType { NONE, TRANSLATE, ROTATE, SCALE };
enum class GizmoAxis { NONE, X, Y, BOTH };

class EditorState {
public:
    static EditorState& Get() {
        static EditorState instance;
        return instance;
    }

    void SetSelected(GameObject* obj) { selectedObject = obj; }
    GameObject* GetSelected() const { return selectedObject; }

    void SetGizmoType(GizmoType type) { activeGizmoType = type; }
    GizmoType GetGizmoType() const { return activeGizmoType; }

    void SetHoveredAxis(GizmoAxis axis) { hoveredAxis = axis; }
    GizmoAxis GetHoveredAxis() const { return hoveredAxis; }

    void SetActiveAxis(GizmoAxis axis) { activeAxis = axis; }
    GizmoAxis GetActiveAxis() const { return activeAxis; }

    bool IsGizmoHovered() const { return hoveredAxis != GizmoAxis::NONE; }
    bool IsGizmoActive() const { return activeAxis != GizmoAxis::NONE; }

private:
    EditorState() : selectedObject(nullptr), activeGizmoType(GizmoType::TRANSLATE), hoveredAxis(GizmoAxis::NONE), activeAxis(GizmoAxis::NONE) {}
    GameObject* selectedObject;
    GizmoType activeGizmoType;
    GizmoAxis hoveredAxis;
    GizmoAxis activeAxis;
};
