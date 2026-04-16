#pragma once
#include "components/TransformComponent.h"
#include "components/Component.h"
#include "utility/templates/Array.h"
#include <vector>
#include <memory>
#include <utility>

class GameObject
{
    public:
        GameObject();
        ~GameObject();

        TransformComponent transform;

        Array<20> cachedVertices;
        Array<20> cachedNormals;

        template <typename T>
        T* GetComponent() {
            for (auto& component : components)
            {
                T* target = dynamic_cast<T*>(component.get());
                if (target) return target;
            }
            return nullptr;
        }

        template <typename T>
        T& AddComponent(std::unique_ptr<T> component) {
            component->owner = this;
            T& ref = *component;
            components.push_back(std::move(component));
            return ref;
        }

        void SetID(size_t newID) { id = newID; }
        const size_t& GetID() const { return id; }

        void AddIgnored(int id);
        void RemoveIgnored(int id);
        const std::vector<int>& GetIgnoredIDs() const { return ignoredIDs; }
        
    private:
        std::vector<std::unique_ptr<Component>> components;

        size_t id;
        std::vector<int> ignoredIDs;
};