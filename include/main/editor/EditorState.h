#pragma once
#include "raylib.h"
#include "main/GameObject.h"
#include "main/components/Constraint.h"
#include "math/Vec2.h"
#include <external/nlohmann/json.hpp>

enum class GizmoType { NONE, TRANSLATE, ROTATE, SCALE };
enum class GizmoAxis { NONE, X, Y, BOTH };

class EditorState {
public:
    static EditorState& Get() {
        static EditorState instance;
        return instance;
    }

    void SetSelected(GameObject* obj) { 
        selectedObject = obj; 
        if (obj) selectedGroup = ""; 
    }
    GameObject* GetSelected() const { return selectedObject; }

    void SetSelectedGroup(const std::string& name) { 
        selectedGroup = name; 
        if (!name.empty()) selectedObject = nullptr; 
    }
    const std::string& GetSelectedGroup() const { return selectedGroup; }

    void ClearSelection() {
        selectedObject = nullptr;
        selectedGroup = "";
    }

    void SetGizmoType(GizmoType type) { activeGizmoType = type; }
    GizmoType GetGizmoType() const { return activeGizmoType; }

    void SetHoveredAxis(GizmoAxis axis) { hoveredAxis = axis; }
    GizmoAxis GetHoveredAxis() const { return hoveredAxis; }

    void SetActiveAxis(GizmoAxis axis) { activeAxis = axis; }
    GizmoAxis GetActiveAxis() const { return activeAxis; }

    bool IsGizmoHovered() const { return hoveredAxis != GizmoAxis::NONE; }
    bool IsGizmoActive() const { return activeAxis != GizmoAxis::NONE; }

    void SetActiveScenePath(const std::string& path) { currentScenePath = path; }
    const std::string& GetActiveScenePath() const { return currentScenePath; }

    void SetViewportMousePos(Vec2 pos) { viewportMousePos = pos; }
    Vec2 GetViewportMousePos() const { return viewportMousePos; }

    void SetViewportSize(Vec2 size) { viewportSize = size; }
    Vec2 GetViewportSize() const { return viewportSize; }

    void SetViewportHovered(bool hovered) { isViewportHovered = hovered; }
    bool IsViewportHovered() const { return isViewportHovered; }

    void SetViewportFocused(bool focused) { isViewportFocused = focused; }
    bool IsViewportFocused() const { return isViewportFocused; }

    // Picking Mode
    enum class PickingMode { NONE, CONSTRAINT_TARGET };
    void SetPickingMode(PickingMode mode, ConstraintType type) { pickingMode = mode; pendingConstraintType = type; }
    PickingMode GetPickingMode() const { return pickingMode; }
    ConstraintType GetPendingConstraintType() const { return pendingConstraintType; }
    void ClearPickingMode() { pickingMode = PickingMode::NONE; }

    // Anchor Manipulation
    struct SelectedAnchor {
        Constraint* constraint = nullptr;
        int index = -1; // -1 for main, 0 or 1 for offsets
        bool isHovered = false;
    };
    void SetHoveredAnchor(const SelectedAnchor& anchor) { hoveredAnchor = anchor; }
    const SelectedAnchor& GetHoveredAnchor() const { return hoveredAnchor; }
    void SetActiveAnchor(const SelectedAnchor& anchor) { activeAnchor = anchor; }
    const SelectedAnchor& GetActiveAnchor() const { return activeAnchor; }

    void CaptureInitialState(const nlohmann::json& state) { initialEditorState = state; }
    const nlohmann::json& GetInitialState() const { return initialEditorState; }
    bool HasInitialState() const { return !initialEditorState.is_null(); }
    void ClearInitialState() { initialEditorState = nlohmann::json(); }

    struct ThemeColors {
        Color viewportBg;
        Color gridColor;
        Color borderColor;
        Color selectionColor;
    };
    void SetThemeColors(ThemeColors colors) { themeColors = colors; }
    const ThemeColors& GetThemeColors() const { return themeColors; }

    private:
    EditorState() : selectedObject(nullptr), selectedGroup(""), activeGizmoType(GizmoType::TRANSLATE), hoveredAxis(GizmoAxis::NONE), activeAxis(GizmoAxis::NONE), currentScenePath("../assets/examples/PrattTruss.json"), viewportMousePos{0,0}, viewportSize{0,0}, isViewportHovered(false), isViewportFocused(false), pickingMode(PickingMode::NONE) {
        themeColors = { WHITE, {200, 200, 200, 100}, DARKGRAY, ORANGE };
        hoveredAnchor = {nullptr, -1, false};
        activeAnchor = {nullptr, -1, false};
    }
    GameObject* selectedObject;
    std::string selectedGroup;
    GizmoType activeGizmoType;
    GizmoAxis hoveredAxis;
    GizmoAxis activeAxis;
    std::string currentScenePath;

    Vec2 viewportMousePos;
    Vec2 viewportSize;
    bool isViewportHovered;
    bool isViewportFocused;

    PickingMode pickingMode;
    ConstraintType pendingConstraintType;

    SelectedAnchor hoveredAnchor;
    SelectedAnchor activeAnchor;

    nlohmann::json initialEditorState;
    ThemeColors themeColors;
    };
