#include "main/GameObject.h"

GameObject::GameObject()
{
}

GameObject::~GameObject()
{
}

void GameObject::SetRigidBody(std::unique_ptr<RigidBody> rb) 
{
    rigidBody = std::move(rb);    
}

void GameObject::SetRenderer(std::unique_ptr<Renderer> r)
{
    renderer = std::move(r);
}

void GameObject::SetCollider(std::unique_ptr<Collider> c)
{
    collider = std::move(c);
}