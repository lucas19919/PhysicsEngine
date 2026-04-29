#pragma once
#include "raylib.h"

#include "math/Vec2.h"

class EditorCamera {
public:
    EditorCamera(float screenWidth, float screenHeight);

    void Begin();
    void End();

    void Pan(Vec2 delta);
    void Zoom(float delta, Vec2 mousePos);

    Vec2 ScreenToWorldMeters(Vec2 screenPos) const;
    Vec2 WorldToScreenPixels(Vec2 worldPos) const;

    const Camera2D& GetRaylibCamera() const { return camera; }
    Camera2D& GetRaylibCamera() { return camera; }

private:
    Camera2D camera;
};
