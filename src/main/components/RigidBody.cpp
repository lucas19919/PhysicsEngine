#include "main/components/RigidBody.h"
#include "math/Vec2.h"

RigidBody::RigidBody(float m, float e, Vec2 v, Vec2 a, Vec2 F)
{
    mass = m;
    invMass = 1 / m;
    restitution = e;

    velocity = v;
    acceleration = a;

    netForce = F;
}

RigidBody::~RigidBody()
{
}


void RigidBody::SetMass(float m)
{
    mass = m;
    invMass = 1 / m;
}

void RigidBody::ApplyForce(Vec2 force)
{
    netForce += force;
}

void RigidBody::ClearForces() 
{
    netForce = Vec2();
}