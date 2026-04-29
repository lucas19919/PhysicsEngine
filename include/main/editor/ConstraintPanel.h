#pragma once
#include "main/editor/Panel.h"
#include <string>

namespace Editor {

class ConstraintPanel : public Panel {
public:
    void OnImGui(World& world) override;
    const char* GetName() const override { return "Constraints"; }
};

} // namespace Editor
