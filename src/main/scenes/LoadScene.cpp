#include "main/scenes/LoadScene.h"
#include "main/utility/Instantiate.h"
#include "main/GameObject.h"
#include "math/Vec2.h"
#include "main/physics/Config.h"
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <unordered_map>
#include "main/components/Collider.h"
#include "main/components/Renderer.h"
#include "main/components/RigidBody.h"
#include "main/components/TransformComponent.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Motor.h"
#include "main/components/controllertypes/MotorController.h"


using json = nlohmann::json;

static Color ParseColor(const std::string& str)
{
    if (str == "RED")      return RED;
    if (str == "GREEN")    return GREEN;
    if (str == "BLUE")     return BLUE;
    if (str == "BROWN")    return BROWN;
    if (str == "DARKGRAY") return DARKGRAY;
    if (str == "YELLOW")   return YELLOW;
    if (str == "ORANGE")   return ORANGE;
    if (str == "WHITE")    return WHITE;
    if (str == "BLACK")    return BLACK;
    return GRAY;
}

static Vec2 ParseVec2(const json& j)
{
    return Vec2(j["x"].get<float>(), j["y"].get<float>());
}

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
            .WithTransform(Vec2(screenWidth / 2.0f, screenHeight + 100.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(screenWidth, 200.0f))
            .Create(world, 9000);

        Instantiate()
            .WithTransform(Vec2(screenWidth / 2.0f, -100.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(screenWidth, 200.0f))
            .Create(world, 9001);

        Instantiate()
            .WithTransform(Vec2(-100.0f, screenHeight / 2.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(200.0f, screenHeight))
            .Create(world, 9002);

        Instantiate()
            .WithTransform(Vec2(screenWidth + 100.0f, screenHeight / 2.0f), 0.0f)
            .WithCollider(ColliderType::BOX, Vec2(200.0f, screenHeight))
            .Create(world, 9003);
    }

    std::unordered_map<int, GameObject*> idMap;

    for (const auto& item : sceneData["objects"])
    {
        GameObject* obj = LoadObject(item, world);
        if (obj && item.contains("id"))
        {
            obj->SetID(item["id"].get<int>());
            idMap[item["id"].get<int>()] = obj;
        }
    }

    if (sceneData.contains("generators"))
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> jitter(-Config::generatorJitterRange, Config::generatorJitterRange);

        for (const auto& generator : sceneData["generators"])
        {
            if (!generator.contains("grid") || !generator.contains("object")) continue;

            const auto& grid = generator["grid"];
            int rows = grid["rows"];
            int cols = grid["columns"];
            float startX = grid["startX"];
            float startY = grid["startY"];
            float spacingX = grid["spacingX"];
            float spacingY = grid["spacingY"];

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

    if (sceneData.contains("constraints"))
    {
        LoadConstraints(sceneData["constraints"], world, idMap);
    }

    if (sceneData.contains("controllers"))
    {
        LoadControllers(sceneData["controllers"], world);
    }
}

GameObject* LoadScene::LoadObject(const json& item, World& world)
{
    if (!item.contains("components")) return nullptr;
    const auto& components = item["components"];

    float posX = 0.0f, posY = 0.0f, rotation = 0.0f;

    if (components.contains("TransformComponent"))
    {
        posX = components["TransformComponent"]["position"]["x"];
        posY = components["TransformComponent"]["position"]["y"];
        rotation = components["TransformComponent"]["rotation"];
    }

    Instantiate builder;
    builder.WithTransform(Vec2(posX, posY), rotation);

    if (components.contains("Collider"))
    {
        const auto& col = components["Collider"];
        std::string type = col["type"];

        if (type == "BOX")
        {
            builder.WithCollider(ColliderType::BOX, ParseVec2(col["size"]));
        }
        else if (type == "CIRCLE")
        {
            builder.WithCollider(ColliderType::CIRCLE, col["radius"].get<float>());
        }
        else if (type == "POLYGON")
        {
            Array<20> verts;
            for (const auto& v : col["vertices"])
                verts.PushBack(ParseVec2(v));
            builder.WithCollider(ColliderType::POLYGON, verts);
        }
    }

    if (components.contains("Renderer"))
    {
        const auto& ren = components["Renderer"];
        Shape shape;
        shape.color = ParseColor(ren["color"]);
        std::string form = ren["form"];

        if (form == "R_BOX")
        {
            shape.form = RenderShape::R_BOX;
            shape.scale = ParseVec2(ren["scale"]);
        }
        else if (form == "R_CIRCLE")
        {
            shape.form = RenderShape::R_CIRCLE;
            shape.scale = ren["scale"].get<float>();
        }
        else if (form == "R_POLYGON")
        {
            shape.form = RenderShape::R_POLYGON;
            Array<20> verts;
            for (const auto& v : ren["scale"])
                verts.PushBack(ParseVec2(v));
            shape.scale = verts;
        }

        builder.WithRenderer(shape);
    }

    if (components.contains("RigidBody"))
    {
        const auto& rb = components["RigidBody"];
        const auto& props = rb["properties"];
        const auto& lin = rb["linearState"];
        const auto& ang = rb["angularState"];

        builder.WithRigidBody(
            Properties{
                props["mass"], props["restitution"],
                props["inertia"], props["friction"]
            },
            LinearState{
                ParseVec2(lin["velocity"]),
                ParseVec2(lin["acceleration"]),
                ParseVec2(lin["netForce"])
            },
            AngularState{
                ang["angularVelocity"],
                ang["angularAcceleration"],
                ang["torque"]
            },
            Settings{ rb.value("gravityEnabled", true) }
        );
    }

    return builder.Create(world, -1);
}

void LoadScene::LoadConstraints(const json& constraints, World& world, const std::unordered_map<int, GameObject*>& idMap)
{
    for (const auto& item : constraints)
    {
        int ID = item.value("id", -1);
        std::string type = item["type"];

        if (type == "DISTANCE")
        {
            int anchorId = item["anchor"];
            int attachedId = item["attached"];
            float length = item["length"];
            Vec2 anchorOffset = item.contains("anchorOffset") ? ParseVec2(item["anchorOffset"]) : Vec2();
            Vec2 attachedOffset = item.contains("attachedOffset") ? ParseVec2(item["attachedOffset"]) : Vec2();

            auto itA = idMap.find(anchorId);
            auto itB = idMap.find(attachedId);
            if (itA == idMap.end() || itB == idMap.end()) continue;

            auto distance = std::make_unique<DistanceConstraint>(itA->second, itB->second, length, anchorOffset, attachedOffset);
            if (ID != -1) distance->SetID(ID);
            world.AddConstraint(std::move(distance));
        }
        else if (type == "PIN")
        {
            std::vector<PinAttachment> attachments;
            Vec2 anchor = Vec2();

            for (const auto& att : item["attachments"])
            {
                auto it = idMap.find(att["id"].get<int>());
                if (it == idMap.end()) continue;

                anchor = att.contains("localAnchor") ? ParseVec2(att["localAnchor"]) : Vec2();
                attachments.push_back({ it->second, anchor });
            }

            if (attachments.empty()) continue;

            bool fixedX = item.value("fixedX", true);
            bool fixedY = item.value("fixedY", true);

            if (!fixedX && !fixedY) continue;

            Vec2 pinPos = item.contains("position") ? ParseVec2(item["position"]) : attachments[0].obj->transform.position;
            auto pin = std::make_unique<PinConstraint>(attachments, pinPos, fixedX, fixedY);
            if (ID != -1) pin->SetID(ID);
            world.AddConstraint(std::move(pin));
        }
        else if (type == "JOINT")
        {
            std::vector<JointAttachment> attachments;
            for (const auto& att : item["attachments"])
            {
                auto it = idMap.find(att["id"].get<int>());
                if (it == idMap.end()) continue;

                Vec2 anchor = att.contains("localAnchor") ? ParseVec2(att["localAnchor"]) : Vec2();
                attachments.push_back(JointAttachment{ it->second, anchor });
            }

            if (attachments.size() < 2) continue;

            for (size_t i = 0; i < attachments.size(); i++)
            {
                for (size_t j = i + 1; j < attachments.size(); j++)
                {
                    attachments[i].obj->AddIgnored(attachments[j].obj->GetID());
                    attachments[j].obj->AddIgnored(attachments[i].obj->GetID());
                }
            }

            Vec2 position = item.contains("position") ? ParseVec2(item["position"]) : attachments[0].obj->transform.position;

            auto joint = std::make_unique<JointConstraint>(attachments, position, item.value("collisions", false));
            if (ID != -1) joint->SetID(ID);
            world.AddConstraint(std::move(joint));
        }
        else if (type == "MOTOR")
        {
            int rotorId = item["rotor"];
            Vec2 localPosition = item.contains("localPosition") ? ParseVec2(item["localPosition"]) : Vec2();
            float torque = item["torque"].get<float>();

            auto it = idMap.find(rotorId);
            if (it == idMap.end()) continue;

            auto motor = std::make_unique<MotorConstraint>(it->second, localPosition, torque);
            if (ID != -1) motor->SetID(ID);
            world.AddConstraint(std::move(motor));
        }
    }
}

void LoadScene::LoadControllers(const json& controllers, World& world)
{
    for (const auto& item : controllers)
    {
        std::string type = item["type"];

        if (type == "MOTOR")
        {
            bool active = item.value("active", true);
            std::vector<MotorConstraint*> motors;

            if (item.contains("constraints"))
            {
                for (const auto& cId : item["constraints"])
                {
                    int idToFind = cId.get<int>();
                    Constraint* targetConstraint = nullptr;
                    
                    for (const auto& c : world.GetConstraints())
                    {
                        if (c->GetID() == idToFind)
                        {
                            targetConstraint = c.get();
                            break;
                        }
                    }
                    
                    if (targetConstraint)
                    {
                        MotorConstraint* motor = dynamic_cast<MotorConstraint*>(targetConstraint);
                        if (motor)
                        {
                            motors.push_back(motor);
                        }
                    }
                }
            }

            float torqueMax = item.value("torqueMax", 100.0f);

            auto controller = std::make_unique<MotorController>(active, motors, torqueMax);
            world.AddController(std::move(controller));
        }
    }
}