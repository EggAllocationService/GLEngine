# Agent Guide for GLEngine

This document captures the non-obvious knowledge needed to work effectively in this repository.

## Project Overview

GLEngine is a cross-platform 3D graphics engine written from scratch in C++20. It uses **WebGPU** (via a custom `wgpu-native` fork) for rendering and **GLFW** for windowing/input. It is structured as a CMake-built shared library (`glengine`) plus a top-level demo executable (`test3d`). The architecture borrows heavily from Unreal Engine (Actors, Components, Pawns, Possession).

## Essential Commands

All commands assume the repo root as working directory.

### Configure and Build

```bash
# Release build (matches the existing build/ directory)
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release

# Debug build
cmake -B build-debug -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build

# Build a specific target
cmake --build build --target test3d
cmake --build build --target glengine
```

- The project uses `FetchContent` to download **zstd**, **meshoptimizer**, **harfbuzz** (optional), and a prebuilt `wgpu-native` binary release. First configure may take a while and requires network access.
- The existing `build/`, `cmake-build-debug/`, and `cmake-build-release/` directories are ignored by `.gitignore`.

### Run

```bash
./build/test3d
```

The demo executable expects `assets.pak` to be in the same directory. CMake generates it as a dependency of `test3d`, so building the target produces it.

### Disable Text Rendering

Harfuzz adds significant build time. Turn it off if you do not need Slug text rendering:

```bash
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release -DTEXT_RENDERING=OFF
```

## Project Structure

| Directory | Purpose |
|-----------|---------|
| `include/` | Public engine headers. Mirrors `src/` layout. |
| `src/` | Engine implementation. |
| `src/3d/` | Actor/Component/Pawn/Transform world system. |
| `src/pipeline/` | Rendering abstractions and the old GL renderer. |
| `src/pipeline/wgpu/` | WebGPU renderer, pipelines, GPU buffers, post-processing. |
| `src/pipeline/wgpu/shaders/` | Built-in WGSL shaders, embedded as C arrays at build time. |
| `src/input/` | `InputManager` and `MouseManager`. |
| `src/assets/` | Built-in OBJ models, packed into `builtin.pak`. |
| `test3d/` | Demo application, custom actors, demo assets/shaders. |
| `cmake/` | Custom CMake modules and build-time tools (`embed.cpp`, `paktool.cpp`, `meshtool.cpp`, `optimizer.cpp`). |
| `external/` | Vendored GLFW and `glfw3webgpu`, plus STB headers. |

## Architecture and Control Flow

### Frame Loop

`Engine::MainLoop()` does:

1. `glfwPollEvents()`
2. `Engine::Update(deltaTime)`
3. `Engine::Render()`

`Update` advances actors and components, then calls `RenderObject::UpdateEnd()` on registered render objects (e.g. light trackers, instancing trackers).

`Render`:
1. Acquires a `FrameBundle` from `WGPURenderer::BeginFrame()`.
2. Computes camera matrices and builds `RenderUniforms`.
3. Calls `RenderObject::RenderStart()` for global render objects (lights, etc.).
4. Iterates all enabled actors; for each with `ActorSceneComponent`, pushes the actor transform onto a `MatrixStack`, then pushes each scene component transform, calls `ActorSceneComponent::Render(pass, stack)`, and pops.
5. Applies post-processing effects.
6. Presents via `WGPURenderer::PresentFrame()`.

### Actor / Component System

- `world::Actor` is an abstract entity. Override `Update(double deltaTime)`.
- `world::ActorComponent` is behavior-only, no transform.
- `world::ActorPrimitiveComponent` adds a local `Transform` (parentable).
- `world::ActorSceneComponent` is renderable; implement `Render(RenderBundle&, MatrixStack&)`.
- `world::Pawn` extends `Actor` with possession/camera support.
- Components **must** be created through `Actor::CreateComponent<T>()`, never `std::make_shared` directly. The constructor relies on the static pointer `ActorComponent::CURRENT_ACTOR_CONSTRUCTING` and `CURRENT_ENGINE_CONSTRUCTING`, which are set only inside `Engine::SpawnActor` and `Actor::CreateComponent`.

```cpp
// Correct
auto actor = engine->SpawnActor<MyActor>();
auto mesh = actor->CreateComponent<StaticMeshComponent>();

// Wrong - component will not know its owning actor/engine
auto bad = std::make_shared<StaticMeshComponent>();
```

### Pawns and Possession

- One pawn can be possessed at a time via `Engine::Possess(shared_ptr<Pawn>)`.
- `Engine::Possess(nullptr)` spawns and possesses a `DefaultPawn` (free-fly camera) at the previous pawn's location.
- Input is directed to the possessed pawn only when the mouse mode is `CAPTIVE` or when `Engine::SetAllowNonFocusedPawnInput(true)` is set.
- `OnPossess(InputManager*)` is where a pawn binds its input.
- A pawn's active `CameraLike` component provides the view/projection matrices.
- `Engine::SetFakePawn()` overrides which pawn's camera is used for rendering, without changing input possession (useful for debugging third-person views).

### Input System

Two `InputManager` instances exist:

- `Engine::GetInputManager()` — global, persists for the program lifetime.
- `Engine::GetPawnInputManager()` — reset on every `Possess`, passed to `OnPossess()`.

Input layers (in order):

1. Focused widget (not currently implemented in the demo but reserved).
2. Possessed pawn (`CAPTIVE` or `allowNonFocusedPawnInput`).
3. Global input manager.

Binding types:

- `AddAction(key, callback)` — fires once per key press.
- `AddAxis(positiveKey, negativeKey, callback)` — fires every frame while held, passing `±deltaTime` or `0`.
- `AddMouseAxis(callback)` — receives `float2` mouse delta; only active in `CAPTIVE` mode.

### Coordinate System and Math

- The engine uses a **left-handed** coordinate system: **+X right, +Y up, +Z into the screen**. This is not the OpenGL convention.
- Custom math headers: `include/Vectors.h`, `include/Matrix.h`, `include/GLMath.h`.
- `mat4`/`mat3` are stored **column-major**.
- Vectors support GLSL-style swizzles (`foo.xy`, `foo.yxz`).
- Built-in `PI` constant is defined in `GLMath.h`.
- View matrices are derived from the absolute camera transform matrix via `math::viewMatrix()` (`src/GLMath.cpp`).

## Rendering Pipeline

### WebGPU Renderer

`WGPURenderer` (`include/pipeline/wgpu/WGPURenderer.h`) manages the WebGPU device, surface, pipelines, and resource upload.

Key types:

- `RenderBundle` — active render pass encoder.
- `FrameBundle` — command encoder + color/depth textures for a frame.
- `RenderUniforms` — per-frame globals uploaded to group 0 binding 0.
- `GPUMesh` — GPU vertex/index buffer wrapper.
- `RenderPipeline` — standard vertex/fragment pipeline.
- `MeshPipeline` — WebGPU mesh-shader pipeline.
- `ComputePipeline` — compute pipeline.

Required native WebGPU features (enabled in `WGPURenderer`):

- `WGPUNativeFeature_Immediates`
- `WGPUNativeFeature_PolygonModeLine`
- `WGPUNativeFeature_MeshShader`
- `WGPUNativeFeature_ShaderI16`

`maxImmediateSize` is set to 128 bytes.

### Standard Shader Layout

Built-in shaders (`src/pipeline/wgpu/shaders/*.wgsl`) expect:

- **Group 0, Binding 0**: `RenderUniforms` uniform buffer.
- **Group 0, Binding 1**: `LightInfo` storage buffer.
- **Group 1, Binding 0**: Per-material `Material` uniform buffer.
- Immediate data: `var<immediate>` for per-draw transforms (e.g. `m: ModelData` or `m: mat4x4<f32>`).

Example from `BasicLit.wgsl`:

```wgsl
@group(0) @binding(0) var<uniform> camera: RenderUniforms;
@group(0) @binding(1) var<storage, read> lights: array<LightInfo>;
@group(1) @binding(0) var<uniform> material: Material;
var<immediate> m: ModelData;
```

Mesh shaders must explicitly `enable wgpu_mesh_shader;` and use the `@mesh(mesh_output)` attribute.

### Creating a Custom Render Pipeline

See `test3d/MeshDemoComponent.cpp`:

```cpp
auto renderer = engine->GetRenderer();
auto shaders = renderer->CompileShader(embed_mesh_wgsl);  // embedded shader
renderer->BuildMeshPipeline(
    "DemoMesh",
    shaders,
    false,              // useTaskShaders
    {},                 // bind group layouts
    sizeof(mat4),       // immediate data size
    nullptr             // RenderPipelineExtras
);
```

Retrieve later via `renderer->GetMeshPipelineByName("DemoMesh")`.

## Resources and Assets

### Resource System

`ResourceManager` loads and caches resources. Resources derive from `glengine::Resource` and must be constructible from `(std::istream&, WGPURenderer*)`.

```cpp
auto mesh = engine->GetResourceManager()->GetResource<StaticMesh>("/assets/cube.obj");
auto font = engine->GetResourceManager()->GetResource<Font>("/builtin/fonts/trim.ttf");
```

### Pak Files

The engine mounts `.pak` archives as virtual filesystems:

```cpp
engine->GetResourceManager()->MountPak("/assets", "assets.pak");
```

CMake generates pak files and embedded C arrays:

- `build_pak(NAME assets ...)` — packs files into `assets.pak`.
- `create_resource_library(NAME DemoShaders SOURCES ...)` — embeds WGSL/text files as `embed_<name>_wgsl` C arrays.

The engine mounts built-in paks automatically in `Engine::Engine()`:

- `/builtin` from `builtin.pak` (cube, plane, sphere OBJs).
- `/builtin/fonts` if `TEXT_RENDERING` is enabled.

### Mesh Assets

- `.obj` files can be loaded directly as `StaticMesh`.
- `.mesh` files are optimized, meshletized, zstd-compressed meshes produced by `meshtool`. They are tracked by Git LFS (`.gitattributes`).
- `StaticMesh` constructor recalculates normals if missing and **normalizes vertex positions to the (-1, 1) range**.

```bash
./build/meshtool source.obj output.mesh
```

## Code Conventions and Style

- C++20 with GNU extensions enabled (`CMAKE_CXX_EXTENSIONS ON`).
- Namespace `glengine` with sub-namespaces (`glengine::world`, `glengine::pipeline::wgpu`, etc.).
- Public classes are marked `GLENGINE_EXPORT` (generated from `glengine_export.h`).
- Headers use `#pragma once`.
- Member variables: many use trailing underscores (`actor_`, `components_`), but not universally; follow the surrounding file.
- Prefer `std::shared_ptr` for engine-owned objects. Ownership flows downward (Actor owns Components).
- Use `std::weak_ptr` at higher levels to avoid dangling references (e.g. `possessedPawn`).

## Important Gotchas

1. **Do not construct Actors or Components manually.** Always use `Engine::SpawnActor<T>()` and `Actor::CreateComponent<T>()`. The static construction pointers are the only way components discover their engine/owner.

2. **Left-handed coordinate system.** `+Z` is into the screen. If your meshes or cameras look wrong, check that you are not assuming OpenGL's right-handed convention.

3. **Matrices are column-major.** When reading code that constructs matrices from raw arrays, the memory layout is column-major even though the initializer lists are written linearly.

4. **Meshes are auto-normalized.** `StaticMesh` scales all vertices into a (-1, 1) bounding box. Do not rely on original scene units for `.obj` assets.

5. **WGSL `var<immediate>` is custom.** The shader syntax `var<immediate>` is a wgpu-native extension for push-style immediate data, not standard WGSL. It maps to the `immediateDataBytes` argument in pipeline builders.

6. **WebGPU native fork.** The project downloads `wgpu-native` from `EggAllocationService/wgpu-native` releases, not the upstream `gfx-rs/wgpu-native` repo. It requires specific native features (mesh shaders, immediates, etc.).

7. **No formal test suite.** The only validation target is `test3d`. Run it after changes to verify rendering behavior.

8. **CMake build tools are generated.** `util_embed`, `paktool`, and `meshtool` are not checked-in binaries; they are built from `cmake/` sources as part of the configure/build process.

9. **Mouse input callback inverts Y.** `mouseMoveExec` passes `-y` to `MouseManager::HandleMotion`.

10. **Window size vs framebuffer size.** `Engine::SetWindowSize()` stores the window size but calls `renderer->Resize()` with the framebuffer size from `glfwGetFramebufferSize`.

11. **PakFile.h header issue.** `include/PakFile.h` is currently not valid C++ (it uses `namespace` without a surrounding language context). It appears to be a stale/in-progress file. Do not include it directly; the actual pak format constants are duplicated where needed.

## Adding a New Actor or Component

1. Create a header/impl pair under `test3d/` (for demo actors) or the appropriate `include/` and `src/` directories (for engine types).
2. Derive from `glengine::world::Actor` or the correct component base.
3. Override `Update(double)` for actors/components and `Render(...)` for scene components.
4. Register input in `OnPossess()` for pawns.
5. Spawn via `engine->SpawnActor<T>()` in `main.cpp` or another actor.

## Debugging Hints

- Use `RenderBundle::PushDebug("name")` / `PopDebug()` to annotate WebGPU render passes; visible in graphics debuggers.
- `Engine::GetLastPerformanceTimes()` returns update and render durations in milliseconds.
- Set `allowNonFocusedPawnInput` to true to test pawn input without capturing the mouse.
- Use `Engine::SetFakePawn()` to render from a different camera while keeping input on the possessed pawn.

## Dependencies Fetched by CMake

| Dependency | Source | Purpose |
|------------|--------|---------|
| zstd | facebook/zstd `dev` | Pak file compression |
| meshoptimizer | zeux/meshoptimizer v1.3 | Meshlet generation |
| harfbuzz | harfbuzz/harfbuzz 14.2.0 | Text shaping (optional) |
| wgpu-native | EggAllocationService/wgpu-native 30.0.1-kyle.3 | WebGPU implementation |
