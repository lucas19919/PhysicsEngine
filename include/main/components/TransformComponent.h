#pragma once
#include "math/Vec2.h"
#include "main/components/Component.h"

class GameObject;

class TransformComponent : public Component
{
    public:
        TransformComponent();

        const char* GetName() const override { return "Transform"; }
        void OnInspectorGui() override;

        Vec2 position;
        float rotation;
        bool isDirty = true;

        void SetPosition(const Vec2& newPos) { position = newPos; isDirty = true; }
        void SetRotation(float newRot) { rotation = newRot; isDirty = true; }
};