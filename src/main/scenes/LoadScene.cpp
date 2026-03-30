#include "main/scenes/LoadScene.h"
#include "main/utility/Instantiate.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <libraries/json/json.hpp>

using json = nlohmann::json;

void LoadScene::Load(const std::string& filePath, World& world, int screenWidth, int screenHeight)
{
    world.Clear();

    std::ifstream file(filePath);
    if (!file.is_open()) return;

    json sceneData;
    file >> sceneData;

    if (sceneData.value("useWalls", false))
    {
        Instantiate()
            .WithTransform(Vec2(screenWidth / 2.0f, screenHeight + 25.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(screenWidth, 50.0f))
            .Create(world);

        Instantiate()
            .WithTransform(Vec2(screenWidth / 2.0f, -25.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(screenWidth, 50.0f))
            .Create(world);

        Instantiate()
            .WithTransform(Vec2(-25.0f, screenHeight / 2.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(50.0f, screenHeight))
            .Create(world);

        Instantiate()
            .WithTransform(Vec2(screenWidth + 25.0f, screenHeight / 2.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(50.0f, screenHeight))
            .Create(world);
    }

    for (const auto& item : sceneData["objects"])
    {
        LoadObject(item, world);
    }

    if (sceneData.contains("generators"))
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> jitter(-4.0f, 4.0f);

        for (const auto& generator : sceneData["generators"])
        {
            if (!generator.contains("grid") || !generator.contains("object")) continue;

            int rows = generator["grid"]["rows"];
            int cols = generator["grid"]["columns"];
            float startX = generator["grid"]["startX"];
            float startY = generator["grid"]["startY"];
            float spacingX = generator["grid"]["spacingX"];
            float spacingY = generator["grid"]["spacingY"];

            json genObject = generator["object"];

            for (int r = 0; r < rows; r++)
            {
                for (int c = 0; c < cols; c++)
                {
                    float posX = startX + (c * spacingX) + jitter(gen);
                    float posY = startY + (r * spacingY) + jitter(gen);

                    genObject["components"]["TransformComponent"]["position"]["x"] = posX;
                    genObject["components"]["TransformComponent"]["position"]["y"] = posY;

                    LoadObject(genObject, world);
                }
            }
        }
    }
}

void LoadScene::LoadObject(const json& item, World& world)
{
    if (!item.contains("components")) return;
    const auto& components = item["components"];

    float posX = 0.0f;
    float posY = 0.0f;
    float rotation = 0.0f;

    if (components.contains("TransformComponent"))
    {
        posX = components["TransformComponent"]["position"]["x"];
        posY = components["TransformComponent"]["position"]["y"];
        rotation = components["TransformComponent"]["rotation"];
    }

    ColliderType colType = ColliderType::BOX;
    Vec2 boxSize;
    float circleRadius = 0.0f;
    Array<20> polyVerts;
    
    if (components.contains("Collider"))
    {
        std::string type = components["Collider"]["type"];
        if (type == "BOX")
        {
            colType = ColliderType::BOX;
            boxSize.x = components["Collider"]["size"]["x"];
            boxSize.y = components["Collider"]["size"]["y"];
        }
        else if (type == "CIRCLE")
        {
            colType = ColliderType::CIRCLE;
            circleRadius = components["Collider"]["radius"];
        }
        else if (type == "POLYGON")
        {
            colType = ColliderType::POLYGON;
            for (const auto& v : components["Collider"]["vertices"])
            {
                polyVerts.push_back(Vec2(v["x"], v["y"]));
            }
        }
    }

    Shape renderShape;
    if (components.contains("Renderer"))
    {
        std::string form = components["Renderer"]["form"];
        std::string colorStr = components["Renderer"]["color"];

        Color col = GRAY;
        if (colorStr == "RED") col = RED;
        else if (colorStr == "GREEN") col = GREEN;
        else if (colorStr == "BLUE") col = BLUE;

        renderShape.color = col;

        if (form == "R_BOX")
        {
            renderShape.form = RenderShape::R_BOX;
            Vec2 size = Vec2(components["Renderer"]["scale"]["x"], components["Renderer"]["scale"]["y"]);
            renderShape.scale = size;
        }
        else if (form == "R_CIRCLE")
        {
            renderShape.form = RenderShape::R_CIRCLE;
            renderShape.scale = components["Renderer"]["scale"];
        }
        else if (form == "R_POLYGON")
        {
            renderShape.form = RenderShape::R_POLYGON;
            Array<20> vertices;

            for (const auto& v : components["Renderer"]["scale"])
            {
                vertices.push_back(Vec2(v["x"], v["y"]));
            }

            renderShape.scale = vertices;
        }
    }

    Instantiate builder = Instantiate().WithTransform(Vec2(posX, posY), rotation);

    if (components.contains("Collider"))
    {
        if (colType == ColliderType::BOX)
        {
            builder.WithCollider(colType, boxSize);
        }
        else if (colType == ColliderType::CIRCLE)
        {
            builder.WithCollider(colType, circleRadius);
        }
        else if (colType == ColliderType::POLYGON)
        {
            builder.WithCollider(colType, polyVerts);
        }
    }

    if (components.contains("Renderer"))
    {
        builder.WithRenderer(renderShape);
    }

    if (components.contains("RigidBody"))
    {
        Properties props = { 1.0f, 0.5f, 100.0f, 1.0f };
        LinearState linear = { Vec2(), Vec2(), Vec2() };
        AngularState angular = { 0.0f, 0.0f, 0.0f };

        props.mass = components["RigidBody"]["properties"]["mass"];
        props.restitution = components["RigidBody"]["properties"]["restitution"];
        props.friction = components["RigidBody"]["properties"]["friction"];
        props.inertia = components["RigidBody"]["properties"]["inertia"];

        linear.velocity.x = components["RigidBody"]["linearState"]["velocity"]["x"];
        linear.velocity.y = components["RigidBody"]["linearState"]["velocity"]["y"];
        linear.acceleration.x = components["RigidBody"]["linearState"]["acceleration"]["x"];
        linear.acceleration.y = components["RigidBody"]["linearState"]["acceleration"]["y"];
        linear.netForce.x = components["RigidBody"]["linearState"]["netForce"]["x"];
        linear.netForce.y = components["RigidBody"]["linearState"]["netForce"]["y"];

        angular.angularVelocity = components["RigidBody"]["angularState"]["angularVelocity"];
        angular.angularAcceleration = components["RigidBody"]["angularState"]["angularAcceleration"];
        angular.torque = components["RigidBody"]["angularState"]["torque"];

        builder.WithRigidBody(props, linear, angular);
    }

    builder.Create(world);
}    