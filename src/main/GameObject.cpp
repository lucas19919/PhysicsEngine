#include "main/GameObject.h"
#include <algorithm>

GameObject::GameObject() : id((size_t)-1)
{
}

GameObject::~GameObject()
{
}


void GameObject::AddIgnored(size_t id)
{
    ignoredIDs.insert(id);
}

void GameObject::RemoveIgnored(size_t id)
{
    ignoredIDs.erase(id);
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