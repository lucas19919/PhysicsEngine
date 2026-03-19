#pragma once
#include "components/TransformComponent.h"
#include "components/RigidBody.h"
#include "components/Renderer.h"
#include "components/Collider.h"

class GameObject
{
    public:
        GameObject();
        ~GameObject();

        TransformComponent transform;

        RigidBody* GetRigidBody() { return rigidBody; }
        Renderer* GetRenderer() { return renderer; }
        Collider* GetCollider() { return collider; }

        void SetRigidBody(RigidBody* rb);
        void SetRenderer(Renderer* r);
        void SetCollider(Collider* c);

    private:
        RigidBody* rigidBody;
        Collider* collider;
        Renderer* renderer;
};