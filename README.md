# CodeVkEngine

CodeVkEngine Is my personal graphics engine that I extend from this [**public template**](https://github.com/Carcodee/Vulkan-Renderer-Template). Here I implement diferent rendering techniques such as, Radiance Cascades in 2d, GPU oclussion culling, Cluster Rendering and more.

## Features

- **RenderGraph-Based Architecture**: Flexible setup of rendering pipelines designed to facilitate rapid prototyping. This system is functional but still undergoing optimization for improved performance.

- **Automatic Descriptor Set handling**

- **Visual Pipeline Node Editor**: A node-based editor designed to create render nodes (shaders), enabling rapid prototyping and iteration of rendering techniques.

- **Modular Vulkan Interface**: Built with modularity in mind, allowing rapid iteration and customization (including automatic descriptor setup).
  
- Shaders hot reload/reflection system.

- **GLTF Loader**: Import and render GLTF models with ease, streamlining 3D asset integration.

- **Simple First-Person Camera**: Includes a basic first-person camera controller.
  
- **Imgui Integration.**

- **Eficcient Resource Manager.**


- **Renderers**:
  - [**Cluster Renderer**](https://github.com/Carcodee/CodeVk_Renderer/blob/main/src/Rendering/Renderers/ClusterRenderer.hpp): Implements GPU occlusion culling for point lights (supporting up to 1000) and meshes, leveraging GPU-driven rendering and position reconstruction from a depth map for optimized performance.
https://github.com/user-attachments/assets/936df445-45d0-4f93-bba5-baa3ce272bb2
  - [**2D Renderer Renderer**](https://github.com/Carcodee/CodeVk_Renderer/blob/main/src/Rendering/Renderers/FlatRenderer.hpp): Responsible for rendering all 2D techniques, currently featuring a basic 2D paint canvas and an implementation of Radiance Cascades.
https://github.com/user-attachments/assets/5355f3d6-e4c6-4743-8de7-51b87b09fbe3
https://github.com/user-attachments/assets/0aa9c22a-2a91-41ed-8d49-fb49fab54940
https://github.com/user-attachments/assets/02fb8ed3-b220-4eec-92b4-bf2a60b959b9
  - [**Debug Renderer**](https://github.com/Carcodee/CodeVk_Renderer/blob/main/src/Rendering/Renderers/FlatRenderer.hpp): Handles the rendering of all meshes added to this renderer. Currently supports debugging meshes using wireframe or point mesh modes. In the provided example, it is debugging the camera frustum, which influences mesh rendering.
https://github.com/user-attachments/assets/634bb33c-9047-4c51-8751-4d1b9f596929



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
- **GLTF**: For standarized 3D models format loading.
- **SpirvCross**: For shader reflection.
- **Slang**: For modern shader development
- **tinygltf**: For loading GLTF models.
- **ImGui**: For graphical user interface support.
- **LegitProfiler**: For visual profiling.


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

