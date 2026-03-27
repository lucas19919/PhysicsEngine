#include "main/physics/Manifold.h"
#include "main/physics/SAT.h"
#include "main/GameObject.h"
        
CollisionManifold Manifold::GenCircleCircle(Collider* c1, Collider* c2)
{
    Collision collision = SAT::CircleCircle(c1, c2);
    CollisionManifold cm = { collision };



    return cm;
}

CollisionManifold Manifold::GenBoxCircle(Collider* c1, Collider* c2)
{
    Collision collision = SAT::BoxCircle(c1, c2);
    CollisionManifold cm = { collision };



    return cm;
}

CollisionManifold Manifold::GenBoxBox(Collider* c1, Collider* c2)
{
    Collision collision = SAT::BoxBox(c1, c2);
    CollisionManifold cm = { collision };



    return cm;
}

CollisionManifold Manifold::GenPolyCircle(Collider* c1, Collider* c2)
{
    Collision collision = SAT::PolygonCircle(c1, c2);
    CollisionManifold cm = { collision };



    return cm;  
}

CollisionManifold Manifold::GenPolyBox(Collider* c1, Collider* c2)
{
    Collision collision = SAT::PolygonBox(c1, c2);
    CollisionManifold cm = { collision };



    return cm;
}

CollisionManifold Manifold::GenPolyPoly(Collider* c1, Collider* c2)
{
    Collision collision = SAT::PolygonPolygon(c1, c2);
    CollisionManifold cm = { collision };



    return cm;
}