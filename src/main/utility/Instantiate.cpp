#include "main/utility/Instantiate.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/PolygonCollider.h"

Instantiate& Instantiate::WithRigidBody(Properties properties, LinearState linearState, AngularState angularState)
{
    std::unique_ptr<RigidBody> rb = std::make_unique<RigidBody>(properties, linearState, angularState);
    obj->SetRigidBody(std::move(rb));
    return *this;
}

Instantiate& Instantiate::WithRenderer(Shape shape)
{
    std::unique_ptr<Renderer> r = std::make_unique<Renderer>(shape);
    obj->SetRenderer(std::move(r));
    return *this;
}

Instantiate& Instantiate::WithCollider(ColliderType type, std::variant<Vec2, float, Array<20>> bounds)
{
    switch (type)
    {
    case ColliderType::CIRCLE:
        obj->SetCollider(std::move(std::make_unique<CircleCollider>(std::get<float>(bounds))));
        break;
    case ColliderType::BOX:
        obj->SetCollider(std::move(std::make_unique<BoxCollider>(std::get<Vec2>(bounds))));
        break;
    case ColliderType::POLYGON:
        obj->SetCollider(std::move(std::make_unique<PolygonCollider>(std::get<Array<20>>(bounds))));
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

GameObject* Instantiate::Create(World& world)
{
    world.AddGameObject(std::unique_ptr<GameObject>(obj));
    return obj;
}

Instantiate::Instantiate()
{
    obj = new GameObject();
}