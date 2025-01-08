# CodeVkRenderer: A Vulkan-Based Renderer

CodeVkRenderer Is my personal Renderer that I extend from this [**public template**](https://github.com/Carcodee/Vulkan-Renderer-Template). Here I implement diferent rendering techniques such as, Radiance Cascades in 2d, GPU oclussion culling, Cluster Rendering and more.

## Features

- **RenderGraph-Based Architecture**: Flexible setup of rendering pipelines designed to facilitate rapid prototyping. This system is functional but still undergoing optimization for improved performance.

- **Modular Vulkan Interface**: Built with modularity in mind, allowing rapid iteration and customization (including automatic descriptor setup).
  
- Simple shaders hot reload/reflection system

- **GLTF Loader**: Import and render GLTF models with ease, streamlining 3D asset integration.

- **Simple First-Person Camera**: Includes a basic first-person camera controller.

- **Examples**:
  - [**Forward Renderer**](https://github.com/Carcodee/CodeVk_Renderer/blob/main/src/Rendering/Examples/ForwardRenderer.hpp): A straightforward implementation of a forward rendering pipeline.
  - [**Compute Renderer**](https://github.com/Carcodee/CodeVk_Renderer/blob/main/src/Rendering/Examples/ComputeRenderer.hpp): Demonstrates compute shader usage with the RenderGraph.

- **Imgui Integration**.

## Getting Started

### Project Structure

The project is organized into two primary branches: **Main** and **Personal**.

- **Main**: This branch contains a streamlined version of the renderer, stripped of any personal additions. It serves as a lightweight, clean base version of the project.
  
- **Personal**: This branch builds upon the template found in the Main branch, incorporating all of my personal features and customizations.

### Prerequisites

- **C++20 Compiler**: A modern C++ compiler supporting C++20 features is required.
- **CMake**: Version 3.26 or later.
- **Vulkan SDK**: Ensure you have the latest version of the Vulkan SDK installed.

### Building the Project

1. Clone the repository.
   ```sh
   git clone https://github.com/Carcodee/CodeVk_Renderer.git
   ```
2. Create a build directory and run CMake.
   ```sh
   mkdir build && cd build
   cmake ..
   ```
3. Compile the project.
   ```sh
   make
   ```
   
## Dependencies

CodeVk_Renderer relies on several third-party libraries, including:

- **Vulkan**: Graphics API
- **GLFW**: For window management and input.
- **GLM**: For linear algebra and math operations.
- **tinygltf**: For loading GLTF models.
- **ImGui**: For graphical user interface support.

All dependencies can be resolved through the `CMakeLists.txt` included in the repository.

### Directory Structure

```
CodeVk_Renderer/
  |- src/
    |- Core/                # Core Vulkan setup (instance, device, etc.)
    |- Rendering/           # RenderGraph and rendering utilities
      |- Examples/          # Example renderers
    |- Systems/             # Utility systems (OS, Shaders, ModelLoader, etc.)
  |- shaders/               # SPIR-V shaders
  |- assets/                # Assets like models and textures
  |- include/               # Header files
  |- build/                 # Build directory (not included in repo)
```

