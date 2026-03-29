#include "main/GameObject.h"

GameObject::GameObject() : transform(this)
{
}

GameObject::~GameObject()
{
}

void GameObject::SetRigidBody(std::unique_ptr<RigidBody> rb) 
{
    rigidBody = std::move(rb);    
    
    if (rigidBody != nullptr) {
        rigidBody->parent = this; 
    }
}

void GameObject::SetRenderer(std::unique_ptr<Renderer> r)
{
    renderer = std::move(r);
    
    if (renderer != nullptr) {
        renderer->parent = this;
    }
}

void GameObject::SetCollider(std::unique_ptr<Collider> c)
{
    collider = std::move(c);
    
    if (collider != nullptr) {
        collider->parent = this;
    }
}