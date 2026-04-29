#pragma once
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "components/Collider.h"
#include "components/Component.h"
#include "components/RigidBody.h"
#include "components/TransformComponent.h"
#include "utility/templates/Array.h"

class GameObject
{
    public:
        GameObject();
        ~GameObject();

        TransformComponent transform;

        //cached common components
        RigidBody* rb = nullptr;
        Collider* c = nullptr;

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

            if constexpr (std::is_base_of_v<RigidBody, T>) {
                rb = static_cast<RigidBody*>(&ref);
            } else if constexpr (std::is_base_of_v<Collider, T>) {
                c = static_cast<Collider*>(&ref);
            }

            return ref;
        }

        void RemoveComponent(Component* component);
        void Scale(float sx, float sy);

        void SetID(size_t newID) { id = newID; }
        const size_t& GetID() const { return id; }

        void AddIgnored(size_t id);
        void RemoveIgnored(size_t id);
        const std::unordered_set<size_t>& GetIgnoredIDs() const { return ignoredIDs; }

        const std::vector<std::unique_ptr<Component>>& GetComponents() const { return components; }

        void SetName(const std::string& objName) { name = objName; }
        const std::string& GetName() const { return name; }

        void SetGroupName(const std::string& name) { groupName = name; }
        const std::string& GetGroupName() const { return groupName; }
        
    private:
        std::vector<std::unique_ptr<Component>> components;

        size_t id;
        std::string groupName;
        std::unordered_set<size_t> ignoredIDs;
        std::string name;
};