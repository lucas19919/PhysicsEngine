#include "main/components/constrainttypes/Pin.h"
#include "main/components/Constraint.h"
#include "main/World.h"
#include "main/physics/Config.h"

PinConstraint::PinConstraint(const std::vector<PinAttachment>& attachments, Vec2 pos, bool fixedX, bool fixedY)
    : attachments(attachments), fixedX(fixedX), fixedY(fixedY)
{ 
    this->position = pos;
}

ConstraintType PinConstraint::GetType() const
{
    return ConstraintType::PIN;
}

void PinConstraint::Solve(float dt)
{
    if (staticallyDetermined) return;

    float biasConstraint = Config().biasConstraint;

    for (auto& att : attachments)
    {
        RigidBody* rb = att.obj->GetRigidBody();
        if (!rb) continue;

        float invMass = rb->GetInvMass();
        float invInertia = rb->GetInvInertia();
        if (invMass == 0.0f) continue;

        float s = sinf(att.obj->transform.rotation);
        float c = cosf(att.obj->transform.rotation);
        Vec2 rotatedAnchor = Vec2(
            att.localAnchor.x * c - att.localAnchor.y * s,
            att.localAnchor.x * s + att.localAnchor.y * c
        );

        Vec2 worldAnchor = att.obj->transform.position + rotatedAnchor;
        Vec2 delta = worldAnchor - position;

        Vec2 velocity = rb->GetVelocity();
        float angularVelocity = rb->GetAngularVelocity();

        Vec2 pointVelocity = velocity + Vec2(-angularVelocity * rotatedAnchor.y, angularVelocity * rotatedAnchor.x);

        Vec2 bias = (biasConstraint / dt) * delta;
        Vec2 impulse(0.0f, 0.0f);

        if (fixedX) {
            Vec2 n(1, 0); // X direction
            float rn = rotatedAnchor.Cross(n);
            float effMass = invMass + rn * rn * invInertia;
            if (effMass > 0.0f) {
                float velAlong = pointVelocity.x;
                float lambda = -(velAlong + bias.x) / effMass;
                impulse.x = lambda;
            }
        }

        if (fixedY) {
            Vec2 n(0, 1); // Y direction
            float rn = rotatedAnchor.Cross(n);
            float effMass = invMass + rn * rn * invInertia;
            if (effMass > 0.0f) {
                float velAlong = pointVelocity.y;
                float lambda = -(velAlong + bias.y) / effMass;
                impulse.y = lambda;
            }
        }

        velocity += impulse * invMass;
        angularVelocity += rotatedAnchor.Cross(impulse) * invInertia;

        rb->SetVelocity(velocity);
        rb->SetAngularVelocity(angularVelocity);
    }

    if (!attachments.empty())
    {
        auto& first = attachments[0];
        float sin = sinf(first.obj->transform.rotation);
        float cos = cosf(first.obj->transform.rotation);

        Vec2 rotatedAnchor = Vec2(
            first.localAnchor.x * cos - first.localAnchor.y * sin,
            first.localAnchor.x * sin + first.localAnchor.y * cos
        );

        Vec2 worldAnchor = first.obj->transform.position + rotatedAnchor;

        if (!fixedX) position.x = worldAnchor.x;
        if (!fixedY) position.y = worldAnchor.y;
    }
}