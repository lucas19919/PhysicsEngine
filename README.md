# Halliday2D

A custom 2D Physics Engine built in C++.

# Plinko
![](https://github.com/lucas19919/PhysicsEngine/blob/main/gifs/plinko.gif)

# Double Pendulum
![](https://github.com/lucas19919/PhysicsEngine/blob/main/gifs/pendulum.gif)

**Using:**
 - Raylib for rendering,
 - SAT for collision detection
 - Sutherland-Hodgeman Polygon Clipping for contact points
 - Spatial Hasing for optimization
 - nlohmann/json for JSON Parsing.
 - IMGUI for UI

 **Features:**
 - Rigidbody Physics
	- Linear
	- Angular
 - Circle, Box and Convex Polygons Colliders
 - Sequential Impulse Solver
 - Constraints
	- Distance
	- Fixed and Rolling Pins
	- Joints
	- Motor
 - Controllers
	- Motor Controller
 - JSON Level Loading

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

To load a scene, please add the relative path into the UI textbox, "../assets/examples/(...).json", to the name of your file. 

To change any internal configuration varaibles, please edit "Config.h". 

*Space* to pause, *R* to reset the level.

# Engine and Architechture:

Currently the engine includes circles, rectangles and convex polygons, and calculates both linear and rotational physics. The rigidbody includes forces, torque, mass, inertia and so on.

Each physics body is instantiated as a GameObject. A GameObject can hold the following components: TransformComponent, Rigidbody, Collider, Renderer. This was based on Unity's system of GameObjects and components. Each GameObject can then be given further constraints: Distance, Pin (fixed or rolling), Joint. Each of these constraints are assigned post instatantion of the GameObjects, and GameObjects are assigned via ID to them at launch. 

Scenes are loaded via a path in main to the assets folder using nlohmann/json as a json parser. Please see assets/demos/ ... for various demo's of the engine, and examples of how to create levels.

Physics are handled within a World class, which is created at launch and contains all GameObjects and Constraints. The world is updated, via the World.step(dt) method, which handles physics via the following pipeline:  

- Clear all caches
- Update velocity from acceleration and external forces
- Update the broadphase, updating the spatial hash grid to locate GameObjects
- Generate candidate pairs from the grid, determined via checking AABB bounds
- Build contact constraints and gather all rigidbody data
- Warmstart all constraints
- Solve all constraints iteratively via a impulse solver
- Update positions from velocity
- Update sleep conditions for rigidbodies
- Cleanup

The collision handling is done via SAT collision detection and with Sutherland Hogemann polygon clipping to find collision points. The solver class is first called to generate a collision manifold, sorting the collision by the collider types (ie circle circle, or box circle ...). Once sorted the manifold is calculated, containing contact points, collision normals, and penetration depths. This data is then collected in a contact constraint, which is then resolved by the solver. Based on the collision and properties of the RigidBody, an impulse is then calculated. The impulse is then applied along the collision normal to each collision RigidBody.

**To Do:**
1. Gravity Rework (pixel to meter)
2. Rework constraint solver to seperate position/velocity
3. Motor controller (for robotic arms etc)
4. Better Level Editor
