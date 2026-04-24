#pragma once

class GameObject;

class Component {
    public:
        GameObject* owner = nullptr; 
        virtual ~Component() = default;

        virtual const char* GetName() const = 0;
        virtual void OnInspectorGui() {}
};