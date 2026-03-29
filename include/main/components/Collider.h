#pragma once

class GameObject;

enum ColliderType { 
    CIRCLE, 
    BOX,
    POLYGON
};

class Collider
{
    public:
        virtual ~Collider() = default;
        virtual ColliderType GetType() const = 0;

        bool isActive = true;
        void Toggle();

    };