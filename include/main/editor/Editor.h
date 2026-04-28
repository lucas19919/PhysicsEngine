#pragma once
#include <memory>
#include <vector>

#include "main/World.h"
#include "main/editor/EditorCamera.h"
#include "main/editor/Panel.h"
#include "main/utility/InputHandler.h"

namespace Editor {

class Editor {
public:
    Editor(World& world, EditorCamera& camera, InputHandler& input);
    ~Editor();

    void Update(World& world);

private:
    std::vector<std::unique_ptr<Panel>> panels;
};

} // namespace Editor
