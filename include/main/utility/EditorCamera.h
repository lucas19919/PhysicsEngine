#pragma once
#include "raylib.h"
#include "math/Vec2.h"

class EditorCamera {
public:
    EditorCamera(float screenWidth, float screenHeight);

    void Begin();
    void End();

    void Pan(Vector2 delta);
    void Zoom(float delta, Vector2 mousePos);

    // Convert Screen Pixels -> World Meters
    Vec2 ScreenToWorldMeters(Vector2 screenPos) const;
    
    // Convert World Meters -> Screen Pixels
    Vector2 WorldToScreenPixels(Vec2 worldPos) const;

    Camera2D& GetRaylibCamera() { return camera; }

private:
    Camera2D camera;
};
