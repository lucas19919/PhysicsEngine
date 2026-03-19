#include "main/GameObject.h"

GameObject::GameObject() : transform(this)
{
    rigidBody = nullptr;
    collider = nullptr;
    renderer = nullptr;
}

GameObject::~GameObject()
{
    if (rigidBody) delete rigidBody;
    if (collider) delete collider;
    if (renderer) delete renderer;
}

void GameObject::SetRigidBody(RigidBody* rb) 
{
    if (rigidBody != nullptr) delete rigidBody; 
    
    rigidBody = rb; 
    
    if (rigidBody != nullptr) {
        rigidBody->parent = this; 
    }
}

void GameObject::SetRenderer(Renderer* r)
{
    if (renderer != nullptr) delete renderer;
    renderer = r;
    
    if (renderer != nullptr) {
        renderer->parent = this;
    }
}

void GameObject::SetCollider(Collider* c)
{
    if (collider != nullptr) delete collider;
    collider = c;
    
    if (collider != nullptr) {
        collider->parent = this;
    }
}