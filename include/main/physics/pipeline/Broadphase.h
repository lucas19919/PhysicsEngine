#pragma once
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "main/GameObject.h"
#include "main/physics/Config.h"
#include "main/physics/SpatialHash.h"

class Broadphase
{
    public:
        Broadphase() : spatialHash(Config::spatialHashCellSize) {}

        void Clear();

        void UpdateBroadphase(std::vector<std::unique_ptr<GameObject>>& gameObjects);
        std::vector<std::pair<GameObject*, GameObject*>> GeneratePairs();

        const std::unordered_map<unsigned int, std::vector<GameObject*>>& GetGridMap() const { return gridMap; }
    private:
        SpatialHash spatialHash;
        std::unordered_map<unsigned int, std::vector<GameObject*>> gridMap;
        std::unordered_set<uint64_t> candidatePairKeys;
};