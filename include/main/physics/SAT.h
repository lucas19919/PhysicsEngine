#pragma once
#include <vector>
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
        static Collision CircleCircle(Collider* c1, Collider* c2);
        static Collision BoxBox(Collider* b1, Collider* b2);
        static Collision BoxCircle(Collider* b1, Collider* c1);
        static Collision PolygonCircle(Collider* p1, Collider* c1);
        static Collision PolygonBox(Collider* p1, Collider* b1);
        static Collision PolygonPolygon(Collider* p1, Collider* p2);

    private:
        struct Projection {
            float min;
            float max;
        };

        static Projection Project(std::vector<Vec2> vertices, const Vec2 axis);
        static Projection CircleProject(CircleCollider* c, const Vec2 axis);
        static std::vector<Vec2> GetVertices(Collider* b);
        static std::vector<Vec2> GetNormals(std::vector<Vec2> vertices);        
};