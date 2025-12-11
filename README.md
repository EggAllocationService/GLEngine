# GLEngine

3D Graphics Engine written from scratch in C++, using freeglut. 

## Engine

### General

A frame is split into Update and Render passes. Update advances the state of the world, while Render draws it to the screen. All Update functions are given a deltaTime parameter representing the number of seconds since the last frame,
allowing for framerate-independent movement. The world is made of Actors, which are objects orchestrating Components, which are what is actually rendered. Special kinds of Actors called Pawns can be Possessed by the user, allowing them to
control the pawn in some way. Memory is tracked via shared pointers, and ownership flows downwards so when an Actor is destroyed it and its subobjects are immediately freed. Weak pointers are used at higher levels to avoid segfaults.

The engine supports window reshapes out of the box, and abstracts almost all GLUT functionality to allow for a more cohesive experience. Also included is a robust 2D UI rendering system (from assignment 1) and an onscreen console, allowing for
debugging commands to be executed. A simplified input system allows binding C++ lambdas to keys, and optionally handling relative mouse input.

### 3D
The rendering engine exposes the following 3d primitives, which are used in the assignment implementations:

- Actor: A logical representation of an "entity" with a point in 3d space. Does not contain any rendering code, instead an Actor orchestrates multiple components contained within it.
- Actor Components: Composable units of functionality that are used to build an Actor
    - ActorComponent: Most basic form. Is behaviour-only; they do not have transforms and cannot render anything, but do have an Update function called every frame
    - ActorPrimitiveComponent: A subclass of ActorComponent which has a transform (parented to the owning Actor). Used for objects that exist in the world but are not rendered, like a camera or control point
    - ActorSceneComponent: Subclass of ActorPrimitiveComponent which can render objects into the world. This is what's responsible for rendering 3d models or debug lines
    - A few pre-made helper components in the engine directory (engine/src/3d/components and engine/src/3d/mesh):
        - CameraComponent: Represents a camera that the world can be viewed through. Controls FOV and near/far plane distance.
        - PointLightComponent: Provides functionality to add a point light to actors. All instances of this component are collected during Update() and used to feed glLight calls at the start of rendering
        - StaticMeshComponent: Renders a static mesh. Not much to say about this one, it does what it says on the tin. Also exposes material properties
- Pawn: A subclass of Actor which can be controlled by the user. Contains a default Camera Component at the origin, but this can be overridden with the SetActiveCamera function
    - DefaultPawn (private): a free-flying Pawn implementation, supporting WASD movement and arrow keys for camera direction. Used as fallback
- Transform: A class containing separate translation/rotation/scale vectors, and a cached transform matrix which is kept up-to-date. Can be parented to another transform.

Source code for these classes is in `engine/src/3d` and `engine/include/3d`

Of note is that the engine does not use the conventional OpenGL coordinate system; It is instead a left-handed coordinate system where +Z is into the screen, +X is right, and +Y is up. This more closely
tracks with what I'm used to, but required me to implement my own functions for generating view/transform matricies. Those functions are in `src/GLMath.cpp`. Using my own view function is significantly more efficent as well,
as I already have access to an absolute transform matrix for the active camera, which can just be transposed and manipulated slightly to produce a view matrix.

I also wrote my own implementation of transform matricies instead of using glRotate/glScale/glTranslate. There are two main reasons: I can generate a transform matrix for a (translate, scale, rotate) triplet without incurring costly
matrix multiplication, and allowing direct access to the matricies allows me to examine the world when I'm not rendering. This is used in the engine to compute absolute positions and rotations for cameras and point lights, which may
be attached through numerous parent transforms. All related code is in `src/3d/Transform.cpp`.

Same as my last assignment, most code makes heavy use of the linear algebra library I wrote. It mirrors GLSL syntax and provides fixed-length 2/3/4D int/float vectors, and 3x3/4x4 matricies. Matrix-Matrix multiplication and Matrix-Vector
multiplication is supported, as well as lanewise/scalar vector arithmatic (including vector swizzling!). All related code is in the headers `include/Vectors.h` and `include/Matrix.h`. All types are templated
and the math routines have been authored with auto-vectorization in mind.

### Pawns

I'm borrowing a lot of terminology from Unreal Engine, as that's a game engine I have a lot of experience with. Pawns represent things that the user *can* control, but not neccesarily what the user *is* controlling.

The users input is directed via "Possession". One pawn can be possesed at a time (by calling `Engine::Possess(shared_ptr<Pawn>)`). Possessing a pawn performs a few actions:

- Calls OnUnpossess() on the previously possessed pawn (if present)
- Uses the pawn's active camera component as a point of view for the world
- Resets the pawn input manager so that the previous pawn no longer has input directed to it
- Calls OnPossess() on the new pawn, passing an InputManager pointer (discussed in the "Input" section")

You can possess a `nullptr`, in which case the engine will spawn a DefaultPawn (essentially just a free-cam) and possess that.

The main advantage of this architecture is it allows better encapsulation; all code related to the sub in this assignment is contained in one file.
If we wanted to spawn a second sub and allow the user to switch between them, it's as simple as calling `Engine::Possess` with the new sub. Camera and input are automatically switched, leaving the old one empty. Or for example, if we
wanted to be able to fire torpedoes, we'd spawn a torpedo Pawn and possess that, allowing the user to steer it until collision at which point we re-possess the sub.

### Keyboard Input

The engine has three input layers. Keyboard input can either be consumed by a layer or fall through to the next.

- Focused Widget: The currently focused widget is prioritized above all else; if any widget is focused (i.e. a text input box) all input will be consumed by this layer.
- Possessed Pawn: The Pawn currently controlled by the player is then offered all keyboard input, if there is a binding set up then it will be consumed at this layer
- Global Input: Finally, a global "fallback" layer exists as a catch-all

Keyboard input is handled by the `InputManager` class in `engine/src/input`. There are two active input managers for each engine instance: a global one and a pawn one. The global manager persists for the lifetime of the program and is for
general keybinds (by default, it allows the user to open the console by pressing backtick). The pawn manager is reset each time `Engine::Possess` is called, and is passed to the pawn's OnPossess function. This is how a pawn sets up input
mappings for itself.

The InputManager supports three kinds of bindings: Action, Axis, and Mouse:
- Action mappings are the simplest kind: the user presses a button, and the callback function you passed gets called. Used in this assignment for toggling fog/wireframe, and opening the console
- Axis mappings describe a one-dimensional axis, where one key has a positive influence and a negative influence. In this assignment, used to implement all the directional controls. Each frame that a key bound to an axis is held,
  the specified callback function is executed. The function accepts a float parameter, which will either be positive or negative deltaTime, or zero (depending on which keys in the axis definition are held).
- Mouse bindings allow for relative mouse input. Not used in this assignment, but can be used for first-person look cameras. The callback is only ran if the mouse mode is set to CAPTIVE.

### Rendering:

The rendering system is pretty simple, each actor is iterated and we look for any ActorSceneComponents on each one, if there's at least one we push that actor's transform to the stack and iterate child scene components, pushing their
matrices to the stack and calling `ActorSceneComponent::Render` for each. Most components set up a material then call some glu drawing function.

The only notable exception is lights, where during the Update phase `LightInfo` structs are submitted to a `RenderObjects` instance. `RenderObjects` exists to track items that require special handling during rendering. Each `LightInfo`
struct contains an absolute world-space position, as well as color information. These values are passed to `glLight` function calls at the start of the Render phase.

### Resources:

The engine provides a global ResourceManager object, to centralize resource loading and management. The only resource type supported at the moment is `StaticMesh`, which is a non-animated mesh loaded from a .obj file.

After the initial vertex data is loaded, the engine re-calculates normal vectors (if required, existing normals will be kept if present in the file), then normalizes the scale so all vertices are within a (-1, 1) range. Finally, a display list
is compiled with the vertex data, which is then called when the mesh needs to be rendered.

### Console:

A built in development console is provided, to allow for more complex commands to be added without complicated keybinds. By default pressing the backtick key will open the console, which appears as a text bar along the bottom of the screen.
Applications can add their own commands to this console, but there are two default commands included:

- detach: Spawns an empty DefaultPawn at the currently possessed pawn's origin, and possesses it.
- quit: Closes the application

Autocomplete is provided for command names, and the five closest matches will be shown in a box above the text caret. Pressing enter submits the command, which will close the console if a command is successfuly executed.

Internally, commands are stored in a (string, function) map. When the first word of the input matches a command, that is stripped and the corresponding function pointer is called with a string view containing the remainder of the string (to allow for arguments).
