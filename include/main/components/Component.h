#pragma once

class GameObject;

class Component {
    public:
        GameObject* owner = nullptr; 
        virtual ~Component() = default;
};