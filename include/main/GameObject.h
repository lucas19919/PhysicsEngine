#pragma once
#include "components/TransformComponent.h"
#include "components/RigidBody.h"
#include "components/Renderer.h"
#include "components/Collider.h"
#include <vector>
#include <memory>

class GameObject
{
    public:
        GameObject();
        ~GameObject();

        void SetID(size_t newID) { id = newID; }
        const size_t& GetID() const { return id; }

        Array<20> cachedVertices;
        Array<20> cachedNormals;

        TransformComponent transform;

        RigidBody* GetRigidBody() { return rigidBody.get(); }
        Collider* GetCollider() { return collider.get(); }
        Renderer* GetRenderer() { return renderer.get(); }

        void SetRigidBody(std::unique_ptr<RigidBody> rb);
        void SetCollider(std::unique_ptr<Collider> c);
        void SetRenderer(std::unique_ptr<Renderer> r);

        void AddIgnored(int id);
        void RemoveIgnored(int id);
        const std::vector<int>& GetIgnoredIDs() const { return ignoredIDs; }
        
    private:
        std::unique_ptr<RigidBody> rigidBody;
        std::unique_ptr<Collider> collider;
        std::unique_ptr<Renderer> renderer;

        size_t id;

        std::vector<int> ignoredIDs;
};