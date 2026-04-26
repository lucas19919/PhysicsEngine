#include "main/components/Renderer.h"
#include "math/Vec2.h"
#include <cmath>

#include "external/imgui/imgui.h"

bool Renderer::OnInspectorGui(World* world) {
    bool changed = false;
    float color[4] = { (float)shape.color.r / 255.0f, (float)shape.color.g / 255.0f, (float)shape.color.b / 255.0f, (float)shape.color.a / 255.0f };
    if (ImGui::ColorEdit4("Color", color)) {
        shape.color.r = (unsigned char)(color[0] * 255.0f);
        shape.color.g = (unsigned char)(color[1] * 255.0f);
        shape.color.b = (unsigned char)(color[2] * 255.0f);
        shape.color.a = (unsigned char)(color[3] * 255.0f);
        changed = true;
    }
    
    const char* shapes[] = { "Circle", "Box", "Polygon" };
    int currentShape = (int)shape.form;
    if (ImGui::Combo("Form", &currentShape, shapes, IM_ARRAYSIZE(shapes))) {
        shape.form = (RenderShape)currentShape;
        
        // Reset scale to defaults for the new shape type
        if (shape.form == R_CIRCLE) shape.scale = 1.0f;
        else if (shape.form == R_BOX) shape.scale = Vec2(1.0f, 1.0f);
        else if (shape.form == R_POLYGON) shape.scale = Array<20>();
        
        UpdateLocalCoordinates();
        changed = true;
    }

    if (shape.form == R_CIRCLE) {
        float radius = std::get<float>(shape.scale);
        if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.01f, 100.0f)) {
            shape.scale = radius;
            UpdateLocalCoordinates();
            changed = true;
        }
    } else if (shape.form == R_BOX) {
        Vec2 size = std::get<Vec2>(shape.scale);
        if (ImGui::DragFloat2("Size", &size.x, 0.1f, 0.01f, 100.0f)) {
            shape.scale = size;
            UpdateLocalCoordinates();
            changed = true;
        }
    }

    return changed;
}

Renderer::Renderer(Shape shape) : shape(shape) 
{
    UpdateLocalCoordinates();
}

void Renderer::UpdateLocalCoordinates()
{
    localCoordinates = Array<20>();

    if (shape.form == RenderShape::R_BOX)
    {
        float x = std::get<Vec2>(shape.scale).x / 2.0f;
        float y = std::get<Vec2>(shape.scale).y / 2.0f;
        
        localCoordinates.PushBack(Vec2(-x, -y));
        localCoordinates.PushBack(Vec2(x, -y));
        localCoordinates.PushBack(Vec2(x, y));
        localCoordinates.PushBack(Vec2(-x, y));
    }
    else if (shape.form == RenderShape::R_POLYGON)
    {
        localCoordinates = std::get<Array<20>>(shape.scale);
    }
    else if (shape.form == RenderShape::R_CIRCLE)
    {
        localCoordinates.PushBack(Vec2());
    }
}

Array<20> Renderer::GetLocalCoordinates() const
{
    return localCoordinates;
}

Array<20> Renderer::GetWorldCoordinates(Vec2 position) const
{
    if (shape.form == RenderShape::R_CIRCLE)
    {
        return { position };
    }
    else if (shape.form == RenderShape::R_BOX)
    {
        return worldCoordinates;
    }
    else if (shape.form == RenderShape::R_POLYGON)
    {
        return worldCoordinates;
    }

    return {};
}

#include "math/RotationMatrix.h"

Array<20> Renderer::UpdateWorldCoordinates(Vec2 position, float rotation)
{
    worldCoordinates = Array<20>();

    RotMatrix rot(rotation);

    for (size_t i = 0; i < localCoordinates.Size(); i++)
    {
        Vec2 vertex = localCoordinates[i];
        Vec2 rotated = rot.Rotate(vertex);
        worldCoordinates.PushBack(Vec2(rotated.x + position.x, rotated.y + position.y));
    }

    return worldCoordinates;
}
