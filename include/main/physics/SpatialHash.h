#pragma once
#include "math/Vec2.h"
#include "main/components/Collider.h"
#include "main/GameObject.h"
#include <vector>


class SpatialHash
{
public:
    SpatialHash(float cellSize) : cellSize(cellSize) {}

    unsigned int GetHash(Vec2 position) const;
    std::vector<Vec2> GetBounding(GameObject* obj);

    float GetCellSize() const { return cellSize; }
private:
    float cellSize;
};