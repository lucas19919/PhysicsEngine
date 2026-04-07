#include "main/World.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/physics/ManifoldHandler.h"
#include "main/physics/Solver.h"
#include "main/physics/Config.h"
#include <cstdint>

World::World() : spatialHash(Config().spatialHashCellSize)
{
}

std::vector<std::unique_ptr<GameObject>>& World::GetGameObjects()
{
    return gameObjects;
}

void World::AddGameObject(std::unique_ptr<GameObject> obj)
{
    gameObjects.push_back(std::move(obj));
}

void World::AddConstraint(std::unique_ptr<Constraint> c)
{
    constraints.push_back(std::move(c));
}

void World::Clear()
{
    gameObjects.clear();
    currentFrameContacts.clear();
    lastFrameContacts.clear();
    gridMap.clear();
    candidatePairs.clear();
    candidatePairKeys.clear();
    constraints.clear();
}

//step world forward by dt seconds
void World::Step(float dt)
{
    if (isPaused) return;

    PrepareFrame(dt);
    IntegrateVelocities(dt);

    UpdateBroadphase();
    GeneratePairs();

    BuildContacts();
    PrepareContacts();
    SolveConstraints(dt);

    IntegratePositions(dt);

    UpdateSleep(dt);
    FinishFrame(dt);
}

void World::PrepareFrame(float dt)
{
    currentFrameContacts.clear();
    candidatePairs.clear();
    candidatePairKeys.clear();

    for (auto& cell : gridMap)
    {
        cell.second.clear();
    }
}

void World::IntegrateVelocities(float dt)
{
    //loop through all gameobjects and update physics and positions
    for (const auto& objPtr : gameObjects)
    {
        GameObject* obj = objPtr.get();
        RigidBody* rb = obj->GetRigidBody();   

        if (!rb) continue;
        if (rb->IsSleeping()) continue;

        float mass = rb->GetMass();
        float invMass = rb->GetInvMass();
        float invInertia = rb->GetInvInertia();

        //linear motion
        if (rb->IsGravityEnabled())
            rb->ApplyForce(Config().gravity * mass);

        rb->SetAcceleration(rb->GetForce() * invMass);
        rb->SetVelocity(rb->GetVelocity() + rb->GetAcceleration() * dt);

        //angular motion
        rb->SetAngularAcceleration(rb->GetTorque() * invInertia);
        rb->SetAngularVelocity(rb->GetAngularVelocity() + rb->GetAngularAcceleration() * dt);

        rb->ClearForces();
        rb->ClearTorque();
    }
}

//update bounds and spatial hash grid
void World::UpdateBroadphase()
{
    for (const auto& objPtr : gameObjects)
    {
        GameObject* obj = objPtr.get();
        RigidBody* rb = obj->GetRigidBody();
        Collider* c = obj->GetCollider();

        if (!c) continue;

        //update cached vertices/normals and bounds for spatial hash
        if (!rb || !rb->IsSleeping())
        {
            obj->cachedVertices = SAT::GetVertices(obj);
            obj->cachedNormals = SAT::GetNormals(obj->cachedVertices);
            c->SetBounds(spatialHash.GetBounding(obj));
        }

        //determine which cells the object occupies and add to grid map
        int minX = std::floor(c->GetBounds().min.x / spatialHash.GetCellSize());
        int maxX = std::floor(c->GetBounds().max.x / spatialHash.GetCellSize());
        int minY = std::floor(c->GetBounds().min.y / spatialHash.GetCellSize());
        int maxY = std::floor(c->GetBounds().max.y / spatialHash.GetCellSize());

        //loop through occupied cells and add object to grid map
        for (int x = minX; x <= maxX; x++)
        {
            for (int y = minY; y <= maxY; y++)
            {
                unsigned int hash = spatialHash.GetHash(Vec2(x * spatialHash.GetCellSize(), y * spatialHash.GetCellSize()));
                gridMap[hash].push_back(obj);
            }
        }
    }
}

//generate candidate pairs from broadphase grid map
void World::GeneratePairs()
{
    //for each cell, generate pairs if they overlap bounds
    for (const auto& pair : gridMap)
    {
        const std::vector<GameObject*>& cell = pair.second;
        if (cell.size() < 2) continue; //need at least 2 objects to form a pair

        for (size_t i = 0; i < cell.size(); i++)
        {
            for (size_t j = i + 1; j < cell.size(); j++)
            {
                GameObject* obj1 = cell[i];
                GameObject* obj2 = cell[j];

                RigidBody* rb1 = obj1->GetRigidBody();
                RigidBody* rb2 = obj2->GetRigidBody();

                //check if bounds overlap before adding pair
                if (SAT::TestBounds(obj1, obj2))
                {
                    if (obj1 > obj2) std::swap(obj1, obj2); 

                    uint64_t key = (uint64_t)(uintptr_t)obj1 ^ ((uint64_t)(uintptr_t)obj2 << 32);
                    if (!candidatePairKeys.insert(key).second) continue;

                    candidatePairs.emplace_back(obj1, obj2);
                }
            }
        }
    }
}

//fill contacts
void World::BuildContacts()
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

        bool rb1Sleeping = rb1 && rb1->IsSleeping();
        bool rb2Sleeping = rb2 && rb2->IsSleeping();

        if (rb1Sleeping && rb2Sleeping) continue;

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

            float restitutionThreshold = Config().restitutionThreshold;
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

//warmstart
void World::PrepareContacts()
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

            if (Config().warmStart)
                Solver::Warmstart(contact);

            break;
        }
    }
}

//solver iterations
void World::SolveConstraints(float dt)
{
    for (int i = 0; i < Config().impulseIterations; i++)
    {
        for (auto& contact : currentFrameContacts)
        {
            Solver::ResolveConstraints(contact);
        }

        for (auto& constraint : constraints)
        {
            constraint->Solve(dt);
        }
    }

    for (int i = 0; i < Config().positionIterations; i++)
    {
        for (auto& contact : currentFrameContacts)
        {
            Solver::ResolvePosition(contact);
        }
    }
}

void World::IntegratePositions(float dt)
{
    for (const auto& objPtr : gameObjects)
    {
        GameObject* obj = objPtr.get();
        RigidBody* rb = obj->GetRigidBody();   

        if (!rb) continue;
        if (rb->IsSleeping()) continue;

        obj->transform.position += rb->GetVelocity() * dt;
        obj->transform.rotation += rb->GetAngularVelocity() * dt;
    }
}

void World::UpdateSleep(float dt)
{
    sleepCounter = 0;
    for (const auto& objPtr : gameObjects)
    {
        GameObject* obj = objPtr.get();
        RigidBody* rb = obj->GetRigidBody();

        if (!rb) continue;

        float m = rb->GetMass();
        float i = rb->GetInertia();

        float vSq = rb->GetVelocity().MagSq();
        float aV = rb->GetAngularVelocity();

        float kE = 0.5 * ((m * vSq) + (i * aV * aV));

        if (kE <= Config().energyThreshold)
        {
            rb->SetSleepTimer(rb->GetSleepTimer() + dt);
            if (rb->GetSleepTimer() >= Config().sleepTime)
            {
                rb->SetSleeping(true);
                sleepCounter++;
            }     
        }
        else
        {
            rb->SetSleepTimer(0.0f);
            rb->SetSleeping(false);
        }

         //DEBUG:
        if (Config().debugSleep)
        {
            if (rb->IsSleeping())
                obj->GetRenderer()->SetColor({ 100, 100, 255, 255 });
            else
                obj->GetRenderer()->SetColor({ 255, 100, 100, 255 });
        }
    }
}

//cleanup caches
void World::FinishFrame(float dt)
{
    //cleanup
    std::swap(lastFrameContacts, currentFrameContacts);
}