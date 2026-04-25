#pragma once
#include "main/GameObject.h"
#include "main/World.h"
#include "raylib.h"
#include "main/editor/EditorCamera.h"


void Render(World& world, const EditorCamera& camera);
void GizmoRender(World& world, const EditorCamera& camera);
void GizmoUpdate(World& world, const EditorCamera& camera);