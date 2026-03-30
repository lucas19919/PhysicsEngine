#include "main/components/Renderer.h"
#include "math/Vec2.h"
#include <cmath>

Renderer::Renderer(Shape shape) : shape(shape) 
{
    if (shape.form == RenderShape::R_BOX)
    {
        float x = std::get<Vec2>(shape.scale).x / 2.0f;
        float y = std::get<Vec2>(shape.scale).y / 2.0f;
        
        localCoordinates.push_back(Vec2(-x, -y));
        localCoordinates.push_back(Vec2(x, -y));
        localCoordinates.push_back(Vec2(x, y));
        localCoordinates.push_back(Vec2(-x, y));
    }
    else if (shape.form == RenderShape::R_POLYGON)
    {
        localCoordinates = std::get<Array<20>>(shape.scale);
    }
    else if (shape.form == RenderShape::R_CIRCLE)
    {
        localCoordinates.push_back(Vec2());
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

Array<20> Renderer::UpdateWorldCoordinates(Vec2 position, float rotation)
{
    worldCoordinates = Array<20>();

    float cosR = std::cos(rotation);
    float sinR = std::sin(rotation);

    for (size_t i = 0; i < localCoordinates.size(); i++)
    {
        Vec2 vertex = localCoordinates[i];
        float x = vertex.x * cosR - vertex.y * sinR;
        float y = vertex.x * sinR + vertex.y * cosR;
        worldCoordinates.push_back(Vec2(x + position.x, y + position.y));
    }

    return worldCoordinates;
}