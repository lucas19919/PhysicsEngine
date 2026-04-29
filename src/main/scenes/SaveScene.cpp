#include "main/scenes/SaveScene.h"
#include "main/GameObject.h"
#include "main/components/Collider.h"
#include "main/components/Renderer.h"
#include "main/components/RigidBody.h"
#include "main/components/TransformComponent.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/collidertypes/PolygonCollider.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/components/constrainttypes/Joint.h"
#include "main/components/constrainttypes/Motor.h"
#include "main/components/controllertypes/MotorController.h"
#include <fstream>
#include <iomanip>
#include <unordered_set>

using json = nlohmann::json;

static json ColorToJSON(Color color)
{
    return { {"r", color.r}, {"g", color.g}, {"b", color.b}, {"a", color.a} };
}

static json Vec2ToJSON(Vec2 v)
{
    return { {"x", v.x}, {"y", v.y} };
}

void SaveScene::Save(const std::string& filePath, World& world)
{
    json scene = SerializeScene(world);

    std::ofstream file(filePath);
    if (file.is_open())
    {
        file << std::setw(4) << scene << std::endl;
    }
}

json SaveScene::SerializeScene(World& world)
{
    json scene;
    scene["name"] = "Saved Scene";
    scene["worldSize"] = Vec2ToJSON(world.GetWorldSize());
    scene["useWalls"] = Config::useWalls;
    scene["screenWidth"] = Config::screenWidth;
    scene["screenHeight"] = Config::screenHeight;

    json generators = json::array();
    std::unordered_set<std::string> generatorNames;
    for (const auto& gen : world.GetGenerators()) {
        json g;
        g["name"] = gen.name;
        g["grid"] = {
            {"rows", gen.rows},
            {"columns", gen.columns},
            {"startX", gen.startX},
            {"startY", gen.startY},
            {"spacingX", gen.spacingX},
            {"spacingY", gen.spacingY}
        };
        g["object"] = json::parse(gen.objectJson);
        generators.push_back(g);
        generatorNames.insert(gen.name);
    }
    scene["generators"] = generators;
    
    json objects = json::array();
    for (const auto& objPtr : world.GetGameObjects())
    {
        if (objPtr->GetGroupName() == "INTERNAL_WALL") continue;
        if (generatorNames.count(objPtr->GetGroupName())) continue;
        objects.push_back(SerializeObject(objPtr.get()));
    }
    scene["objects"] = objects;

    std::vector<Constraint*> constraintPtrs;
    for (const auto& c : world.GetConstraints()) constraintPtrs.push_back(c.get());
    scene["constraints"] = SerializeConstraints(constraintPtrs);

    std::vector<Controller*> controllerPtrs;
    for (const auto& c : world.GetControllers()) controllerPtrs.push_back(c.get());
    scene["controllers"] = SerializeControllers(controllerPtrs);

    return scene;
}

void SaveScene::SaveCollection(const std::string& filePath, World& world, const std::string& groupName)
{
    if (groupName.empty()) return;

    json prefab;
    prefab["name"] = groupName;

    std::unordered_set<size_t> objectIDs;
    json objectsArr = json::array();
    
    for (const auto& objPtr : world.GetGameObjects()) {
        if (objPtr->GetGroupName() == groupName) {
            objectIDs.insert(objPtr->GetID());
            objectsArr.push_back(SerializeObject(objPtr.get()));
        }
    }

    if (objectIDs.empty()) return;
    prefab["objects"] = objectsArr;

    std::vector<Constraint*> groupConstraints;
    std::unordered_set<size_t> constraintIDs;
    for (const auto& c : world.GetConstraints()) {
        bool internal = false;
        switch (c->GetType()) {
            case DISTANCE: {
                auto* dc = static_cast<DistanceConstraint*>(c.get());
                if (objectIDs.count(dc->anchor->GetID()) && objectIDs.count(dc->attached->GetID())) internal = true;
                break;
            }
            case PIN: {
                auto* pc = static_cast<PinConstraint*>(c.get());
                bool allIn = true;
                for (auto& att : pc->attachments) if (!objectIDs.count(att.obj->GetID())) { allIn = false; break; }
                if (allIn) internal = true;
                break;
            }
            case JOINT: {
                auto* jc = static_cast<JointConstraint*>(c.get());
                bool allIn = true;
                for (auto& att : jc->attachments) if (!objectIDs.count(att.obj->GetID())) { allIn = false; break; }
                if (allIn) internal = true;
                break;
            }
            case MOTOR: {
                auto* mc = static_cast<MotorConstraint*>(c.get());
                if (objectIDs.count(mc->rotor->GetID())) internal = true;
                break;
            }
        }
        if (internal) {
            groupConstraints.push_back(c.get());
            constraintIDs.insert(c->GetID());
        }
    }
    prefab["constraints"] = SerializeConstraints(groupConstraints);

    std::vector<Controller*> groupControllers;
    for (const auto& ctrl : world.GetControllers()) {
        if (ctrl->GetType() == ControllerType::CNT_MOTOR) {
            auto* mc = static_cast<MotorController*>(ctrl.get());
            bool allMotorsIn = true;
            for (auto* m : mc->GetMotors()) if (!constraintIDs.count(m->GetID())) { allMotorsIn = false; break; }
            if (allMotorsIn && !mc->GetMotors().empty()) groupControllers.push_back(ctrl.get());
        }
    }
    prefab["controllers"] = SerializeControllers(groupControllers);

    std::ofstream file(filePath);
    if (file.is_open()) file << std::setw(4) << prefab << std::endl;
}

json SaveScene::SerializeObject(GameObject* obj)
{
    json j;
    j["id"] = obj->GetID();
    j["name"] = obj->GetName();
    if (!obj->GetGroupName().empty()) j["groupName"] = obj->GetGroupName();

    const auto& ignored = obj->GetIgnoredIDs();
    if (!ignored.empty()) {
        json ignoredArr = json::array();
        for (size_t id : ignored) ignoredArr.push_back(id);
        j["ignored"] = ignoredArr;
    }

    json components;
    components["TransformComponent"] = { {"position", Vec2ToJSON(obj->transform.position)}, {"rotation", obj->transform.rotation} };

    if (obj->c)
    {
        json col;
        if (obj->c->GetType() == ColliderType::BOX) {
            col["type"] = "BOX";
            col["size"] = Vec2ToJSON(static_cast<BoxCollider*>(obj->c)->size);
        } else if (obj->c->GetType() == ColliderType::CIRCLE) {
            col["type"] = "CIRCLE";
            col["radius"] = static_cast<CircleCollider*>(obj->c)->radius;
        } else if (obj->c->GetType() == ColliderType::POLYGON) {
            col["type"] = "POLYGON";
            json verts = json::array();
            auto& vArr = static_cast<PolygonCollider*>(obj->c)->vertices;
            for (size_t i = 0; i < vArr.Size(); i++) verts.push_back(Vec2ToJSON(vArr[i]));
            col["vertices"] = verts;
        }
        components["Collider"] = col;
    }

    Renderer* ren = obj->GetComponent<Renderer>();
    if (ren)
    {
        json r;
        Shape shape = ren->GetShape();
        r["color"] = ColorToJSON(shape.color);
        if (shape.form == RenderShape::R_BOX) {
            r["form"] = "R_BOX";
            r["scale"] = Vec2ToJSON(std::get<Vec2>(shape.scale));
        } else if (shape.form == RenderShape::R_CIRCLE) {
            r["form"] = "R_CIRCLE";
            r["scale"] = std::get<float>(shape.scale);
        } else if (shape.form == RenderShape::R_POLYGON) {
            r["form"] = "R_POLYGON";
            json verts = json::array();
            auto& vArr = std::get<Array<20>>(shape.scale);
            for (size_t i = 0; i < vArr.Size(); i++) verts.push_back(Vec2ToJSON(vArr[i]));
            r["scale"] = verts;
        }
        components["Renderer"] = r;
    }

    if (obj->rb)
    {
        json rb;
        rb["gravityEnabled"] = obj->rb->IsGravityEnabled();
        rb["properties"] = { {"mass", obj->rb->GetMass()}, {"restitution", obj->rb->GetRestitution()}, {"inertia", obj->rb->GetInertia()}, {"friction", obj->rb->GetFriction()} };
        rb["linearState"] = { {"velocity", Vec2ToJSON(obj->rb->GetVelocity())}, {"acceleration", Vec2ToJSON(obj->rb->GetAcceleration())}, {"netForce", Vec2ToJSON(obj->rb->GetForce())} };
        rb["angularState"] = { {"angularVelocity", obj->rb->GetAngularVelocity()}, {"angularAcceleration", obj->rb->GetAngularAcceleration()}, {"torque", obj->rb->GetTorque()} };
        components["RigidBody"] = rb;
    }

    j["components"] = components;
    return j;
}

json SaveScene::SerializeConstraints(const std::vector<Constraint*>& constraints)
{
    json arr = json::array();
    for (auto* cPtr : constraints)
    {
        json c;
        c["id"] = cPtr->GetID();
        if (cPtr->GetType() == ConstraintType::DISTANCE) {
            c["type"] = "DISTANCE";
            auto* dc = static_cast<DistanceConstraint*>(cPtr);
            c["anchor"] = dc->anchor->GetID();
            c["attached"] = dc->attached->GetID();
            c["length"] = dc->length;
            c["anchorOffset"] = Vec2ToJSON(dc->anchorOffset);
            c["attachedOffset"] = Vec2ToJSON(dc->attachedOffset);
        } else if (cPtr->GetType() == ConstraintType::PIN) {
            c["type"] = "PIN";
            auto* pc = static_cast<PinConstraint*>(cPtr);
            c["position"] = Vec2ToJSON(pc->position);
            c["fixedX"] = pc->fixedX;
            c["fixedY"] = pc->fixedY;
            json atts = json::array();
            for (const auto& att : pc->attachments) atts.push_back({ {"id", att.obj->GetID()}, {"localX", att.localX}, {"localY", att.localY} });
            c["attachments"] = atts;
        } else if (cPtr->GetType() == ConstraintType::JOINT) {
            c["type"] = "JOINT";
            auto* jc = static_cast<JointConstraint*>(cPtr);
            c["position"] = Vec2ToJSON(jc->position);
            c["collisions"] = jc->collisions;
            json atts = json::array();
            for (const auto& att : jc->attachments) atts.push_back({ {"id", att.obj->GetID()}, {"localX", att.localX}, {"localY", att.localY} });
            c["attachments"] = atts;
        } else if (cPtr->GetType() == ConstraintType::MOTOR) {
            c["type"] = "MOTOR";
            auto* mc = static_cast<MotorConstraint*>(cPtr);
            c["rotor"] = mc->rotor->GetID();
            c["localPosition"] = Vec2ToJSON(mc->localPosition);
            c["torque"] = mc->torque;
        }
        arr.push_back(c);
    }
    return arr;
}

json SaveScene::SerializeControllers(const std::vector<Controller*>& controllers)
{
    json arr = json::array();
    for (auto* ctrlPtr : controllers)
    {
        if (ctrlPtr->GetType() == ControllerType::CNT_MOTOR) {
            json c;
            c["type"] = "MOTOR";
            auto* mc = static_cast<MotorController*>(ctrlPtr);
            c["active"] = mc->active;
            c["torqueMax"] = mc->GetTorqueMax();
            json motors = json::array();
            for (auto* m : mc->GetMotors()) motors.push_back(m->GetID());
            c["constraints"] = motors;
            arr.push_back(c);
        }
    }
    return arr;
}
