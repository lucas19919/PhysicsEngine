#include "main/components/RigidBody.h"
#include "math/Vec2.h"
#include <algorithm>

RigidBody::RigidBody(Properties properties, LinearState linearState, AngularState angularState)
{
    mass = properties.mass > 0.0f ? properties.mass : 1.0f;
    inertia = properties.inertia > 0.0f ? properties.inertia : 1.0f;
    restitution = std::clamp(properties.restitution, 0.0f, 1.0f);
    friction = std::max(0.0f, properties.friction);

    invMass = (properties.mass > 0.0f) ? (1.0f / properties.mass) : 0.0f;
    invInertia = (properties.inertia > 0.0f) ? (1.0f / properties.inertia) : 0.0f;

    velocity = linearState.velocity;
    acceleration = linearState.acceleration;
    netForce = linearState.netForce;

    angularVelocity = angularState.angularVelocity;
    angularAcceleration = angularState.angularAcceleration;
    torque = angularState.torque;
}

RigidBody::~RigidBody()
{
}

void RigidBody::AddContact(Vec2 normal)
{
    contactNormalSum += normal;
    contactCount++;
}

void RigidBody::ResetContacts()
{
    contactNormalSum = Vec2(0, 0);
    contactCount = 0;
}

void RigidBody::WakeUp()
{
    isSleeping = false;
    sleepTimer = 0.0f;
}

void RigidBody::UpdateSleep(float dt)
{
    if (isSleeping) return;
    isSurrounded = false;

    if (contactCount >= 3)
    {
        if (contactNormalSum.MagSq() < 2.5f)
        {
            isSurrounded = true;
            isSleeping = true;
        }
    }

    float eK = 0.5f * mass * velocity.MagSq() + 0.5f * inertia * (angularVelocity * angularVelocity);
    if (eK < energyThreshold)
    {
        sleepTimer += dt;
        if (sleepTimer >= timeToSleep)
            isSleeping = true;
    }
    else
        sleepTimer = 0.0f;

    if (isSleeping)
    {
        velocity = Vec2(0, 0);
        angularVelocity = 0.0f;
    }
}

void RigidBody::SetMass(float m)
{
    mass = m;
    invMass = (m > 0.0f) ? (1.0f / m) : 0.0f;
}

void RigidBody::ApplyForce(Vec2 force)
{
    netForce += force;
}

void RigidBody::ClearForces() 
{
    netForce = Vec2();
}

void RigidBody::ClearTorque()
{
    torque = 0.0f;
}