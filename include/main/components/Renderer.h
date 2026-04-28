#pragma once
#include <variant>
#include <vector>

#include "raylib.h"

#include "main/components/Component.h"
#include "main/utility/templates/Array.h"
#include "math/Vec2.h"

class GameObject;

enum RenderShape {
    R_CIRCLE,
    R_BOX,
    R_POLYGON
};

typedef struct {
    RenderShape form;
    Color color = RED;
    std::variant<Vec2, float, Array<20>> scale; 
} Shape;

class Renderer : public Component
{
    public:
        Renderer(Shape shape);
        ~Renderer() = default;

        const char* GetName() const override { return "Renderer"; }
        bool OnInspectorGui(class World* world = nullptr) override;

        Array<20> GetLocalCoordinates() const;
        Array<20> GetWorldCoordinates(Vec2 position) const;
        Array<20> UpdateWorldCoordinates(Vec2 position, float rotation);

        Shape GetShape() const { return shape; }

        void UpdateLocalCoordinates();
        void Scale(float sx, float sy);

        //Debug helper
        void SetColor(Color color) { shape.color = color; }

    private:
        Shape shape;
        
        Array<20> localCoordinates;
        Array<20> worldCoordinates;
};