# CodeVkEngine

CodeVkEngine is a personal Vulkan graphics engine for rapid rendering experiments. It started from the public [Vulkan Renderer Template](https://github.com/Carcodee/Vulkan-Renderer-Template), but the current repository has moved toward a render-graph driven engine with shader reflection, hot reload, ImGui tooling, and several prototype renderers.

The current default renderer is **FlatRenderer**. It is the most recent active path in `src/main.cpp` and focuses on 2D rendering experiments, a paintable canvas, animated sprites, material layers, and Radiance Cascades.

## Current State

- **Default runtime path:** `FlatRenderer` plus ImGui and the render graph profiler/debug UI.
- **Executable target:** `Engine`, produced under the CMake binary directory at `bin/Engine.exe`.
- **Primary platform:** Windows, Vulkan, GLFW Win32 surface, Visual Studio/MSVC-style dependency layout.
- **Shader workflow:** GLSL and Slang are both used. Compiled SPIR-V is checked in under `src/Shaders/spirvGlsl` and `src/Shaders/spirvSlang`.
- **Renderer selection:** renderers are wired manually in `CreateRenderers()` inside `src/main.cpp`.

## Features

- **Render graph architecture**
  - Named render passes with explicit dependencies.
  - Graphics and compute pass support.
  - Queue batching and a profiler/debug UI for inspecting execution order.
  - Render node metadata serialization into `Resources/Engine/RenderNodes`.

- **Vulkan resource layer**
  - Core wrappers for images, buffers, samplers, descriptors, shader modules, pipelines, swapchain, synchronization, and present queue.
  - Automatic descriptor binding by resource name.
  - Dynamic resource manager for buffers, images, staged uploads, and shader assets.

- **Shader system**
  - GLSL and Slang support.
  - SPIR-V reflection through SPIRV-Cross.
  - Runtime pipeline recreation with `R`.
  - Shader compilation debug path with `0`.
  - Batch shader compiler script at `src/Shaders/compile.bat`.

- **ImGui tooling**
  - Render graph timeline and DAG views.
  - Node editor prototype.
  - Renderer-specific debug tabs when the corresponding renderer is enabled.
  - LegitProfiler integration.

- **Asset support**
  - glTF model loading through tinygltf.
  - PLY point-cloud / Gaussian splat loading through happly.
  - Texture, sprite-sheet, material, and animation resource helpers.

## Renderers

### FlatRenderer

`src/Rendering/Renderers/FlatRenderer.hpp`

This is the latest active renderer and the default renderer created by `src/main.cpp`.

FlatRenderer currently includes:

- A compute-driven painting pass using storage images.
- Paint layers for light, occluders, and debug rays.
- Five Radiance Cascades by default.
- Probe generation, radiance output, cascade merge, and final result passes.
- PBR-style material inputs for the 2D background: albedo, normal, roughness, AO, and height.
- Sprite animation sampling.
- ImGui controls for brush radius, brush color, layer selection, canvas clearing, cascade settings, light settings, texture powers, and background material selection.

Use the right mouse button inside the window to paint. The ImGui **Radiance Cascades** tab exposes the live settings.

![Flat Renderer / Radiance Cascades](https://github.com/user-attachments/assets/06dff945-a14a-4c8b-a135-df53979131fe)

Demo videos:

https://github.com/user-attachments/assets/5355f3d6-e4c6-4743-8de7-51b87b09fbe3

https://github.com/user-attachments/assets/0aa9c22a-2a91-41ed-8d49-fb49fab54940

https://github.com/user-attachments/assets/dca2c173-a05d-4da8-828e-f8b720450680

### ClusterRenderer

`src/Rendering/Renderers/ClusterRenderer.hpp`

ClusterRenderer is currently present but commented out in `CreateRenderers()`. It implements a deferred-style path with:

- GPU mesh culling.
- Point-light cluster/tile culling.
- G-buffer outputs for color, normals, tangents, metallic/roughness, UVs, and depth.
- Indirect indexed draws.
- A lighting pass that reconstructs data from the G-buffer.
- First-person camera movement with `W`, `A`, `S`, `D`, right mouse look, and left shift speed boost.

The renderer currently loads `Resources/Assets/Models/floating_lighthouse/scene.gltf` in its setup.

![Cluster Renderer Demo](https://github.com/user-attachments/assets/261a7f85-a560-4fb4-be84-d3ef6517899c)

### GSRenderer

`src/Rendering/Renderers/GSRenderer.hpp`

GSRenderer is a prototype Gaussian splat renderer. It is present but commented out in `CreateRenderers()`.

Current behavior:

- Loads a PLY point cloud from `Resources/Assets/PointClouds/Woman/pc2.ply`.
- Converts splats into arrays used by the GPU shader.
- Draws splats with indexed indirect draws.
- Supports CPU-side depth sorting through the ImGui **Update Sort** button.

Known limitation: GPU sorting is not implemented yet, so the sort must be refreshed manually after meaningful camera changes.

![Gaussian Splats Renderer](https://github.com/user-attachments/assets/9ba7596f-0c5f-46e8-8bf7-9a9ddc7857c2)

### DebugRenderer

`src/Rendering/Renderers/DebugRenderer.hpp`

DebugRenderer is created every run, but it only builds an active pass when `ClusterRenderer` is enabled. Its current use is drawing the cluster renderer camera frustum in wireframe over the final image.

![Debug Renderer Demo](https://github.com/user-attachments/assets/8dacb7c3-1b7e-4374-8ebc-8027921413b0)

### Experimental Renderers

These renderers are included in the source tree but are not active by default:

- `GIRenderer`: simple shader/template based pass for GI/shader experiments.
- `HairRenderer`: tessellation/geometry shader pipeline scaffold.
- `TemplateRenderer`: minimal pattern for adding a new renderer.

## Runtime Controls

Global controls:

- `R`: recreate render node pipelines / reload shader resources.
- `0`: run the shader compilation debug path.
- Right mouse button: active interaction button for camera look in 3D renderers and painting in FlatRenderer.

FlatRenderer:

- Right mouse button: paint at the current mouse position.
- ImGui **Radiance Cascades** tab: brush, layer, cascade, light, texture, material, and animation controls.

ClusterRenderer and GSRenderer:

- `W`, `A`, `S`, `D`: camera movement.
- Right mouse button: mouse-look camera rotation.
- Left shift: speed boost in ClusterRenderer.
- GSRenderer ImGui **Update Sort**: manually resort splats by current view.

## Switching Renderers

Renderer selection is currently manual. Edit `CreateRenderers()` in `src/main.cpp`.

The latest/default setup is:

```cpp
renderers.try_emplace("FlatRenderer", std::make_unique<Rendering::FlatRenderer>(core, windowProvider));
Rendering::FlatRenderer* flatRenderer = dynamic_cast<Rendering::FlatRenderer*>(renderers.at("FlatRenderer").get());
flatRenderer->SetRenderOperation();
```

To test another renderer, comment out FlatRenderer and uncomment the renderer block you want. Some debug UI tabs only appear when the matching renderer exists in the `renderers` map.

## Prerequisites

- Windows with a Vulkan-capable GPU.
- C++20 compiler.
- CMake 3.26 or newer.
- Vulkan SDK.
- Visual Studio 2022/MSVC-compatible toolchain recommended.
- CUDA Toolkit and the repo-local `dependencies/CodeCudaEngine` package, because `CMakeLists.txt` requires `find_package(CodeCudaEngine REQUIRED)`.
- Repo-local dependencies under `dependencies/`.

The current CMake file expects several dependencies at fixed repo-relative paths, including GLFW `lib-vc2022`, Slang libraries, ImGui docking, imgui-node-editor, SPIRV-Cross, tinygltf, happly, nlohmann/json, stb, LegitProfiler, and CodeCudaEngine.

## Build

From the repository root:

```sh
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug --target Engine
```

The executable is written to:

```text
cmake-build-debug/bin/Engine.exe
```

If you use CLion, open the repository root and build the `Engine` target.

## Shader Compilation

Most runtime paths use checked-in SPIR-V. To rebuild shaders manually:

```bat
src\Shaders\compile.bat
```

The script compiles:

- GLSL from `src/Shaders/glsl` into `src/Shaders/spirvGlsl`.
- Slang from `src/Shaders/slang` into `src/Shaders/spirvSlang`.

At runtime, `R` recreates pipelines and `0` calls the render graph shader compilation debug path.

## Important Paths

```text
src/main.cpp                         Application entry point and renderer selection
src/Engine/                          Vulkan core, render graph, resources, descriptors, shaders
src/Rendering/Renderers/             Flat, cluster, Gaussian splat, GI, hair, debug renderers
src/Rendering/RThings/               Camera, model, material, lights, animation, render resources
src/UI/                              Render graph node editor and UI helpers
src/Shaders/glsl/                    GLSL shader sources
src/Shaders/slang/                   Slang shader sources
src/Shaders/spirvGlsl/               Compiled GLSL SPIR-V
src/Shaders/spirvSlang/              Compiled Slang SPIR-V
Resources/Assets/                    Models, textures, animations, point clouds
Resources/Engine/RenderNodes/        Serialized render node metadata
dependencies/                        Third-party libraries expected by CMake
```

## Third-Party Libraries

CodeVkEngine currently uses:

- Vulkan
- GLFW
- GLM
- stb
- SPIRV-Cross
- tinygltf
- ImGui docking
- imgui-node-editor
- Slang
- nlohmann/json
- happly
- LegitProfiler
- CodeCudaEngine

## Known Limitations

- The engine is Windows-focused right now because the app creates a Win32 Vulkan surface and uses Windows/MSVC dependency paths.
- Renderer selection is compile-time/manual through `src/main.cpp`.
- GSRenderer sorting is CPU/manual through the UI; GPU radix sort is present as experimental shader code but not wired into the active path.
- Some renderers are scaffolds or experiments, not polished product features.
- Swapchain recreation is partially handled by the render graph, but individual renderer `RecreateSwapChainResources()` methods are mostly placeholders.
