#include "main/physics/pipeline/ContactManager.h"
#include "main/physics/ManifoldHandler.h"
#include "main/physics/Solver.h"

void ContactManager::PrepareFrame()
{
    currentFrameContacts.clear();
}

void ContactManager::FinishFrame()
{
    std::swap(lastFrameContacts, currentFrameContacts);
}

void ContactManager::Clear()
{
    currentFrameContacts.clear(); 
    lastFrameContacts.clear(); 
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

        RigidBody* rb1 = obj1->GetRigidBody();
        RigidBody* rb2 = obj2->GetRigidBody();


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

        for (int i = 0; i < contactConstraint.pointCount; i++)
        {
            Vec2 p = contactConstraint.points[i];
            Vec2 r1 = p - obj1->transform.position;
            Vec2 r2 = p - obj2->transform.position;

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
        unsigned int key = contact.key;
        for (auto& lastContact : lastFrameContacts)
        {
            if (lastContact.key != key) continue;

            for (int i = 0; i < contact.pointCount; i++)
            {
                contact.accumulatedNormalImpulse[i] = lastContact.accumulatedNormalImpulse[i];
                contact.accumulatedTangentImpulse[i] = lastContact.accumulatedTangentImpulse[i];
            }

            if (Config::warmStart)
                Solver::Warmstart(contact);

            break;
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