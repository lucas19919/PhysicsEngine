#pragma once
#include "main/World.h"
#include <string>
#include "main/scenes/LoadScene.h"
#include "main/components/Controller.h"
#include "main/utility/EditorCamera.h"

class InputHandler
{
    public:
        void Update(World& world, EditorCamera& camera, const std::string& filePath, int screenWidth, int screenHeight, float dt);
        
        Vec2 GetMouseWorldPos() const { return physicsMousePos; }

    private:
        Vec2 physicsMousePos;
};
