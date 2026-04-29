#pragma once
#include <string>

#include <external/nlohmann/json.hpp>

#include "main/World.h"

class SaveScene
{
public:
    static void Save(const std::string& filePath, World& world);
    static void SaveCollection(const std::string& filePath, World& world, const std::string& groupName);
    static nlohmann::json SerializeObject(GameObject* obj);
    static nlohmann::json SerializeScene(World& world);

private:
    static nlohmann::json SerializeConstraints(const std::vector<Constraint*>& constraints);
    static nlohmann::json SerializeControllers(const std::vector<Controller*>& controllers);
};
