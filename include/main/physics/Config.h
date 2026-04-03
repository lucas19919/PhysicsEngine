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
    int solverIterations = 8;                // subticks for solver

    // Collision resolution / position correction
    float contactSlop = 0.1f;        
    float positionCorrectionPercent = 0.6f; 

    // Scene loading / generators
    float generatorJitterRange = 4.0f; // LoadScene: uniform_real_distribution(-4.0f,4.0f)

    // Debug
    bool debugSleepingStates = true; // Draw::Render, colors sleeping objects differently
};