#pragma once
#include <vector>

#include "main/GameObject.h"
#include "main/components/Collider.h"
#include "math/Vec2.h"

class SpatialHash
{
public:
    SpatialHash(float cellSize) : cellSize(cellSize) {}

    unsigned int GetHash(Vec2 position) const;
    BBox GetBounding(GameObject* obj);

    float GetCellSize() const { return cellSize; }
private:
    float cellSize;
};