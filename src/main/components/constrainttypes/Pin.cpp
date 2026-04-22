#include "main/components/constrainttypes/Pin.h"
#include "main/components/Constraint.h"
#include "main/World.h"
#include "main/physics/Config.h"
#include "math/RotationMatrix.h"
#include "math/Matrix2x2.h"

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
    for (PinAttachment att : attachments)
    {
        GameObject* obj = att.obj;
        RigidBody* rb = obj->rb;
        if (!rb) continue;

        Vec2 r = RotMatrix(obj->transform.rotation).Rotate(att.localAnchor);

        Vec2 worldAnchor = obj->transform.position + r;
        Vec2 delta = position - worldAnchor;

        Vec2 v = rb->GetVelocity();
        float w = rb->GetAngularVelocity();
        
        Vec2 pointVelocity(v.x - w * r.y, v.y + w * r.x);
        Vec2 biasVelocity = delta * (Config::biasConstraint / dt);
        Vec2 targetVelocity = biasVelocity - pointVelocity;
        
        //k
        float iI = rb->GetInvInertia();
        float iM = rb->GetInvMass();

        if (fixedX && fixedY)
        {
            Matrix2x2 K(
                iM + (r.y * r.y) * iI,
                -r.x * r.y * iI,
                -r.x * r.y * iI,       
                iM + (r.x * r.x) * iI
            );

            Matrix2x2 K_inv = K.Inverse();
            Vec2 impulse = K_inv * targetVelocity;

            rb->SetVelocity(v + impulse * iM);
            rb->SetAngularVelocity(w + (r.Cross(impulse) * iI));
        }
        else if (fixedX)
        {
            float effMassX = iM + (r.y * r.y) * iI;
            if (effMassX > 0.0f)
            {
                float impulseX = targetVelocity.x / effMassX;
                rb->SetVelocity(Vec2(v.x + impulseX * iM, v.y));
                rb->SetAngularVelocity(w + (r.Cross(Vec2(impulseX, 0.0f)) * iI));
            }
        }
        else if (fixedY)
        {
            float effMassY = iM + (r.x * r.x) * iI;
            if (effMassY > 0.0f)
            {
                float impulseY = targetVelocity.y / effMassY;
                rb->SetVelocity(Vec2(v.x, v.y + impulseY * iM));
                rb->SetAngularVelocity(w + (r.Cross(Vec2(0.0f, impulseY)) * iI));
            }
        }

        if (!fixedX) position.x = worldAnchor.x;
        if (!fixedY) position.y = worldAnchor.y;
    }
}