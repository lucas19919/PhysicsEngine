#pragma once

class GameObject;

enum ColliderType { 
    CIRCLE, 
    BOX 
};

class Collider
{
    public:
        virtual ~Collider() = default;
        virtual ColliderType getType() const = 0;

        bool isActive = true;
        void Toggle();

        GameObject* parent;
    };