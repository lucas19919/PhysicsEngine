#include "main/physics/pipeline/ContactManager.h"
#include "main/physics/ManifoldHandler.h"
#include "main/physics/Solver.h"

void ContactManager::PrepareFrame()
{
    currentFrameContacts.clear();
}

void ContactManager::FinishFrame()
{
    impulseCache.clear();
    for (const auto& contact : currentFrameContacts)
    {
        ImpulseCache cache;
        cache.pointCount = contact.pointCount;
        for (int i = 0; i < contact.pointCount; i++)
        {
            cache.normalImpulse[i] = contact.accumulatedNormalImpulse[i];
            cache.tangentImpulse[i] = contact.accumulatedTangentImpulse[i];
        }
        impulseCache[contact.key] = cache;
    }
}

void ContactManager::Clear()
{
    currentFrameContacts.clear(); 
    lastFrameContacts.clear(); 
    impulseCache.clear();
}

void ContactManager::BuildContacts(std::vector<std::pair<GameObject*, GameObject*>> candidatePairs)
{
    for (const auto& pair : candidatePairs)
    {
        GameObject* obj1 = pair.first;
        GameObject* obj2 = pair.second;

        CollisionManifold cm = Solver::ResolveManifold(obj1, obj2);
        
        if (!cm.collision.isColliding) continue;
        ContactConstraint contactConstraint{};  

        contactConstraint.obj1 = obj1;
        contactConstraint.obj2 = obj2;

        uintptr_t a = reinterpret_cast<uintptr_t>(obj1);
        uintptr_t b = reinterpret_cast<uintptr_t>(obj2);
        contactConstraint.key = static_cast<unsigned int>(a * 73856093u ^ b * 19349669u);

        RigidBody* rb1 = obj1->rb;
        RigidBody* rb2 = obj2->rb;


        contactConstraint.rb1 = rb1;
        contactConstraint.rb2 = rb2;

        contactConstraint.normal = cm.collision.normal;
        contactConstraint.penetration = cm.collision.depth;

        if (rb1 && rb2)
        {
            contactConstraint.restitution = std::max(rb1->GetRestitution(), rb2->GetRestitution());
            contactConstraint.friction = std::sqrt(rb1->GetFriction() * rb2->GetFriction());
        }
        else if (rb1)
        {
            contactConstraint.restitution = rb1->GetRestitution();
            contactConstraint.friction = rb1->GetFriction();
        }
        else if (rb2)
        {
            contactConstraint.restitution = rb2->GetRestitution();
            contactConstraint.friction = rb2->GetFriction();
        }
        else 
        {
            contactConstraint.restitution = 0.0f;
            contactConstraint.friction = 0.0f;
        }

        contactConstraint.pointCount = cm.points.Size();
        for (int i = 0; i < contactConstraint.pointCount; i++)
        {
            contactConstraint.points[i] = cm.points[i];
        }

        float invMass1 = rb1 ? rb1->GetInvMass() : 0.0f;
        float invMass2 = rb2 ? rb2->GetInvMass() : 0.0f;
        float invInertia1 = rb1 ? rb1->GetInvInertia() : 0.0f;
        float invInertia2 = rb2 ? rb2->GetInvInertia() : 0.0f;

        Vec2 normal = contactConstraint.normal;
        Vec2 tangent = Vec2(-normal.y, normal.x);

        for (int i = 0; i < contactConstraint.pointCount; i++)
        {
            Vec2 p = contactConstraint.points[i];
            Vec2 r1 = p - obj1->transform.position;
            Vec2 r2 = p - obj2->transform.position;
            contactConstraint.r1[i] = r1;
            contactConstraint.r2[i] = r2;

            //normal mass
            float rn1 = r1.Cross(normal);
            float rn2 = r2.Cross(normal);
            float kNormal = invMass1 + invMass2 + (rn1 * rn1 * invInertia1) + (rn2 * rn2 * invInertia2);
            contactConstraint.normalMass[i] = kNormal > 0.0f ? 1.0f / kNormal : 0.0f;

            //tangent mass
            float rt1 = r1.Cross(tangent);
            float rt2 = r2.Cross(tangent);
            float kTangent = invMass1 + invMass2 + (rt1 * rt1 * invInertia1) + (rt2 * rt2 * invInertia2);
            contactConstraint.tangentMass[i] = kTangent > 0.0f ? 1.0f / kTangent : 0.0f;

            Vec2 v1 = rb1 ? rb1->GetVelocity() + Vec2(-rb1->GetAngularVelocity() * r1.y, rb1->GetAngularVelocity() * r1.x) : Vec2(0,0);
            Vec2 v2 = rb2 ? rb2->GetVelocity() + Vec2(-rb2->GetAngularVelocity() * r2.y, rb2->GetAngularVelocity() * r2.x) : Vec2(0,0);
            
            float relativeVelocity = contactConstraint.normal.Dot(v2 - v1);

            float restitutionThreshold = Config::restitutionThreshold;
            if (relativeVelocity < -restitutionThreshold)
            {
                contactConstraint.restitutionBias[i] = -contactConstraint.restitution * relativeVelocity;
            }
            else
            {
                contactConstraint.restitutionBias[i] = 0.0f;
            }
        }

        currentFrameContacts.push_back(contactConstraint);
    }
}

void ContactManager::PrepareContacts(float dt)
{
    for (auto& contact : currentFrameContacts)
    {
        auto it = impulseCache.find(contact.key);
        if (it != impulseCache.end())
        {
            const ImpulseCache& cache = it->second;
            int count = std::min(contact.pointCount, cache.pointCount);
            for (int i = 0; i < count; i++)
            {
                contact.accumulatedNormalImpulse[i] = cache.normalImpulse[i];
                contact.accumulatedTangentImpulse[i] = cache.tangentImpulse[i];
            }

            if (Config::warmStart)
                Solver::Warmstart(contact);
        }
    }
}

void ContactManager::SolveConstraints(const std::vector<std::unique_ptr<Constraint>>& constraints, float dt)
{
    for (int i = 0; i < Config::impulseIterations; i++)
    {
        for (auto& contact : currentFrameContacts)
        {
            Solver::ResolveConstraints(contact);
        }

        for (auto& constraint : constraints)
        {
            constraint->Solve(dt / (float)Config::impulseIterations);
        }
    }

    for (int i = 0; i < Config::positionIterations; i++)
    {
        for (auto& contact : currentFrameContacts)
        {
            Solver::ResolvePosition(contact);
        }
    }   
}