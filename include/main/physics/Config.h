#pragma once
#include "math/Vec2.h"

struct Config
{
    // Window / rendering
    int screenWidth = 1280;          // src/main.cpp
    int screenHeight = 960;          // src/main.cpp
    int targetFPS = 60;              // src/main.cpp

    // World / physics
    Vec2 gravity = Vec2(0.0f, 600.0f);
    float spatialHashCellSize = 35.0f;
    
    // Integration / stepping
    int impulseIterations = 8;               // velocity solver iterations
    int positionIterations = 3;              // position solver iterations

    bool warmStart = false;                 // warmstart solver

    // Collision resolution / position correction
    float contactSlop = 0.05f;        
    float positionCorrectionPercent = 0.2f; 

    // Scene loading / generators
    float generatorJitterRange = 4.0f; // LoadScene

    // Debug
    
};