#pragma once
#include "main/World.h"
#include "main/World.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/utility/Instantiate.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "libraries/json/json.hpp"
#include <string>

class LoadScene
{
    public:
        static void Load(const std::string& filePath, World& world, int screenWidth, int screenHeight);
    private:
        static void LoadObject(const nlohmann::json& item, World& world);
};