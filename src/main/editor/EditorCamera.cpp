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
    // Manual conversion to avoid dependency on global Raylib screen state
    // worldPos = (screenPos - offset) / zoom + target
    float worldX = (screenPos.x - camera.offset.x) / camera.zoom + camera.target.x;
    float worldY = (screenPos.y - camera.offset.y) / camera.zoom + camera.target.y;
    
    return Vec2(worldX * Config::PixelToMeter, worldY * Config::PixelToMeter);
}

Vec2 EditorCamera::WorldToScreenPixels(Vec2 worldPos) const {
    // Manual conversion to avoid dependency on global Raylib screen state
    // screenPos = (worldPos - target) * zoom + offset
    float worldPixelsX = worldPos.x * Config::MeterToPixel;
    float worldPixelsY = worldPos.y * Config::MeterToPixel;
    
    float screenX = (worldPixelsX - camera.target.x) * camera.zoom + camera.offset.x;
    float screenY = (worldPixelsY - camera.target.y) * camera.zoom + camera.offset.y;
    
    return Vec2(screenX, screenY);
}
