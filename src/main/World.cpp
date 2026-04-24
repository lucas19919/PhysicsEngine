#include "main/World.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/physics/Config.h"

World::World()
{ 
    integrate = std::make_unique<Integrate>();
    broadphase = std::make_unique<Broadphase>();
    contactManager = std::make_unique<ContactManager>();
    timer = Timer();
}

std::vector<std::unique_ptr<GameObject>>& World::GetGameObjects()
{
    return gameObjects;
}

void World::AddGameObject(std::unique_ptr<GameObject> obj)
{
    if (obj->GetID() == (size_t)-1)
    {
        obj->SetID(nextID++);
    }
    gameObjects.push_back(std::move(obj));
}

void World::AddConstraint(std::unique_ptr<Constraint> c)
{
    constraints.push_back(std::move(c));
}

void World::UpdateCaches()
{
    broadphase->UpdateBroadphase(gameObjects);
}

void World::Clear()
{
    gameObjects.clear();
    constraints.clear();
    controllers.clear();  

    integrate.reset();
    broadphase->Clear();
    contactManager->Clear();

    nextID = 0;

    timer.~Timer();
}

//step world forward by dt seconds
void World::Step(float dt)
{
    if (isPaused) return;

    int subTicks = Config::pipelineSubTicks;
    float subDt = dt / subTicks;

    integrateVelocityTime = 0.0f;
    broadphaseTime = 0.0f;
    solverTime = 0.0f;
    integratePositionTime = 0.0f;

    for (int i = 0; i < subTicks; i++)
    {
        //frame subtick begin
        contactManager->PrepareFrame();

        timer.StartTimer();
        integrate->IntegrateVelocity(gameObjects, subDt);
        integrateVelocityTime += timer.StopTimer();

        timer.StartTimer();
        broadphase->UpdateBroadphase(gameObjects); 
        contactManager->BuildContacts(broadphase->GeneratePairs());
        broadphaseTime += timer.StopTimer();

        timer.StartTimer();
        contactManager->PrepareContacts(subDt);
        contactManager->SolveConstraints(constraints, subDt);
        solverTime += timer.StopTimer();

        timer.StartTimer();
        integrate->IntegratePosition(gameObjects, subDt);
        integratePositionTime += timer.StopTimer();

        contactManager->FinishFrame();  
        //end subtick frame      
    }
}