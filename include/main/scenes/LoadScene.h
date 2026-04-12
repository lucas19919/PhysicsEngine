#pragma once
#include <string>
#include <unordered_map>
#include <libraries/json/json.hpp>

class World;
class GameObject;

class LoadScene
{
    public:
        static void Load(const std::string& filePath, World& world, int screenWidth, int screenHeight);
    private:
        static GameObject* LoadObject(const nlohmann::json& item, World& world);
        static void LoadConstraints(const nlohmann::json& constraints, World& world, const std::unordered_map<int, GameObject*>& idMap);
        static void LoadControllers(const nlohmann::json& controllers, World& world);
};