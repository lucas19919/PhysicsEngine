#pragma once
#include <string>
#include <unordered_map>
#include <external/nlohmann/json.hpp>
#include "main/World.h"

class GameObject;

class LoadScene
{
    public:
        static void Load(const std::string& filePath, World& world, int screenWidth, int screenHeight);
        static void LoadFromJSON(const nlohmann::json& sceneData, World& world, int screenWidth, int screenHeight);
        static void Regenerate(World& world, GeneratorDef& def, const std::string& clearGroupName = "");
        static GameObject* LoadObject(const nlohmann::json& item, World& world);
        static std::unique_ptr<GameObject> LoadObjectOrphan(const nlohmann::json& item, World& world);
        
        static void LoadCollection(const nlohmann::json& data, World& world, Vec2 offset = Vec2(0,0));

    private:
        static void LoadConstraints(const nlohmann::json& constraints, World& world, const std::unordered_map<int, GameObject*>& idMap);
        static void LoadControllers(const nlohmann::json& controllers, World& world);
};