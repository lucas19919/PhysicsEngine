#include "main/GameObject.h"
#include <algorithm>

GameObject::GameObject() : id((size_t)-1)
{
}

GameObject::~GameObject()
{
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

void GameObject::RemoveComponent(Component* component)
{
    auto it = std::remove_if(components.begin(), components.end(), [component](const std::unique_ptr<Component>& cPtr) {
        return cPtr.get() == component;
    });

    if (it != components.end())
    {
        // Null out cached pointers if they pointed to the removed component
        if (component == rb) rb = nullptr;
        if (component == c) c = nullptr;

        components.erase(it, components.end());
    }
}