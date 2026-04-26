#pragma once
#include "math/Vec2.h"
#include "main/components/Component.h"

class GameObject;

enum ConstraintType
{
    DISTANCE,
    PIN,
    JOINT,
    MOTOR
};

class Constraint : public Component
{
    public: 
        Vec2 position;

        void SetID(size_t newID) { id = newID; }
        const size_t& GetID() const { return id; }

        virtual ~Constraint() = default;
        virtual ConstraintType GetType() const = 0;
        virtual void Solve(float dt) = 0;
        virtual bool InvolvesObject(GameObject* obj) const = 0;

        bool OnInspectorGui(class World* world = nullptr) override { return false; }
        
    private:
        size_t id;
};