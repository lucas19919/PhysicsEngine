#pragma once
#include "raylib.h"

class GameObject;

class Renderer
{
    public:
        ~Renderer() = default;
        Renderer(Color c);

        GameObject* parent;

        Color color = RED;
};