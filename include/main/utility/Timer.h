#pragma once
#include <chrono>

class Timer
{
    public:
        Timer() = default;
        ~Timer() = default;

        void StartTimer() { startPoint = std::chrono::high_resolution_clock::now(); }

        float StopTimer() { 
            auto endTime = std::chrono::high_resolution_clock::now();

            auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startPoint).time_since_epoch().count();
            auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

            auto duration = end - start;
            double ms = duration * 0.001; 

            return static_cast<float>(ms);
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> startPoint;
};