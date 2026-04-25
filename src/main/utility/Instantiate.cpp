#include "main/utility/Instantiate.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/PolygonCollider.h"
#include "main/components/constrainttypes/Distance.h"
#include "main/components/constrainttypes/Pin.h"
#include "main/components/constrainttypes/Joint.h"

Instantiate& Instantiate::WithRigidBody(Properties properties, LinearState linearState, AngularState angularState, Settings settings)
{
    std::unique_ptr<RigidBody> rb = std::make_unique<RigidBody>(properties, linearState, angularState, settings);
    obj->AddComponent<RigidBody>(std::move(rb));
    return *this;
}

Instantiate& Instantiate::WithRenderer(Shape shape)
{
    std::unique_ptr<Renderer> r = std::make_unique<Renderer>(shape);
    obj->AddComponent<Renderer>(std::move(r));
    return *this;
}

Instantiate& Instantiate::WithCollider(ColliderType type, std::variant<Vec2, float, Array<20>> bounds)
{
    switch (type)
    {
    case ColliderType::CIRCLE:
        obj->AddComponent<CircleCollider>(std::make_unique<CircleCollider>(std::get<float>(bounds)));
        break;
    case ColliderType::BOX:
        obj->AddComponent<BoxCollider>(std::make_unique<BoxCollider>(std::get<Vec2>(bounds)));
        break;
    case ColliderType::POLYGON:
        obj->AddComponent<PolygonCollider>(std::move(std::make_unique<PolygonCollider>(std::get<Array<20>>(bounds))));
        break;
    default:
        break;
    }

    return *this;
}

Instantiate& Instantiate::WithTransform(Vec2 position, float rotation)
{
    obj->transform.position = position;
    obj->transform.rotation = rotation;
    return *this;
}

GameObject* Instantiate::Create(World& world, size_t id)
{
    obj->SetID(id);
    GameObject* ptr = obj.get();
    world.AddGameObject(std::move(obj));
    return ptr;
}

std::unique_ptr<GameObject> Instantiate::CreateOrphan(size_t id)
{
    obj->SetID(id);
    return std::move(obj);
}

Instantiate::Instantiate()
{
    obj = std::make_unique<GameObject>();
}