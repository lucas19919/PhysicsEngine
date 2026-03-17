#include "physics/RigidBody.h"
#include "math/Vec2.h"

RigidBody::RigidBody(float m, Vec2 pos, Vec2 v, Vec2 a, Vec2 F, float r, float e)
{
    mass = m;
    invMass = 1 / m;
    position = pos;
    velocity = v;
    acceleration = a;
    forceSum = F;
    radius = r;
    restitution = e;
}

void RigidBody::SetMass(float m)
{
    mass = m;
    invMass = 1 / m;
}

void RigidBody::ApplyForce(Vec2 force)
{
    forceSum += force;
}

void RigidBody::ClearForces() 
{
    forceSum = Vec2();
}