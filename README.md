# CodeVkRenderer: A Vulkan-Based RenderGraph Renderer

CodeVk_Renderer is a lightweight Vulkan-based rendering engine utilizing a RenderGraph architecture. It aims to simplify the development of graphics applications by providing an easy-to-use interface for rapid prototyping while maintaining the flexibility and control of Vulkan's explicit API.

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

# CodeVk_Renderer: A Vulkan-Based RenderGraph Renderer

CodeVk_Renderer is a lightweight Vulkan-based rendering engine that leverages a RenderGraph architecture. It aims to simplify the development of graphics applications by providing an easy-to-use interface for rapid prototyping while maintaining the flexibility and control of Vulkan's explicit API.

## Code Usage

### Basic Usage

To get started with developing your own renderer, follow these steps:

1. **Create a Renderer Class**
   
   Inherit from `BaseRenderer` and create your own renderer (e.g., `ForwardRenderer`). You can use the provided examples as references.

   ```cpp
   class ForwardRenderer : public BaseRenderer {
       public:
           ForwardRenderer(ENGINE::Core* core, WindowProvider* windowProvider,
                          ENGINE::DescriptorAllocator* descriptorAllocator);
           ~ForwardRenderer() override;
           void SetRenderOperation(ENGINE::InFlightQueue* inflightQueue) override;
           void ReloadShaders() override;
   };
   ```

2. **Initialize Buffers, Shaders, and Images**
   
   - **Load Models**: Use `ModelLoader` to load 3D models into the application. For example, you can load a GLTF model and create vertex and index buffers.
   - **Vertex and Index Buffers**: Create these buffers using `ENGINE::Buffer`. The buffers store the vertex data and indices required for rendering your model.
   - **Shaders**: Load vertex and fragment shaders using the `ENGINE::Shader` utility. Paths for these shaders can be configured to point to custom shader files.
   
   ```cpp
   ModelLoader::GetInstance()->LoadGLTF(modelPath, model);
   vertexBuffer = std::make_unique<ENGINE::Buffer>(
       physicalDevice, logicalDevice, vk::BufferUsageFlagBits::eVertexBuffer,
       vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
       sizeof(M_Vertex3D) * model.vertices.size(), model.vertices.data());
   indexBuffer = std::make_unique<ENGINE::Buffer>(
       physicalDevice, logicalDevice, vk::BufferUsageFlagBits::eIndexBuffer,
       vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
       sizeof(uint32_t) * model.indices.size(), model.indices.data());
   ```

3. **Render Pass Configuration**
   
   Define a render pass in the RenderGraph using `AddPass()`. Configure attachments, shaders, and pipeline layout. The `AddPass()` function is used to create a new rendering pass, which specifies how the rendering pipeline is organized and which resources are needed.

   ```cpp
   auto renderNode = renderGraphRef->AddPass(forwardPassName);
   renderNode->SetVertShader(vertShader.get());
   renderNode->SetFragShader(fragShader.get());
   renderNode->SetFramebufferSize(windowProvider->GetWindowSize());
   renderNode->SetPipelineLayoutCI(layoutCreateInfo);
   renderNode->SetVertexInput(vertexInput);
   renderNode->AddColorAttachmentOutput("color", colInfo);
   renderNode->SetDepthAttachmentOutput("depth", depthInfo);
   renderNode->AddColorBlendConfig(ENGINE::BlendConfigs::B_OPAQUE);
   renderNode->SetDepthConfig(ENGINE::DepthConfigs::D_ENABLE);
   renderNode->AddSamplerResource("sampler", imageShipper.imageView.get());
   renderNode->AddStorageResource("storageImage", computeStorage);
   renderNode->BuildRenderGraphNode();
   ```

4. **Set Render Operations**
   
   Define the rendering logic by setting render operations within the `SetRenderOperation()` function, using `vk::CommandBuffer` to bind resources and draw. This includes:
   - Binding descriptor sets.
   - Binding vertex and index buffers.
   - Issuing draw commands for the loaded model.

   ```cpp
   commandBuffer.bindDescriptorSets(renderGraphRef->GetNode(forwardPassName)->pipelineType,
                                    renderGraphRef->GetNode(forwardPassName)->pipelineLayout.get(), 0, 1,
                                    &descriptorCache->dstSet.get(), 0 , nullptr);

   commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer->bufferHandle.get(), &offset);
   commandBuffer.bindIndexBuffer(indexBuffer->bufferHandle.get(), 0, vk::IndexType::eUint32);
   commandBuffer.drawIndexed(model.indicesCount[i], 1, model.firstIndices[i],
                             static_cast<int32_t>(model.firstVertices[i]), 0);
   ```

5. **Recreate Resources as Needed**

   Implement `ReloadShaders()` to support shader reloading. This function allows you to modify shaders and reload them without restarting the entire application, making the development cycle more iterative.

   ```cpp
   void ReloadShaders() override {
       auto renderNode = renderGraphRef->GetNode(forwardPassName);
       renderNode->RecreateResources();
   }
   ```

### Descriptor Handling Methods

CodeVk_Renderer supports two different methods for handling descriptors: **Descriptor Cache** and **Descriptor Writer**. Below, we describe each method and its application in the rendering engine.

#### 1. Descriptor Cache (Automatic Management)

The **Descriptor Cache** approach is used primarily in `ForwardRenderer`. It automatically manages descriptors required by different shaders. This method abstracts away the low-level descriptor management, making it easier to set up a rendering pipeline without repetitive and manual descriptor binding logic.

- **Setup**: The `Descriptor Cache` is configured by extracting the layout information from the shaders, and the cache builds descriptor sets accordingly.
- **Usage**: During the initialization of `ForwardRenderer`, descriptors are populated by calling `descriptorCache->AddShaderInfo()`, followed by `descriptorCache->BuildDescriptorsCache()`. This setup handles the descriptor requirements for vertex and fragment shaders seamlessly.

```cpp
vertShader->sParser->GetLayout(builder);
fragShader->sParser->GetLayout(builder);
descriptorCache->AddShaderInfo(*vertShader->sParser.get());
descriptorCache->AddShaderInfo(*fragShader->sParser.get());
descriptorCache->BuildDescriptorsCache(descriptorAllocatorRef, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
```

#### 2. Descriptor Writer (Manual Management)

The **Descriptor Writer** approach is used in `ComputeRenderer`. This method provides explicit control over how descriptors are allocated and written to, giving developers more fine-grained control over the descriptor lifecycle.

- **Setup**: The `Descriptor Writer` uses a builder (`writerBuilder`) to create descriptor sets manually. This approach involves creating descriptor layouts and using a descriptor allocator to allocate descriptor sets.
- **Usage**: In `ComputeRenderer`, descriptors are configured using `writerBuilder.AddWriteImage()`, and the set is updated explicitly with `writerBuilder.UpdateSet()`. This allows full control over each descriptor’s configuration and usage.

```cpp
compShader->sParser.get()->GetLayout(builder);
dstLayout = builder.BuildBindings(logicalDevice, vk::ShaderStageFlagBits::eCompute);
dstSet = descriptorAllocatorRef->Allocate(core->logicalDevice.get(), dstLayout.get());
writerBuilder.AddWriteImage(0, computeImageView.get(), computeImageSampler->samplerHandle.get(), vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage);
writerBuilder.UpdateSet(core->logicalDevice.get(), dstSet.get());
```

### Example: Forward Renderer Setup

Here's a detailed example of how `ForwardRenderer` is set up:

1. **Initialization**
   - The `ForwardRenderer` class initializes by loading a GLTF model and creating Vulkan buffers (`vertexBuffer` and `indexBuffer`).
   - Default image and shader paths are established for the resources required.
   - The descriptor cache is configured to manage the shader's descriptor requirements automatically.

2. **Render Pass Setup**
   - A RenderGraph node (`AddPass`) is created to add the rendering pass with attachments and pipeline configurations.
   - Vertex and fragment shaders are loaded, and descriptors are configured for managing resource bindings.

3. **Render Operation Logic**
   - In `SetRenderOperation()`, use the `vk::CommandBuffer` to bind resources, issue draw commands, and execute the rendering of the GLTF model.

4. **Swapchain Handling**
   - Handles recreating the swapchain and updating framebuffer resources if the window is resized or the swapchain becomes invalid.


## Hot Reloading Shaders

To reload shaders during runtime, press the `R` key or invoke the `ReloadShaders()` function in `ForwardRenderer`. This enables quick iteration and testing of shader modifications without restarting the application.


### Customization Tips

- **Shaders**: Add your custom shaders under the shaders path and update `ForwardRenderer` to use them.
- **Models**: Replace the GLTF model path to use your own models.
- **UI**: Use the ImGui integration to add custom UI elements for controlling your scene.

### Contribution

Contributions are welcome! Feel free to submit issues, bug fixes, or feature requests via GitHub. Pull requests are encouraged to improve the engine, especially around its modular Vulkan setup and the RenderGraph optimizations.

