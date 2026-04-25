#include "main/scenes/BoundarySystem.h"
#include "main/utility/Instantiate.h"
#include "main/editor/EditorState.h"
#include "main/physics/Config.h"
#include <algorithm>

void BoundarySystem::UpdateBoundaries(World& world)
{
    auto& gameObjects = world.GetGameObjects();
    
    // Check if selected object is one of the walls and clear it if so
    GameObject* selected = EditorState::Get().GetSelected();
    
    // Remove existing internal walls
    auto it = std::remove_if(gameObjects.begin(), gameObjects.end(), [selected](const std::unique_ptr<GameObject>& obj) {
        bool isWall = obj->GetGroupName() == "INTERNAL_WALL";
        if (isWall && obj.get() == selected) {
            EditorState::Get().SetSelected(nullptr);
        }
        return isWall;
    });
    
    if (it != gameObjects.end()) {
        gameObjects.erase(it, gameObjects.end());
    }

    if (Config::useWalls)
    {
        Vec2 worldSize = world.GetWorldSize();
        float sw = worldSize.x;
        float sh = worldSize.y;
        float halfW = sw / 2.0f;
        float halfH = sh / 2.0f;
        float thickness = 4.0f;

        // Bottom
        GameObject* bottom = Instantiate()
            .WithTransform(Vec2(0.0f, halfH + thickness/2.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(sw, thickness))
            .Create(world, 9000);
        bottom->SetName("Bottom_Wall");
        bottom->SetGroupName("INTERNAL_WALL");

        // Top
        GameObject* top = Instantiate()
            .WithTransform(Vec2(0.0f, -halfH - thickness/2.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(sw, thickness))
            .Create(world, 9001);
        top->SetName("Top_Wall");
        top->SetGroupName("INTERNAL_WALL");

        // Left
        GameObject* left = Instantiate()
            .WithTransform(Vec2(-halfW - thickness/2.0f, 0.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(thickness, sh))
            .Create(world, 9002);
        left->SetName("Left_Wall");
        left->SetGroupName("INTERNAL_WALL");

        // Right
        GameObject* right = Instantiate()
            .WithTransform(Vec2(halfW + thickness/2.0f, 0.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(thickness, sh))
            .Create(world, 9003);
        right->SetName("Right_Wall");
        right->SetGroupName("INTERNAL_WALL");
    }
    
    world.UpdateCaches();
}
