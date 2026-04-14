#include "main/World.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/physics/Config.h"


World::World()
{ 
    integrate = std::make_unique<Integrate>();
    broadphase = std::make_unique<Broadphase>();
    contactManager = std::make_unique<ContactManager>();
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
    constraints.clear();
    controllers.clear();  

    integrate.reset();
    broadphase->Clear();
    contactManager->Clear();
}

//step world forward by dt seconds
void World::Step(float dt)
{
    if (isPaused) return;

    int subTicks = Config::pipelineSubTicks;
    float subDt = dt / subTicks;
    for (int i = 0; i < subTicks; i++)
    {
        //frame subtick begin
        contactManager->PrepareFrame();

        integrate->IntegrateVelocity(gameObjects, subDt);
        broadphase->UpdateBroadphase(gameObjects); 
        contactManager->BuildContacts(broadphase->GeneratePairs());
        contactManager->PrepareContacts(subDt);
        contactManager->SolveConstraints(constraints, subDt);
        integrate->IntegratePosition(gameObjects, subDt);

        contactManager->FinishFrame();  
        //end subtick frame      
    }
}