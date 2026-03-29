#pragma once
#include "main/utility/templates/Array.h"
#include "main/components/collidertypes/BoxCollider.h"
#include "main/components/collidertypes/CircleCollider.h"
#include "main/components/collidertypes/PolygonCollider.h"

struct Collision {
    bool isColliding;
    Vec2 normal;
    float depth;
};

class SAT
{
    public:
        static Collision CircleCircle(GameObject* obj1, GameObject* obj2);
        static Collision BoxBox(GameObject* obj1, GameObject* obj2);
        static Collision BoxCircle(GameObject* obj1, GameObject* obj2);
        static Collision PolygonCircle(GameObject* obj1, GameObject* obj2);
        static Collision PolygonBox(GameObject* obj1, GameObject* obj2);
        static Collision PolygonPolygon(GameObject* obj1, GameObject* obj2);

        struct Projection {
            float min;
            float max;
        };

        static Projection Project(const Array<20>& vertices, const Vec2 axis);
        static Array<20> GetVertices(GameObject* obj);

    private:
        static Projection CircleProject(GameObject* obj, const Vec2 axis);
        static Array<20> GetNormals(const Array<20>& vertices);        
};