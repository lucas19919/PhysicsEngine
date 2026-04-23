#pragma once

#include "main/GameObject.h"
#include "main/components/Constraint.h"
#include "main/components/Controller.h"

#include "main/physics/pipeline/Broadphase.h"
#include "main/physics/pipeline/Integrate.h"
#include "main/physics/pipeline/ContactManager.h"

#include <vector>
#include <memory>
#include "math/Vec2.h"

class World
{
    public:
        World();
        
        void Step(float dt);
        void Clear();       

        void AddGameObject(std::unique_ptr<GameObject> obj);
        std::vector<std::unique_ptr<GameObject>>& GetGameObjects();

        void AddConstraint(std::unique_ptr<Constraint> c);
        const std::vector<std::unique_ptr<Constraint>>& GetConstraints() const { return constraints; }

        void AddController(std::unique_ptr<Controller> c) { controllers.push_back(std::move(c)); }
        const std::vector<std::unique_ptr<Controller>>& GetControllers() const { return controllers; }

        void SetWorldSize(Vec2 size) { worldSize = size; }
        Vec2 GetWorldSize() const { return worldSize; }

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
};