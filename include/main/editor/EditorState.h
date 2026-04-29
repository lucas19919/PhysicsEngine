#pragma once
#include "raylib.h"
#include <external/nlohmann/json.hpp>
#include <vector>
#include <algorithm>

#include "main/GameObject.h"
#include "main/components/Constraint.h"
#include "math/Vec2.h"

class World;

enum class SelectionType { NONE, OBJECT, GROUP, CONSTRAINT };
enum class GizmoType { NONE, TRANSLATE, ROTATE, SCALE };
enum class GizmoAxis { NONE, X, Y, BOTH };

class EditorState {
public:
    static EditorState& Get() {
        static EditorState instance;
        return instance;
    }

    SelectionType GetSelectionType() const { return selectionType; }

    void SetSelectedObject(size_t id) { 
        ClearSelection();
        if (id != (size_t)-1) {
            selectionType = SelectionType::OBJECT;
            selectedObjectIDs.push_back(id);
        }
    }

    void AddSelectedObject(size_t id) {
        if (id == (size_t)-1) return;
        if (selectionType != SelectionType::OBJECT) {
            ClearSelection();
            selectionType = SelectionType::OBJECT;
        }
        if (std::find(selectedObjectIDs.begin(), selectedObjectIDs.end(), id) == selectedObjectIDs.end()) {
            selectedObjectIDs.push_back(id);
        }
    }

    void RemoveSelectedObject(size_t id) {
        if (selectionType != SelectionType::OBJECT) return;
        selectedObjectIDs.erase(std::remove(selectedObjectIDs.begin(), selectedObjectIDs.end(), id), selectedObjectIDs.end());
        if (selectedObjectIDs.empty()) {
            ClearSelection();
        }
        if (activeAnchor.constraintID != (size_t)-1 && activeAnchor.isHovered) {
             // Safe to keep clearing this just in case interaction state is tied to it.
        }
    }

    bool IsSelected(size_t id) const {
        if (selectionType != SelectionType::OBJECT) return false;
        return std::find(selectedObjectIDs.begin(), selectedObjectIDs.end(), id) != selectedObjectIDs.end();
    }

    size_t GetSelectedID() const { 
        return (selectionType == SelectionType::OBJECT && !selectedObjectIDs.empty()) ? selectedObjectIDs[0] : (size_t)-1; 
    }
    
    const std::vector<size_t>& GetSelectedObjectIDs() const { return selectedObjectIDs; }

    // Helper to get raw pointer safely from world
    GameObject* GetSelected(class World& world) const;

    void SetSelectedConstraint(size_t id) {
        ClearSelection();
        if (id != (size_t)-1) { 
            selectionType = SelectionType::CONSTRAINT;
            selectedConstraintID = id;
        }
    }
    size_t GetSelectedConstraintID() const { return selectedConstraintID; }

    Constraint* GetSelectedConstraint(class World& world) const;

    void SetSelectedGroup(const std::string& name) { 
        ClearSelection();
        if (!name.empty()) { 
            selectionType = SelectionType::GROUP;
            selectedGroupName = name;
        }
    }
    const std::string& GetSelectedGroup() const { return selectedGroupName; }

    void ClearSelection() {
        selectionType = SelectionType::NONE;
        selectedObjectIDs.clear();
        selectedGroupName = "";
        selectedConstraintID = (size_t)-1;
        activeAxis = GizmoAxis::NONE;
        hoveredAxis = GizmoAxis::NONE;
        activeAnchor = { (size_t)-1, -1, false };
        hoveredAnchor = { (size_t)-1, -1, false };
        boxSelection = {{0,0}, {0,0}, false};
    }

    void SetGizmoType(GizmoType type) { activeGizmoType = type; }
    GizmoType GetGizmoType() const { return activeGizmoType; }

    void SetHoveredAxis(GizmoAxis axis) { hoveredAxis = axis; }
    GizmoAxis GetHoveredAxis() const { return hoveredAxis; }

    void SetActiveAxis(GizmoAxis axis) { activeAxis = axis; }
    GizmoAxis GetActiveAxis() const { return activeAxis; }

    bool IsGizmoHovered() const { return hoveredAxis != GizmoAxis::NONE || hoveredAnchor.isHovered; }
    bool IsGizmoActive() const { return activeAxis != GizmoAxis::NONE || activeAnchor.constraintID != (size_t)-1; }

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
    void SetPickingMode(PickingMode mode, ConstraintType type, size_t existingID = (size_t)-1) { 
        pickingMode = mode; 
        pendingConstraintType = type; 
        targetConstraintID = existingID;
    }
    PickingMode GetPickingMode() const { return pickingMode; }
    ConstraintType GetPendingConstraintType() const { return pendingConstraintType; }
    size_t GetTargetConstraintID() const { return targetConstraintID; }
    void ClearPickingMode() { pickingMode = PickingMode::NONE; targetConstraintID = (size_t)-1; }

    // Box Selection
    struct BoxSelection {
        Vec2 start;
        Vec2 end;
        bool active = false;
    };
    void SetBoxSelection(const BoxSelection& box) { boxSelection = box; }
    const BoxSelection& GetBoxSelection() const { return boxSelection; }

    // Anchor Manipulation
    struct SelectedAnchor {
        size_t constraintID = (size_t)-1;
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
    EditorState() : selectionType(SelectionType::NONE), selectedGroupName(""), selectedConstraintID((size_t)-1), activeGizmoType(GizmoType::TRANSLATE), hoveredAxis(GizmoAxis::NONE), activeAxis(GizmoAxis::NONE), currentScenePath("../assets/examples/PrattTruss.json"), viewportMousePos{0,0}, viewportSize{0,0}, isViewportHovered(false), isViewportFocused(false), pickingMode(PickingMode::NONE) {
        themeColors = { WHITE, {200, 200, 200, 100}, DARKGRAY, ORANGE };
        hoveredAnchor = {(size_t)-1, -1, false};
        activeAnchor = {(size_t)-1, -1, false};
        boxSelection = {{0,0}, {0,0}, false};
        targetConstraintID = (size_t)-1;
    }
    SelectionType selectionType;
    std::vector<size_t> selectedObjectIDs;
    std::string selectedGroupName;
    size_t selectedConstraintID;
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
    size_t targetConstraintID;

    BoxSelection boxSelection;
    SelectedAnchor hoveredAnchor;
    SelectedAnchor activeAnchor;

    nlohmann::json initialEditorState;
    ThemeColors themeColors;
};
