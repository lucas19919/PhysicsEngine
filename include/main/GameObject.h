#pragma once
#include "components/TransformComponent.h"
#include "components/RigidBody.h"
#include "components/Renderer.h"
#include "components/Collider.h"
#include <memory>

class GameObject
{
    public:
        GameObject();
        ~GameObject();

        TransformComponent transform;

        RigidBody* GetRigidBody() { return rigidBody.get(); }
        Collider* GetCollider() { return collider.get(); }
        Renderer* GetRenderer() { return renderer.get(); }

        void SetRigidBody(std::unique_ptr<RigidBody> rb);
        void SetCollider(std::unique_ptr<Collider> c);
        void SetRenderer(std::unique_ptr<Renderer> r);

        int GetID() const { return id; }

    private:
        std::unique_ptr<RigidBody> rigidBody;
        std::unique_ptr<Collider> collider;
        std::unique_ptr<Renderer> renderer;

        int id;
        int i = 1;
};