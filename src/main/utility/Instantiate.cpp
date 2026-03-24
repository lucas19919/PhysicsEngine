#include "main/utility/Instantiate.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/PolygonCollider.h"

Instantiate& Instantiate::WithRigidBody(Properties properties, LinearState linearState, AngularState angularState)
{
    obj->SetRigidBody(new RigidBody(properties, linearState, angularState));
    return *this;
}

Instantiate& Instantiate::WithRenderer(Shape shape)
{
    obj->SetRenderer(new Renderer(shape));
    return *this;
}

Instantiate& Instantiate::WithCollider(ColliderType type, std::variant<Vec2, float, std::vector<Vec2>> bounds)
{
    switch (type)
    {
    case ColliderType::CIRCLE:
        obj->SetCollider(new CircleCollider(std::get<float>(bounds)));
        break;
    case ColliderType::BOX:
        obj->SetCollider(new BoxCollider(std::get<Vec2>(bounds)));
        break;
    case ColliderType::POLYGON:
        obj->SetCollider(new PolygonCollider(std::get<std::vector<Vec2>>(bounds)));
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
    world.AddGameObject(obj);
    return obj;
}

Instantiate::Instantiate()
{
    obj = new GameObject();
}