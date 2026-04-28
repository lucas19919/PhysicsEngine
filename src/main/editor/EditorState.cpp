#include "main/editor/EditorState.h"
#include "main/World.h"

GameObject* EditorState::GetSelected(class World& world) const {
    if (selectionType != SelectionType::OBJECT || selectedObjectIDs.empty()) return nullptr;
    for (auto& obj : world.GetGameObjects()) if (obj->GetID() == selectedObjectIDs[0]) return obj.get();
    return nullptr;
}

Constraint* EditorState::GetSelectedConstraint(class World& world) const {
    if (selectionType != SelectionType::CONSTRAINT || selectedConstraintID == (size_t)-1) return nullptr;
    for (auto& c : world.GetConstraints()) if (c->GetID() == selectedConstraintID) return c.get();
    return nullptr;
}
