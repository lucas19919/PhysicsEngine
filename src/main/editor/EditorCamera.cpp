#include "main/editor/EditorCamera.h"
#include "main/physics/Config.h"
#include "raymath.h"

EditorCamera::EditorCamera(float screenWidth, float screenHeight) {
    camera.target = { 0.0f, 0.0f };
    camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 0.7f;
}

void EditorCamera::Begin() {
    BeginMode2D(camera);
}

void EditorCamera::End() {
    EndMode2D();
}

void EditorCamera::Pan(Vec2 delta) {
    // Scaling delta by 1/zoom so panning feels consistent at all zoom levels
    Vector2 d = { delta.x, delta.y };
    camera.target = Vector2Add(camera.target, Vector2Scale(d, -1.0f / camera.zoom));
}

void EditorCamera::Zoom(float delta, Vec2 mousePos) {
    // Zoom around mouse position
    Vector2 m = { mousePos.x, mousePos.y };
    Vector2 mouseWorldPos = GetScreenToWorld2D(m, camera);
    
    camera.offset = m;
    camera.target = mouseWorldPos;

    float scaleFactor = 1.1f;
    if (delta > 0) camera.zoom *= scaleFactor;
    else if (delta < 0) camera.zoom /= scaleFactor;

    if (camera.zoom < 0.1f) camera.zoom = 0.1f;
    if (camera.zoom > 10.0f) camera.zoom = 10.0f;
}

Vec2 EditorCamera::ScreenToWorldMeters(Vec2 screenPos) const {
    Vector2 s = { screenPos.x, screenPos.y };
    Vector2 worldPixels = GetScreenToWorld2D(s, camera);
    return Vec2(worldPixels.x * Config::PixelToMeter, worldPixels.y * Config::PixelToMeter);
}

Vec2 EditorCamera::WorldToScreenPixels(Vec2 worldPos) const {
    Vector2 worldPixels = { worldPos.x * Config::MeterToPixel, worldPos.y * Config::MeterToPixel };
    Vector2 screenPixels = GetWorldToScreen2D(worldPixels, camera);
    return Vec2(screenPixels.x, screenPixels.y);
}
