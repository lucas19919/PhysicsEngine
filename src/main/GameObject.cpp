#include "main/GameObject.h"
#include <algorithm>

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

void GameObject::AddIgnored(int id)
{
    if (std::find(ignoredIDs.begin(), ignoredIDs.end(), id) == ignoredIDs.end())
    {
        ignoredIDs.push_back(id);
    }
}

void GameObject::RemoveIgnored(int id)
{
    auto it = std::find(ignoredIDs.begin(), ignoredIDs.end(), id);
    if (it != ignoredIDs.end())
    {
        ignoredIDs.erase(it);
    }
}