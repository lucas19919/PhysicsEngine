#pragma once
#include "main/editor/Panel.h"

namespace Editor {

class InspectorPanel : public Panel {
public:
    void OnImGui(World& world) override;
    const char* GetName() const override { return "Inspector"; }
};

} // namespace Editor
