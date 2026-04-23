#pragma once
#include "math/Vec2.h"
#include <string>

struct Config
{
    static inline int screenWidth = 1280;          
    static inline int screenHeight = 960;          
    static inline int targetFPS = 60;      

    static inline bool drawFPS = true;

    static constexpr float MeterToPixel = 50.0f;
    static constexpr float PixelToMeter = 1.0f / MeterToPixel;

    static inline Vec2 gravity = Vec2(0.0f, 9.81f);
    static inline float spatialHashCellSize = 2.0f;
    
    static inline int pipelineSubTicks = 8; //subticks for pipeline (broadphase, solver etc)
    static inline int impulseIterations = 4; //iterations for impulse collision solver                
    static inline int positionIterations = 1; //iterations for position correction

    static inline bool warmStart = true;

    static inline float restitutionThreshold = 0.5f; //10-15 best for now

    static inline float contactSlop = 0.01f;        
    static inline float positionCorrectionPercent = 0.2f; 

    static inline float biasConstraint = 0.1f; 

    static inline float generatorJitterRange = 0.1f;
};