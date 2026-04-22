#include "main/components/constrainttypes/Joint.h"
#include "main/components/Constraint.h"
#include "main/World.h"
#include "main/physics/Config.h"
#include "math/RotationMatrix.h"
#include "math/Matrix2x2.h"

JointConstraint::JointConstraint(std::vector<JointAttachment> attachments, Vec2 position, bool collisions)
    : attachments(attachments), collisions(collisions)
{ 
    this->position = position;
}

ConstraintType JointConstraint::GetType() const
{
    return ConstraintType::JOINT;
}

void JointConstraint::Solve(float dt)
{
    if (attachments.size() < 2) 
        return;
    else if (attachments.size() == 2)
        SingleJoint(dt);
    else
        ComplexJoint(dt);
}

void JointConstraint::SingleJoint(float dt)
{
    if (attachments.size() < 2) return;

    GameObject* obj1 = attachments[0].obj;
    RigidBody* rb1 = obj1->rb;
    
    float invMass1 = (rb1) ? rb1->GetInvMass() : 0.0f;
    float invInertia1 = (rb1) ? rb1->GetInvInertia() : 0.0f;

    for (size_t i = 1; i < attachments.size(); ++i)
    {
        GameObject* obj2 = attachments[i].obj;
        RigidBody* rb2 = obj2->rb;

        float invMass2 = (rb2) ? rb2->GetInvMass() : 0.0f;
        float invInertia2 = (rb2) ? rb2->GetInvInertia() : 0.0f;

        if (invMass1 == 0.0f && invMass2 == 0.0f) continue;

        RotMatrix rot1(obj1->transform.rotation);
        Vec2 r1 = rot1.Rotate(attachments[0].localAnchor);

        RotMatrix rot2(obj2->transform.rotation);
        Vec2 r2 = rot2.Rotate(attachments[i].localAnchor);

        Vec2 p1 = obj1->transform.position + r1;
        Vec2 p2 = obj2->transform.position + r2;

        Vec2 v1 = rb1 ? rb1->GetVelocity() : Vec2(0,0);
        float w1 = rb1 ? rb1->GetAngularVelocity() : 0.0f;

        Vec2 v2 = rb2 ? rb2->GetVelocity() : Vec2(0,0);
        float w2 = rb2 ? rb2->GetAngularVelocity() : 0.0f;

        Vec2 vp1(v1.x - w1 * r1.y, v1.y + w1 * r1.x);
        Vec2 vp2(v2.x - w2 * r2.y, v2.y + w2 * r2.x);
        Vec2 relative = vp2 - vp1;

        Vec2 distVector = p2 - p1;
        Vec2 biasVelocity = distVector * (Config::biasConstraint / dt);

        float k11 = invMass1 + invMass2 + (r1.y * r1.y * invInertia1) + (r2.y * r2.y * invInertia2);
        float k12 = -r1.x * r1.y * invInertia1 - r2.x * r2.y * invInertia2;
        float k21 = k12;
        float k22 = invMass1 + invMass2 + (r1.x * r1.x * invInertia1) + (r2.x * r2.x * invInertia2);

        Matrix2x2 K(k11, k12, k21, k22);
        Matrix2x2 K_inv = K.Inverse();

        Vec2 targetVelocity = relative + biasVelocity;
        Vec2 impulse = (K_inv * targetVelocity) * -1.0f;

        if (rb1)
        {
            rb1->SetVelocity(v1 - impulse * invMass1);
            rb1->SetAngularVelocity(w1 - (r1.Cross(impulse) * invInertia1));
        }

        if (rb2)
        {
            rb2->SetVelocity(v2 + impulse * invMass2);
            rb2->SetAngularVelocity(w2 + (r2.Cross(impulse) * invInertia2));
        }

        position = (p1 + p2) * 0.5f;
    }
}

void JointConstraint::ComplexJoint(float dt)
{
    Vec2 centerPos = Vec2();
    Vec2 centerVel = Vec2();
    float totalMass = 0.0f;

    for (JointAttachment& attachment : attachments) {
        GameObject* obj = attachment.obj;
        RigidBody* rb = obj->rb;

        if (!rb) continue;

        float mass = rb->GetMass(); 
        totalMass += mass;

        RotMatrix rot(obj->transform.rotation);
        Vec2 r = rot.Rotate(attachment.localAnchor);
        Vec2 p = obj->transform.position + r;

        Vec2 v = rb->GetVelocity();
        float w = rb->GetAngularVelocity();
        Vec2 anchorVel(v.x - w * r.y, v.y + w * r.x);

        centerPos += p * mass;
        centerVel += anchorVel * mass;
    }

    if (totalMass > 0.0f) {
        centerPos /= totalMass;
        centerVel /= totalMass;
    }
    
    this->position = centerPos; 

    for (JointAttachment& attachment : attachments) {
        GameObject* obj = attachment.obj;
        RigidBody* rb = obj->rb;
        if (!rb) continue;

        float invMass = rb->GetInvMass();
        float invInertia = rb->GetInvInertia();

        RotMatrix rot(obj->transform.rotation);
        Vec2 r = rot.Rotate(attachment.localAnchor);
        Vec2 p = obj->transform.position + r;

        Vec2 v = rb->GetVelocity();
        float w = rb->GetAngularVelocity();
        Vec2 anchorVel(v.x - w * r.y, v.y + w * r.x);

        Vec2 velError = anchorVel - centerVel;        
        Vec2 posError = p - centerPos;
        Vec2 biasVelocity = posError * (Config::biasConstraint / dt);

        Vec2 totalError = velError + biasVelocity;

        float k00 = invMass + (r.y * r.y * invInertia);
        float k01 = -r.x * r.y * invInertia;
        float k11 = invMass + (r.x * r.x * invInertia);

        Matrix2x2 K(k00, k01, k01, k11);
        Matrix2x2 iK = K.Inverse();

        Vec2 lambda = (iK * totalError) * -1.0f;

        rb->SetVelocity(rb->GetVelocity() + lambda * invMass);
        rb->SetAngularVelocity(rb->GetAngularVelocity() + r.Cross(lambda) * invInertia);
    }
}