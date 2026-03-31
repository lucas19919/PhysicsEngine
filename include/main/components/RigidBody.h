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

        void AddContact(Vec2 normal);
        void ResetContacts();

        bool isSurrounded = false;
        bool isSleeping = false;
        float sleepTimer = 0.0f;
        float energyThreshold = 15.0f; 
        float timeToSleep = 0.5f;

        void WakeUp();
        void UpdateSleep(float dt);

        void SetMass(float m);
        const float& GetMass() const { return mass; }
        const float& GetInvMass() const { return invMass; }

        void SetInertia(float i) { inertia = i; invInertia = 1.0f / i; }
        const float& GetInertia() const { return inertia; }
        const float& GetInvInertia() const { return invInertia; }

        void SetRestitution(float e) { restitution = e; }
        const float& GetRestitution() const { return restitution; }

        void SetFriction(float f) { friction = f; }
        const float& GetFriction() const { return friction; }

        const Vec2& GetVelocity() const { return velocity; }
        void SetVelocity(Vec2 v) { velocity = v; }

        const Vec2& GetAcceleration() const { return acceleration; }
        void SetAcceleration(Vec2 a) { acceleration = a; }

        void ApplyForce(Vec2 force);
        const Vec2& GetForce() const { return netForce; }   
        void ClearForces();
        
        const float& GetAngularVelocity() const { return angularVelocity; }
        void SetAngularVelocity(float w) { angularVelocity = w; }

        const float& GetAngularAcceleration() const { return angularAcceleration; }
        void SetAngularAcceleration(float a) { angularAcceleration = a; }

        void ApplyTorque(float t) { torque += t; }
        const float& GetTorque() const { return torque; }
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

        Vec2 contactNormalSum;
        int contactCount = 0;
};