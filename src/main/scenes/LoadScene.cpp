#include "main/scenes/LoadScene.h"

#include <fstream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "main/GameObject.h"
#include "main/components/Collider.h"
#include "main/components/Renderer.h"
#include "main/components/RigidBody.h"
#include "main/components/TransformComponent.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Motor.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/components/controllertypes/MotorController.h"
#include "main/physics/Config.h"
#include "main/scenes/BoundarySystem.h"
#include "main/scenes/SaveScene.h"
#include "main/utility/Instantiate.h"
#include "math/Vec2.h"

using json = nlohmann::json;

static Color ParseColor(const json& j)
{
    if (j.is_string()) {
        std::string str = j.get<std::string>();
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
    } else if (j.is_object()) {
        return { 
            j.value("r", (unsigned char)255), 
            j.value("g", (unsigned char)255), 
            j.value("b", (unsigned char)255), 
            j.value("a", (unsigned char)255) 
        };
    }
    return GRAY;
}

static Vec2 ParseVec2(const json& j)
{
    return Vec2(j["x"].get<float>(), j["y"].get<float>());
}

void LoadScene::Load(const std::string& filePath, World& world, int screenWidth, int screenHeight)
{
    if (filePath.empty()) {
        LoadFromJSON(json::object(), world, screenWidth, screenHeight);
        return;
    }

    world.Clear();

    std::ifstream file(filePath);
    if (!file.is_open()) {
        // Fallback for failed file open
        LoadFromJSON(json::object(), world, screenWidth, screenHeight);
        return;
    }

    json sceneData;
    file >> sceneData;

    LoadFromJSON(sceneData, world, screenWidth, screenHeight);
}

void LoadScene::LoadFromJSON(const json& sceneData, World& world, int screenWidth, int screenHeight)
{
    world.Clear();

    Vec2 worldSize = sceneData.contains("worldSize") ? ParseVec2(sceneData["worldSize"]) : Vec2(screenWidth * Config::PixelToMeter, screenHeight * Config::PixelToMeter);
    world.SetWorldSize(worldSize);

    Config::screenWidth = (int)(worldSize.x * Config::MeterToPixel);
    Config::screenHeight = (int)(worldSize.y * Config::MeterToPixel);

    Config::useWalls = sceneData.value("useWalls", false);
    BoundarySystem::UpdateBoundaries(world);

    std::unordered_map<int, GameObject*> idMap;
    size_t maxID = 0;

    if (sceneData.contains("objects"))
    {
        for (const auto& item : sceneData["objects"])
        {
            GameObject* obj = LoadObject(item, world);
            if (obj)
            {
                if (item.contains("id"))
                {
                    int id = item["id"].get<int>();
                    obj->SetID(id);
                    idMap[id] = obj;
                    if ((size_t)id >= maxID) maxID = (size_t)id + 1;
                }
                if (item.contains("name"))
                {
                    obj->SetName(item["name"].get<std::string>());
                }
                if (item.contains("groupName"))
                {
                    obj->SetGroupName(item["groupName"].get<std::string>());
                }
                if (item.contains("ignored"))
                {
                    for (const auto& ignoreId : item["ignored"]) {
                        obj->AddIgnored(ignoreId.get<size_t>());
                    }
                }
            }
        }
    }
    world.SetNextID(maxID);

    if (sceneData.contains("generators"))
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> jitter(-Config::generatorJitterRange, Config::generatorJitterRange);

        for (const auto& generator : sceneData["generators"])
        {
            if (!generator.contains("grid") || !generator.contains("object")) continue;
            
            std::string baseName = generator.value("name", "Generator");
            std::string uniqueName = baseName;
            int counter = 1;
            while (world.GetGenerator(uniqueName) != nullptr) {
                uniqueName = baseName + " " + std::to_string(counter++);
            }
            
            std::string groupName = uniqueName;

            const auto& grid = generator["grid"];
            GeneratorDef def;
            def.name = groupName;
            def.rows = grid["rows"];
            def.columns = grid["columns"];
            def.startX = grid["startX"];
            def.startY = grid["startY"];
            def.spacingX = grid["spacingX"];
            def.spacingY = grid["spacingY"];

            json genObject = generator["object"];
            
            // Initialize template object correctly as an orphan
            def.templateObject = LoadObjectOrphan(genObject, world);
            if (def.templateObject) {
                def.objectJson = SaveScene::SerializeObject(def.templateObject.get()).dump();
            }
            
            world.AddGenerator(std::move(def));

            int rows = def.rows;
            int cols = def.columns;

            // Initial generation
            for (int r = 0; r < rows; r++)
            {
                for (int c = 0; c < cols; c++)
                {
                    float posX = def.startX + (c * def.spacingX) + jitter(gen);
                    float posY = def.startY + (r * def.spacingY) + jitter(gen);

                    genObject["components"]["TransformComponent"]["position"]["x"] = posX;
                    genObject["components"]["TransformComponent"]["position"]["y"] = posY;

                    GameObject* obj = LoadObject(genObject, world);
                    if (obj) {
                        obj->SetGroupName(groupName);
                        obj->SetName(groupName + " (" + std::to_string(r * def.columns + c) + ")");
                    }
                }
            }
        }
    }

    if (sceneData.contains("constraints"))
    {
        size_t maxC = LoadConstraints(sceneData["constraints"], world, idMap);
        world.SetNextConstraintID(maxC);
    }

    if (sceneData.contains("controllers"))
    {
        LoadControllers(sceneData["controllers"], world);
    }

    world.UpdateCaches();
}

void LoadScene::Regenerate(World& world, GeneratorDef& def, const std::string& clearGroupName)
{
    if (def.name.empty() || !def.templateObject) return;

    def.objectJson = SaveScene::SerializeObject(def.templateObject.get()).dump();
    nlohmann::json genObject = nlohmann::json::parse(def.objectJson);

    for (int r = 0; r < def.rows; r++)
    {
        for (int c = 0; c < def.columns; c++)
        {
            float posX = def.startX + (c * def.spacingX);
            float posY = def.startY + (r * def.spacingY);

            genObject["components"]["TransformComponent"]["position"]["x"] = posX;
            genObject["components"]["TransformComponent"]["position"]["y"] = posY;

            GameObject* obj = LoadObject(genObject, world);
            if (obj) {
                obj->SetGroupName(def.name);
                obj->SetName(def.name + " (" + std::to_string(r * def.columns + c) + ")");
            }
        }
    }
    
    world.UpdateCaches();
}

void LoadScene::LoadCollection(const nlohmann::json& data, World& world, Vec2 offset, const std::string& groupName)
{
    std::unordered_map<int, GameObject*> idMap;
    size_t maxID = 0;
    size_t maxC = 0;

    if (data.contains("objects"))
    {
        for (const auto& item : data["objects"])
        {
            GameObject* obj = LoadObject(item, world);
            if (obj)
            {
                obj->transform.SetPosition(obj->transform.position + offset);
                if (!groupName.empty()) obj->SetGroupName(groupName);

                if (item.contains("id"))
                {
                    // We generate a NEW ID for the world, but we need the mapping for internal constraints
                    idMap[item["id"].get<int>()] = obj;
                }
                if (obj->GetID() >= maxID) maxID = obj->GetID() + 1;
            }
        }
    }

    if (data.contains("constraints"))
    {
        // We need to be careful with IDs here. LoadConstraints expects an idMap
        maxC = LoadConstraints(data["constraints"], world, idMap);
    }

    // Synchronize world counters if the loaded IDs are higher than current
    // Note: LoadObject/AddConstraint might have already advanced these.
    // This is a safety check.
    // We don't have GetNextID, so we just set it. 
    // Actually AddGameObject already increments nextID if id was -1.
    // LoadObject uses builder.Create(world, -1), which calls AddGameObject, 
    // so nextID is already handled correctly for NEW objects.
    // But LoadConstraints uses SetID(ID) directly if ID is present.

    if (data.contains("controllers"))
    {
        LoadControllers(data["controllers"], world);
    }
}

static void PopulateBuilder(Instantiate& builder, const json& item)
{
    if (!item.contains("components")) return;
    const auto& components = item["components"];

    float posX = 0.0f, posY = 0.0f, rotation = 0.0f;

    if (components.contains("TransformComponent"))
    {
        posX = components["TransformComponent"]["position"]["x"];
        posY = components["TransformComponent"]["position"]["y"];
        rotation = components["TransformComponent"]["rotation"];
    }

    builder.WithTransform(Vec2(posX, posY), rotation);

    if (components.contains("Collider"))
    {
        const auto& col = components["Collider"];
        std::string type = col["type"];

        if (type == "BOX")
            builder.WithCollider(ColliderType::BOX, ParseVec2(col["size"]));
        else if (type == "CIRCLE")
            builder.WithCollider(ColliderType::CIRCLE, col["radius"].get<float>());
        else if (type == "POLYGON")
        {
            Array<20> verts;
            for (const auto& v : col["vertices"]) verts.PushBack(ParseVec2(v));
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
            for (const auto& v : ren["scale"]) verts.PushBack(ParseVec2(v));
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
            Properties{ props["mass"], props["restitution"], props["inertia"], props["friction"] },
            LinearState{ ParseVec2(lin["velocity"]), ParseVec2(lin["acceleration"]), ParseVec2(lin["netForce"]) },
            AngularState{ ang["angularVelocity"], ang["angularAcceleration"], ang["torque"] },
            Settings{ rb.value("gravityEnabled", true) }
        );
    }
}

GameObject* LoadScene::LoadObject(const json& item, World& world)
{
    Instantiate builder;
    PopulateBuilder(builder, item);
    return builder.Create(world, -1);
}

std::unique_ptr<GameObject> LoadScene::LoadObjectOrphan(const json& item, World& world)
{
    Instantiate builder;
    PopulateBuilder(builder, item);
    return builder.CreateOrphan(-1);
}

size_t LoadScene::LoadConstraints(const json& constraints, World& world, const std::unordered_map<int, GameObject*>& idMap)
{
    size_t maxID = 0;
    for (const auto& item : constraints)
    {
        int ID = item.value("id", -1);
        if (ID != -1 && (size_t)ID >= maxID) maxID = (size_t)ID + 1;

        std::string type = item.value("type", "");

        if (type == "DISTANCE")
        {
            if (!item.contains("anchor") || !item.contains("attached")) continue;
            auto itA = idMap.find(item["anchor"]);
            auto itB = idMap.find(item["attached"]);
            if (itA == idMap.end() || itB == idMap.end()) continue;

            auto distance = std::make_unique<DistanceConstraint>(itA->second, itB->second, item.value("length", 0.0f), 
                item.contains("anchorOffset") ? ParseVec2(item["anchorOffset"]) : Vec2(),
                item.contains("attachedOffset") ? ParseVec2(item["attachedOffset"]) : Vec2());
            if (ID != -1) distance->SetID(ID);
            world.AddConstraint(std::move(distance));
        }
        else if (type == "PIN")
        {
            std::vector<PinAttachment> attachments;
            if (item.contains("attachments")) {
                for (const auto& att : item["attachments"])
                {
                    if (!att.contains("id")) continue;
                    auto it = idMap.find(att["id"].get<int>());
                    if (it != idMap.end()) {
                        float lx = 0, ly = 0;
                        if (att.contains("localX")) {
                            lx = att.value("localX", 0.0f);
                            ly = att.value("localY", 0.0f);
                        } else if (att.contains("localAnchor")) {
                            Vec2 v = ParseVec2(att["localAnchor"]);
                            lx = v.x; ly = v.y;
                        }
                        attachments.push_back({ it->second, lx, ly });
                    }
                }
            }

            if (attachments.empty()) continue;
            auto pin = std::make_unique<PinConstraint>(attachments, 
                item.contains("position") ? ParseVec2(item["position"]) : Vec2(attachments[0].obj->transform.position.x + attachments[0].localX, attachments[0].obj->transform.position.y + attachments[0].localY),
                item.value("fixedX", true), item.value("fixedY", true));
            if (ID != -1) pin->SetID(ID);
            world.AddConstraint(std::move(pin));
        }
        else if (type == "JOINT")
        {
            std::vector<JointAttachment> attachments;
            if (item.contains("attachments")) {
                for (const auto& att : item["attachments"])
                {
                    if (!att.contains("id")) continue;
                    auto it = idMap.find(att["id"].get<int>());
                    if (it != idMap.end()) {
                        float lx = 0, ly = 0;
                        if (att.contains("localX")) {
                            lx = att.value("localX", 0.0f);
                            ly = att.value("localY", 0.0f);
                        } else if (att.contains("localAnchor")) {
                            Vec2 v = ParseVec2(att["localAnchor"]);
                            lx = v.x; ly = v.y;
                        }
                        attachments.push_back({ it->second, lx, ly });
                    }
                }
            }

            if (attachments.size() < 2) continue;
            
            bool allowCollisions = item.value("collisions", false);
            if (!allowCollisions) {
                for (size_t i = 0; i < attachments.size(); i++)
                    for (size_t j = i + 1; j < attachments.size(); j++) {
                        attachments[i].obj->AddIgnored(attachments[j].obj->GetID());
                        attachments[j].obj->AddIgnored(attachments[i].obj->GetID());
                    }
            }

            auto joint = std::make_unique<JointConstraint>(attachments, 
                item.contains("position") ? ParseVec2(item["position"]) : Vec2(attachments[0].obj->transform.position.x + attachments[0].localX, attachments[0].obj->transform.position.y + attachments[0].localY),
                allowCollisions);
            if (ID != -1) joint->SetID(ID);
            world.AddConstraint(std::move(joint));
        }
        else if (type == "MOTOR")
        {
            if (!item.contains("rotor")) continue;
            auto it = idMap.find(item["rotor"]);
            if (it == idMap.end()) continue;

            auto motor = std::make_unique<MotorConstraint>(it->second, 
                item.contains("localPosition") ? ParseVec2(item["localPosition"]) : Vec2(),
                item.value("torque", 0.0f));
            if (ID != -1) motor->SetID(ID);
            world.AddConstraint(std::move(motor));
        }
    }
    return maxID;
}

void LoadScene::LoadControllers(const json& controllers, World& world)
{
    for (const auto& item : controllers)
    {
        if (item["type"] == "MOTOR")
        {
            std::vector<MotorConstraint*> motors;
            for (const auto& cId : item["constraints"])
            {
                int idToFind = cId.get<int>();
                for (const auto& c : world.GetConstraints())
                    if (c->GetID() == idToFind) {
                        if (auto* m = dynamic_cast<MotorConstraint*>(c.get())) motors.push_back(m);
                        break;
                    }
            }
            world.AddController(std::make_unique<MotorController>(item.value("active", true), motors, item.value("torqueMax", 100.0f)));
        }
    }
}
