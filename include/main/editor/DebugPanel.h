#pragma once
#include "main/editor/Panel.h"

namespace Editor {

class DebugPanel : public Panel {
public:
    void OnImGui(World& world) override;
    const char* GetName() const override { return "Debug Settings"; }
};

} // namespace Editor
