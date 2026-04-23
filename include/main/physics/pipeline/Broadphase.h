#pragma once
#include "main/GameObject.h"
#include "main/physics/SpatialHash.h"
#include "main/physics/Config.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>

class Broadphase
{
    public:
        Broadphase() : spatialHash(Config::spatialHashCellSize) {}

        void Clear();

        void UpdateBroadphase(std::vector<std::unique_ptr<GameObject>>& gameObjects);
        std::vector<std::pair<GameObject*, GameObject*>> GeneratePairs();
    private:
        SpatialHash spatialHash;
        std::unordered_map<unsigned int, std::vector<GameObject*>> gridMap;
        std::unordered_set<uint64_t> candidatePairKeys;
};