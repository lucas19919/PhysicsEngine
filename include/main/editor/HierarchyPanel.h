#pragma once
#include "main/editor/Panel.h"

namespace Editor {

class HierarchyPanel : public Panel {
public:
    void OnImGui(World& world) override;
    const char* GetName() const override { return "Hierarchy"; }
};

} // namespace Editor
