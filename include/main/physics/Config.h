#pragma once
#include "math/Vec2.h"

struct Config
{
    
    int screenWidth = 1280;          
    int screenHeight = 960;          
    int targetFPS = 60;      
    
    bool drawFPS = false;

    Vec2 gravity = Vec2(0.0f, 600.0f);
    float spatialHashCellSize = 50.0f;
    
    int impulseIterations = 8;               
    int positionIterations = 1;              

    bool warmStart = true;

    float restitutionThreshold = 15.0f; //10-15 best for now

    float contactSlop = 0.10f;        
    float positionCorrectionPercent = 0.1f; 

    //constraints
    float biasConstraint = 2.0f; 

    //sleep
    float energyThreshold = 25.0f;
    float sleepTime = 0.5f;
    bool debugSleep = false; //different color for sleeping obj

    float generatorJitterRange = 4.0f;
};