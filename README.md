# Halliday2D

A custom 2D Physics Engine built in C++.

## Plinko
![](https://github.com/lucas19919/PhysicsEngine/blob/main/images/gifs/plinko.gif)

## Double Pendulum
![](https://github.com/lucas19919/PhysicsEngine/blob/main/images/gifs/pendulum.gif)

# Overview
## Using:
 - Raylib for rendering,
 - SAT for collision detection
 - Sutherland-Hodgeman Polygon Clipping for contact points
 - Spatial Hasing for optimization
 - nlohmann/json for JSON Parsing.
 - IMGUI for UI

 ## Features:
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
 - Engine Editor
	- Instantiation
	- Constraints
	- Debugging
	- JSON Scene Loading and Saving

## Building the Project:
1. Clone the repository:
	git clone https://github.com/Lucas19919/PhysicsEngine.git
2. Navigate to the directory:
    cd PhysicsEngine
3. Configure and build:
	mkdir build
	cd build
	cmake ..
	cmake --build .

# Usage
## GameObjects

A GameObject, is the fundamental base object of the engine. This was based on Unity's system of GameObjects and works in a similar way. Each GameObject holds Components and Constraints, more on this below, which define how the GameObject intereacts with the physics enviornment.

### Groups

A Group is essentially a folder of objects. A Group can be created via the Hierarchy. 

### Generators

A Generator is a specific type of GameObject, which can generate a **m** by **n** grid of GameObjects. A Generator includes definitions for the spacing, start position, and the template object to be duplicated.

### Prefabs

A Prefab is a saved **Group** of objects, which ca n be instantiated all at once. An example of this is the default car prefab, loading the prefabs folder. Prefabs are created by saving a Group.

## Components

A component is added to a GameObject, and defines various elements and the interaction with the physics environment. Below, a list and descriptions of all currently included components:

### Transform

The Transform is the default component and defines the position, rotation, and scale of the object. The Transform is fundamental, and hence can not be deleted.

### Rigidbody

The Rigidbody is the core of the physics engine. Here all physics parameters are set, which are then used by the solver. The Rigidbody is divided into 3 sections: **Properties**, **Linear State**, and **Angular State**. Each of these sections includes various physics variables:

#### Properties
- Mass
- Restitution 
- Friction
- Inertia
- Is Gravity Applied?

#### Linear State
 - Velocity Vector
 - Acceleration Vector
 - Net Force

#### Angular State
 - Angular Velocity
 - Angular Acceleration
 - Net Torque

#### Note

A Rigidbody with 0 mass will be considered a static object. Applying a net Force or Torque, does not mean continually application. A net Force or Torque, is no different from applying a acceleration, as these are cleared every frame. 

### Collider

A collider is the primary collision object in the engine. A collider does not need to match with the rendershape of the object, as they are defined as seperate components. Each collider can be given a list of ignored collisions, and can also be toggled active. Colliders, are classified into 3 different types:

#### Circle
A circle collider defines a circular collider, by the radius. 

#### Box
A box collider defines a rectangular collider, by a 2D size vector.

#### Polygon
A polygon collider defines a convex polygon, by a list of points. This is currently not yet supported in the editor.

### Renderer

The Renderer defines the rendered view of the GameObject. Working on the same definitions as the collider, the Renderer additionaly defines a color given by RGBA values. 

### Controller

A controller is used to control motors, via various keybinds. This is currently a premature component, and is not included in the editor. To use it, load in the existing car prefab. Currently the controller is hardcoded to *A* and *D*.

## Constraints

A constraint, similiar to a component, can be added to a GameObject, and defines the motion of said GameObject. 

When attaching an object, it is important to differentiate between the position of the constraint globally, the global position of the GameObject, and the local anchor. The local anchor defines the point where the constraint attaches on the object, relative to the global position of the game object. However, if the constraints global position is not overlapping with the global GameObject position plus local anchor position, the Object will slingshot to said position. All of the constraints feature a local anchor system.

Below, a list and descriptions of all currently included constraints:

### Distance

A Distance constraint, drawn as a thin grey line, defines the set distance between 2 GameObjects. This can be thought of as a small beam, holding the distance. This constriant allows the rotation of both GameObjects while they are attached.



### Pins

A Pin constraint, drawn as a black triangle with either a flat or angled lines, represents a point where an object is fixed. A Pin can be defined to be fixed, or rolling, in either the **X** or **Y** direction. Rotation for all attached GameObjects is allowed. 

### Joints

A Joint constraint, drawn as a small grey circle, acts as a joint between objects. A Joint can attach multiple GameObjects, and allows free rotation between them. Collisions between Joint objects can be toggled on and off, although off is reccomended.

### Motor

A Motor constraint, drawn as a red circle, acts as a small motor applying a set torque per frame. A motor is not a joint itself, so may need to be layered over a joint if that is the goal. 

## Keybinds
*Ctrl + N*: Create new Scene
*Ctrl + S*: Save Scene
*G*: Select translation Gizmo.
*E*: Select rotation Gizmo.
*S*: Select scaling Gizmo.
*R*: Reset Scene.
*Space*: Pause Scene.
*DELETE*: Delete selected GameObject.

# Editor

![](https://github.com/lucas19919/PhysicsEngine/blob/main/images/editor/editor.png)

## Overview

The editor is made up of various "Panels", each of which are collapsable or dockable. Each panel has its own functionality, and together, they provide the ability to make custom scenes, without any JSON file editing.

To manipulate a scene, the top bar includes various drop down menus:
- *File* allows for creating a new scene, saving a scene, or loading an existing scene. 
- *Edit* provides functionality for undo and redo. 
- *Theme* provides the option to switch between different editor themes, with RETRO being the default.
- *Window* allows the closing of panels, and reseting to the default panel layout.
- *World* includes various settings, such as setting the size of the physics enviornment in meters, or pixels.

## Panels

 Currently the editor includes the following Editor Panels: 

 - **Hierarchy**
 - **Inspector**
 - **Performance**
 - **Viewport**
 - **Scene Manager**
 - **Debug**
 - **Constraints**

### Hierarchy

The Hierarchy provides a list of all current GameObjects, Generators or Prefabs in the current scene. Via *right click* on the empty space in the Hierarchy, you are also able to create new objects. You may load Prefabs, Generators, GameOjects, or simply a Group. A Group is used to store multiple objects at once. To select a object in the hierarchy for use, simply *left click*. Multiselection via *Shift* or *Ctrl* is also possible.

### Inspector

The inspector provides a list of all Components and Constraints attached to an object. Here, Components can be added or removed. This also includes editing template objects for Generators. For more on the individual Constraints and Components, please check **Usage** above.

### Performance

The performance panel provides a breakdown of solver timing, and current FPS. Timing is handled in *ms*, and differentiates between Integration, Narrowphase and Broadphase. 

### Viewport

The viewport is the most important panel in the editor, showing the current scene. To pan the viewport camera, hold the *middle mouse button* and move your mouse. To zoom, *scroll*. 

The viewport allows for the selection of GameObjects, or Constraints, in the scene using different selection modes, selecteable in the top left corner of the viewport. These modes are currently: 

**<center>Translation (G), Rotation (E), and Scale (S)</center>**

Selecting any of these modes, whilst selecting a GameObject or Constraint, provides a Gizmo. A Gizmo, like in other Editors, provides a popup around the selected object, with different handles, allowing, for example, **X** and **Y** translation by dragging the respective handles.

When selecting an object, you may also notice orange circles appearing, or black triangles. These represent constraints, and their specific anchor points. Smaller green circles, may also represent the local anchor of these constraints.

### Scene Manager

The scene manager is used to load different scenes from the asset, or a user defined root, folder. To set a custom root folder, click the "Set Assets Root.." button, and select using the windows file explorer. Below the button, you can see the various subfolders of the root folder. The engine, by default, comes with a examples folder, a example prefabs folder, and a user folder to store custom creations.

### Debug

The debug panel is used to enable various debug modes, rendering on the viewport, as well as changing the physics config. These modes are all stored in drop down menus, and can be activated via pressing their respective button, or changing their respective sliders. For more info on what the debug is showing, please read **Engine and Architechture** below.

### Constraints

The constraints panel shows all currently active constraints in a scene. Here each constraint can be selected, and highlighted both in the inspector and the viewport, or simply deleted. For the best constraint editing, select the constraint in this panel, and then view it in the inspector.

# Engine and Architechture: //OUT OF DATE

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
1. Rework constraint solver to seperate position/velocity
2. Motor controller (for robotic arms etc)
3. CCD (Continous Collision Detection)
