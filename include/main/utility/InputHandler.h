#pragma once
#include "main/World.h"
#include <string>
#include "main/scenes/LoadScene.h"

class InputHandler
{
    public:
        void Update(World& world, const std::string& filePath, int screenWidth, int screenHeight);
};
