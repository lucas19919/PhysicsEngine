#pragma once
#include <cstddef>

class GameObject;

class Component {
    public:
        GameObject* owner = nullptr; 
        virtual ~Component() = default;

        virtual const char* GetName() const = 0;
        virtual bool OnInspectorGui(class World* world = nullptr) { return false; }

        virtual void OnObjectRemoved(size_t id);
        virtual bool IsInvalid() const { return isComponentDeleted; }
        void ResetInvalid() { isComponentDeleted = false; }

    protected:
        bool isComponentDeleted = false;
};

