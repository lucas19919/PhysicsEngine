#pragma once
#include "main/World.h"
#include <string>
#include "main/scenes/LoadScene.h"
#include "main/components/Controller.h"
#include "raylib.h"

class InputHandler
{
    public:
        void Update(World& world, const std::string& filePath, int viewportWidth, int viewportHeight, float dt, Camera2D camera);
};
