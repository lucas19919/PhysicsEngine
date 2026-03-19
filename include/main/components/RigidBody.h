#pragma once
#include "math/Vec2.h"

class GameObject;

class RigidBody 
{
    public: 
        RigidBody(float m, float e, Vec2 v, Vec2 a, Vec2 F);
        ~RigidBody();

        GameObject* parent;

        Vec2 velocity;
        Vec2 acceleration;

        void SetMass(float m);
        float GetMass() const { return mass; }
        float GetInvMass() const { return invMass; }

        void SetRestitution(float e) { restitution = e; }
        float GetRestitution() const { return restitution; }

        void ApplyForce(Vec2 force);
        Vec2 GetForce() const { return netForce; }        

        void ClearForces();
    private:
        float mass;
        float invMass;
        float restitution;

        Vec2 netForce;
};