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

        bool isSleeping = false;

        void SetMass(float m);
        float GetMass() const { return mass; }
        float GetInvMass() const { return invMass; }

        void SetInertia(float i) { inertia = i; invInertia = 1.0f / i; }
        float GetInertia() const { return inertia; }
        float GetInvInertia() const { return invInertia; }

        void SetRestitution(float e) { restitution = e; }
        float GetRestitution() const { return restitution; }

        void SetFriction(float f) { friction = f; }
        float GetFriction() const { return friction; }

        Vec2 GetVelocity() const { return velocity; }
        void SetVelocity(Vec2 v) { velocity = v; }

        Vec2 GetAcceleration() const { return acceleration; }
        void SetAcceleration(Vec2 a) { acceleration = a; }

        void ApplyForce(Vec2 force);
        Vec2 GetForce() const { return netForce; }   
        void ClearForces();
        
        float GetAngularVelocity() const { return angularVelocity; }
        void SetAngularVelocity(float w) { angularVelocity = w; }

        float GetAngularAcceleration() const { return angularAcceleration; }
        void SetAngularAcceleration(float a) { angularAcceleration = a; }

        void ApplyTorque(float t) { torque += t; }
        float GetTorque() const { return torque; }
        void ClearTorque();
        
    private:
        float mass;
        float invMass;
        float inertia;
        float invInertia;        
        float restitution;
        float friction;

        Vec2 velocity;
        Vec2 acceleration;
        Vec2 netForce;

        float angularVelocity;
        float angularAcceleration;  
        float torque;
};