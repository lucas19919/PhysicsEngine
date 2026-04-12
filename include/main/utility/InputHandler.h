#pragma once
#include "main/World.h"
#include <string>
#include "main/scenes/LoadScene.h"
#include "main/components/Controller.h"

class InputHandler
{
    public:
        void Update(World& world, const std::string& filePath, int screenWidth, int screenHeight, float dt);
};
