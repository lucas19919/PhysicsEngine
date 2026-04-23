#include "main/components/Renderer.h"
#include "math/Vec2.h"
#include <cmath>

Renderer::Renderer(Shape shape) : shape(shape) 
{
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