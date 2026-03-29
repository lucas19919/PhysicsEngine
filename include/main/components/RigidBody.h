#pragma once
#include "math/Vec2.h"

class GameObject;

struct Properties
{
    float mass;
    float restitution;
    float inertia;
    float friction;
};

struct LinearState
{
    Vec2 velocity;
    Vec2 acceleration;
    Vec2 netForce;
};

struct AngularState
{
    float angularVelocity;
    float angularAcceleration;
    float torque;
};

class RigidBody 
{
    public: 
        RigidBody(Properties properties, LinearState linearState, AngularState angularState);
        ~RigidBody();

        GameObject* parent;

        Vec2 velocity;
        Vec2 acceleration;

        void SetMass(float m);
        float GetMass() const { return mass; }
        float GetInvMass() const { return invMass; }

        void SetRestitution(float e) { restitution = e; }
        float GetRestitution() const { return restitution; }

        void SetFriction(float f) { friction = f; }
        float GetFriction() const { return friction; }

        void ApplyForce(Vec2 force);
        Vec2 GetForce() const { return netForce; }   
        void ClearForces();
        
        //clockwise rotation is positive
        float angularVelocity;
        float angularAcceleration;        

        void SetInertia(float i) { inertia = i; invInertia = 1 / i; }
        float GetInertia() const { return inertia; }
        float GetInvInertia() const { return invInertia; }

        void ApplyTorque(float t) { torque += t; }
        float GetTorque() const { return torque; }
        void ClearTorque();
        
    private:
        float mass;
        float invMass;
        float restitution;
        float friction;

        Vec2 netForce;

        float inertia;
        float invInertia;

        float torque;
};