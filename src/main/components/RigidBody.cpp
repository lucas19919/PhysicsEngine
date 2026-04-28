#include "main/components/RigidBody.h"

#include <algorithm>

#include "external/imgui/imgui.h"

#include "math/Vec2.h"

bool RigidBody::OnInspectorGui(World* world) {
    bool changed = false;
    float m = mass;
    if (ImGui::DragFloat("Mass", &m, 0.1f, 0.001f, 10000.0f)) {
        SetMass(m);
        changed = true;
    }

    if (ImGui::DragFloat("Restitution", &restitution, 0.01f, 0.0f, 1.0f)) changed = true;
    if (ImGui::DragFloat("Friction", &friction, 0.01f, 0.0f, 1.0f)) changed = true;
    if (ImGui::DragFloat("Inertia", &inertia, 0.1f, 0.001f, 10000.0f)) {
        if (inertia > 0.0f) invInertia = 1.0f / inertia;
        changed = true;
    }

    if (ImGui::Checkbox("Use Gravity", &gravityEnabled)) changed = true;

    ImGui::Separator();
    ImGui::Text("Linear State");
    if (ImGui::DragFloat2("Velocity", &velocity.x, 0.1f)) changed = true;
    if (ImGui::DragFloat2("Acceleration", &acceleration.x, 0.1f)) changed = true;
    if (ImGui::DragFloat2("Net Force", &netForce.x, 0.1f)) changed = true;

    ImGui::Separator();
    ImGui::Text("Angular State");
    if (ImGui::DragFloat("Angular Velocity", &angularVelocity, 0.1f)) changed = true;
    if (ImGui::DragFloat("Angular Accel", &angularAcceleration, 0.1f)) changed = true;
    if (ImGui::DragFloat("Torque", &torque, 0.1f)) changed = true;

    return changed;
}

RigidBody::RigidBody(Properties properties, LinearState linearState, AngularState angularState, Settings settings)
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

    gravityEnabled = settings.gravityEnabled;
}

RigidBody::~RigidBody()
{
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
