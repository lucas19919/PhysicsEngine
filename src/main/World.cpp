#include "main/World.h"

#include <algorithm>

#include "main/GameObject.h"
#include "main/editor/EditorState.h"
#include "main/physics/Config.h"
#include "main/scenes/LoadScene.h"
#include "math/Vec2.h"

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
    // If it has no ID or its ID is already taken, generate a new one
    auto isIdTaken = [&](size_t id) {
        for (const auto& existing : constraints) if (existing->GetID() == id) return true;
        return false;
    };

    if (c->GetID() == 0 || c->GetID() == (size_t)-1 || isIdTaken(c->GetID()))
    {
        while (isIdTaken(nextConstraintID)) nextConstraintID++;
        c->SetID(nextConstraintID++);
    }
    constraints.push_back(std::move(c));
}

std::vector<Constraint*> World::GetConstraintsForObject(GameObject* obj)
{
    std::vector<Constraint*> result;
    for (auto& c : constraints)
    {
        if (c->InvolvesObject(obj))
        {
            result.push_back(c.get());
        }
    }
    return result;
}

void World::RemoveConstraint(size_t id)
{
    constraints.erase(std::remove_if(constraints.begin(), constraints.end(), [id](const std::unique_ptr<Constraint>& c) {
        return c->GetID() == id;
    }), constraints.end());
}

void World::UpdateCaches()
{
    broadphase->UpdateBroadphase(gameObjects);
}

void World::Clear()
{
    EditorState::Get().ClearSelection();
    gameObjects.clear();
    constraints.clear();
    controllers.clear();  
    generators.clear();
    groups.clear();

    broadphase->Clear();
    contactManager->Clear();

    nextID = 0;
    nextConstraintID = 0;
}

void World::RemoveGameObject(size_t id)
{
    // 1. Notify all systems BEFORE deletion so they can safely check the object's data
    for (auto& c : constraints) c->OnObjectRemoved(id);
    for (auto& c : controllers) c->OnObjectRemoved(id);

    // 2. Remove the object itself
    auto it = std::remove_if(gameObjects.begin(), gameObjects.end(), [id](const std::unique_ptr<GameObject>& obj) {
        return obj->GetID() == id;
    });

    if (it == gameObjects.end()) return; // Object not found
    gameObjects.erase(it, gameObjects.end());

    // 3. Prune components that marked themselves as invalid
    constraints.erase(std::remove_if(constraints.begin(), constraints.end(), [](const std::unique_ptr<Constraint>& c) {
        return c->IsInvalid();
    }), constraints.end());

    controllers.erase(std::remove_if(controllers.begin(), controllers.end(), [](const std::unique_ptr<Controller>& c) {
        return c->IsInvalid();
    }), controllers.end());

    // 4. Clean up physics caches
    contactManager->Clear(); 
    UpdateCaches();
}

void World::RemoveGroup(const std::string& groupName)
{
    RemoveGroupInternal(groupName, true);
    RemoveGenerator(groupName);
    groups.erase(std::remove(groups.begin(), groups.end(), groupName), groups.end());
}

void World::RemoveGroupInternal(const std::string& groupName, bool prune)
{
    if (groupName.empty()) return;

    std::vector<size_t> idsToRemove;
    for (const auto& obj : gameObjects) {
        if (obj->GetGroupName() == groupName) {
            idsToRemove.push_back(obj->GetID());
        }
    }

    if (idsToRemove.empty()) return;

    // Notify for all IDs
    for (size_t id : idsToRemove) {
        for (auto& c : constraints) c->OnObjectRemoved(id);
        for (auto& c : controllers) c->OnObjectRemoved(id);
    }

    // Remove objects
    gameObjects.erase(std::remove_if(gameObjects.begin(), gameObjects.end(), [&](const std::unique_ptr<GameObject>& obj) {
        return obj->GetGroupName() == groupName;
    }), gameObjects.end());

    if (prune) {
        // Prune invalid components
        constraints.erase(std::remove_if(constraints.begin(), constraints.end(), [](const std::unique_ptr<Constraint>& c) {
            return c->IsInvalid();
        }), constraints.end());

        controllers.erase(std::remove_if(controllers.begin(), controllers.end(), [](const std::unique_ptr<Controller>& c) {
            return c->IsInvalid();
        }), controllers.end());
    }

    contactManager->Clear();
    UpdateCaches();
}


void World::RegenerateGenerator(const std::string& currentName, const std::string& oldName)
{
    GeneratorDef* def = GetGenerator(currentName);
    if (!def) return;

    // 1. Remove old objects WITHOUT pruning constraints
    // If we are renaming, we MUST clear the OLD group name.
    // If not renaming, currentName == oldName is handled by the default param or logic below.
    std::string nameToClear = oldName.empty() ? currentName : oldName;
    RemoveGroupInternal(nameToClear, false);

    // 2. Delegate creation to LoadScene (which already has the logic)
    LoadScene::Regenerate(*this, *def);

    // 3. Final Re-validation
    // Now that objects are back, reset the "deleted" flags for any surviving components
    for (auto& c : constraints) c->ResetInvalid();
    for (auto& c : controllers) c->ResetInvalid();

    // 4. Prune anything that is STILL invalid (actually dead)
    constraints.erase(std::remove_if(constraints.begin(), constraints.end(), [](const std::unique_ptr<Constraint>& c) {
        return c->IsInvalid();
    }), constraints.end());

    controllers.erase(std::remove_if(controllers.begin(), controllers.end(), [](const std::unique_ptr<Controller>& c) {
        return c->IsInvalid();
    }), controllers.end());

    contactManager->Clear();
    UpdateCaches();
}

void World::RenameGenerator(const std::string& oldName, const std::string& newName)
{
    if (oldName == newName || newName.empty()) return;

    // 1. Update the definition name
    GeneratorDef* def = GetGenerator(oldName);
    if (def) {
        def->name = newName;
    }

    // 2. Update all instantiated gameobjects
    for (auto& obj : gameObjects) {
        if (obj->GetGroupName() == oldName) {
            obj->SetGroupName(newName);
            // Optionally update the individual object name if it followed the pattern
            std::string objName = obj->GetName();
            if (objName.find(oldName) == 0) {
                obj->SetName(newName + objName.substr(oldName.length()));
            }
        }
    }
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
