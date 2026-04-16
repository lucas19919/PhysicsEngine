#pragma once
#include "math/Vec2.h"
#include "main/components/Component.h"

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

        BBox GetBounds() const { return bounds; }
        void SetBounds(const BBox& newBounds) { bounds = newBounds; }

        bool isActive = true;
        void Toggle();
    
    private:
            BBox bounds;

    };