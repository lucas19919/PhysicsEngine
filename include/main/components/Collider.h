#pragma once
#include "main/components/Component.h"
#include "main/components/TransformComponent.h"
#include "main/utility/templates/Array.h"
#include "math/Vec2.h"

class GameObject;

enum ColliderType { 
    CIRCLE,  
    BOX,
    POLYGON
};

struct BBox {
    Vec2 min;
    Vec2 max;
};

class Collider : public Component
{
    public:
        virtual ~Collider() = default;
        virtual ColliderType GetType() const = 0;

        const char* GetName() const override { return "Collider"; }
        bool OnInspectorGui(class World* world = nullptr) override;

        const Array<20>& GetVertices() const { return cachedVertices; }
        const Array<20>& GetNormals() const { return cachedNormals; }

        virtual void UpdateCache(const TransformComponent& transform) = 0;
        virtual bool TestPoint(Vec2 point) const = 0;
        virtual void Scale(float sx, float sy) = 0;

        BBox GetBounds() const { return bounds; }
        void SetBounds(const BBox& newBounds) { bounds = newBounds; }

        bool isActive = true;
        void Toggle();

    protected:
        Array<20> cachedVertices;
        Array<20> cachedNormals;
        BBox bounds;
};