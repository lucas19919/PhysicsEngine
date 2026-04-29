#pragma once
#include "main/World.h"

namespace Editor {

class Panel {
public:
    virtual ~Panel() = default;
    virtual void OnImGui(World& world) = 0;
    virtual const char* GetName() const = 0;
    
    bool isOpen = true;
};

} // namespace Editor
