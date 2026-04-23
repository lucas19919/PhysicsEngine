#pragma once
#include "main/GameObject.h"

class EditorState {
public:
    static EditorState& Get() {
        static EditorState instance;
        return instance;
    }

    void SetSelected(GameObject* obj) { selectedObject = obj; }
    GameObject* GetSelected() const { return selectedObject; }

private:
    EditorState() : selectedObject(nullptr) {}
    GameObject* selectedObject;
};
