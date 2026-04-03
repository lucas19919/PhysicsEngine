#pragma once
#include "math/Vec2.h"

struct Config
{
    int screenWidth = 1280;          
    int screenHeight = 960;          
    int targetFPS = 60;              

    Vec2 gravity = Vec2(0.0f, 600.0f);
    float spatialHashCellSize = 35.0f;
    
    int impulseIterations = 20;               
    int positionIterations = 8;              

    bool warmStart = true;                 
    float restitutionThreshold = 15.0f;

    float contactSlop = 0.15f;        
    float positionCorrectionPercent = 0.1f; 

    float generatorJitterRange = 4.0f;

    float linearDamping = 0.99f;
    float angularDamping = 0.98f;
};