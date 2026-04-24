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