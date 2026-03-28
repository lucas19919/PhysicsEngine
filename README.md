# PhysicsEngine

A custom 2D Physics Engine built in C++.

# Plinko
![](https://github.com/lucas19919/PhysicsEngine/blob/main/plinko.gif)

Using:
 - Raylib for rendering,
 - SAT for collision detection
 - Sutherland-Hodgeman Polygon Clipping for contact points
 - nlohmann/json for JSON Parsing.

**Building the Project:**
1. Clone the repository:
	git clone https://github.com/Lucas19919/PhysicsEngine.git
2. Navigate to the directory:
    cd PhysicsEngine
3. Configure and build:
	mkdir build
	cd build
	cmake ..
	cmake --build .

# Usage:

To load a scene, please change the relative path in main.cpp, "assets/(...).json", to the name of your file name.

To pause and unpause the simulation please press *space* once loaded. 

# Engine and Architechture:

Currently the engine includes circles, rectangles and convex polygons, and calculates both linear and rotational physics. The rigidbody includes forces, torque, mass, inertia and so on.

Each physics body is instantiated as a GameObject. A GameObject can then hold the following components: TransformComponent, Rigidbody, Collider, Renderer. Each component is then doubly linked back to the GameObject to allow for easy access between components and GameObjects. This was based on Unity's system of GameObjects and components.

To handle the physics, a World class is created containing base vectors such as gravity, and also a list of all GameObjects. For each frame, the world first steps, updating all rigidbodies by the delta time, and then checks for any occouring collisions.

The collision management is handled by the Resolve class including 3 methods, ResolveManifold, ResolveImpulse, and ResolvePosition. ResolveManifold is called first and checks for any 2 colliders, if a collision is occuring between them. If yes it returns a Manifold, containing collision data and a list of contact points. Following this, ResolveImpulse calculates the impulse introudced by said collisions between bodies, updating the states of each rigidbody, updating velocities etc. Lastly, the ResolvePosition method adjusts the position of overlapping bodies to prevent anyoverlap, by slightly moving them away from each other depending on their relative velocities.
  
# TODO:
*1. Collision optimization (world tiling)
2. Input System (refresh scene)
3. Constraints
4. Collision Optimization (box v box)*
