#pragma once
#include <string>

#include "main/World.h"
#include "main/components/Controller.h"
#include "main/editor/EditorCamera.h"
#include "main/scenes/LoadScene.h"

class InputHandler
{
    public:
        void Update(World& world, EditorCamera& camera, const std::string& filePath, int screenWidth, int screenHeight, float dt);
        
        Vec2 GetMouseWorldPos() const { return physicsMousePos; }

    private:
        Vec2 physicsMousePos;
};
