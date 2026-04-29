#pragma once
#include <algorithm>
#include <memory>
#include <vector>

#include "main/GameObject.h"
#include "main/components/Constraint.h"
#include "main/components/Controller.h"
#include "main/physics/pipeline/Broadphase.h"
#include "main/physics/pipeline/ContactManager.h"
#include "main/physics/pipeline/Integrate.h"
#include "main/utility/Timer.h"
#include "math/Vec2.h"

struct GeneratorDef {
    std::string name;
    int rows = 1;
    int columns = 1;
    float startX = 0;
    float startY = 0;
    float spacingX = 1.0f;
    float spacingY = 1.0f;
    std::string objectJson; 
    std::unique_ptr<GameObject> templateObject;
};

class World
{
    public:
        World();
        
        void Step(float dt);
        void UpdateCaches();
        void Clear();       

        void AddGameObject(std::unique_ptr<GameObject> obj);
        std::vector<std::unique_ptr<GameObject>>& GetGameObjects();

        void AddConstraint(std::unique_ptr<Constraint> c);
        const std::vector<std::unique_ptr<Constraint>>& GetConstraints() const { return constraints; }
        std::vector<Constraint*> GetConstraintsForObject(GameObject* obj);
        void RemoveConstraint(size_t id);

        void AddController(std::unique_ptr<Controller> c) { controllers.push_back(std::move(c)); }
        const std::vector<std::unique_ptr<Controller>>& GetControllers() const { return controllers; }

        const std::vector<ContactConstraint>& GetContacts() const { return contactManager->GetContacts(); }

        const std::unordered_map<unsigned int, std::vector<GameObject*>>& GetSpatialGrid() const { return broadphase->GetGridMap(); }

        void SetWorldSize(Vec2 size) { worldSize = size; }
        Vec2 GetWorldSize() const { return worldSize; }

        void RemoveGameObject(size_t id);
        void RemoveGroup(const std::string& groupName);

        void AddGenerator(GeneratorDef&& def) { generators.push_back(std::move(def)); }
        std::vector<GeneratorDef>& GetGenerators() { return generators; }
        GeneratorDef* GetGenerator(const std::string& name) {
            for (auto& g : generators) if (g.name == name) return &g;
            return nullptr;
        }
        void RemoveGenerator(const std::string& name) {
            generators.erase(std::remove_if(generators.begin(), generators.end(), [&](const GeneratorDef& g) {
                return g.name == name;
            }), generators.end());
        }

        void AddGroup(const std::string& name) { if (std::find(groups.begin(), groups.end(), name) == groups.end()) groups.push_back(name); }
        const std::vector<std::string>& GetGroups() const { return groups; }

        void RegenerateGenerator(const std::string& currentName, const std::string& oldName = "");
        void RenameGenerator(const std::string& oldName, const std::string& newName);

        void SetNextID(size_t id) { nextID = id; }
        void SetNextConstraintID(size_t id) { nextConstraintID = id; }

        //abstract stuff like this later ???
        bool isPaused = true;

    private:
        std::unique_ptr<Integrate> integrate;
        std::unique_ptr<Broadphase> broadphase;
        std::unique_ptr<ContactManager> contactManager;

        Vec2 worldSize = Vec2(32.0f, 18.0f);
        
        std::vector<std::unique_ptr<GameObject>> gameObjects;
        std::vector<std::unique_ptr<Constraint>> constraints;
        std::vector<std::unique_ptr<Controller>> controllers;
        std::vector<GeneratorDef> generators;
        std::vector<std::string> groups;

        void RemoveGroupInternal(const std::string& groupName, bool prune);

        size_t nextID = 0;
        size_t nextConstraintID = 0;

        //timing
        Timer timer;
    public:
        float integrateVelocityTime = 0.0f;
        float integratePositionTime = 0.0f;
        float broadphaseTime = 0.0f;
        float solverTime = 0.0f;
};
