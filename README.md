# Level of detail in huge scenes

## Requirements

- Vulkan SDK [found here](https://vulkan.lunarg.com/sdk/home). Make sure to restart Visual Studio after the installation.

## Project structure

```
── BP Pocarovsky
|   |── assets
│   ├── external
│   ├── src
│   │   ├── core
│   │   │   ├── Device.hpp/cpp
│   │   │   ├── Instance.hpp/cpp
│   │   │   ├── Pipeline.hpp/cpp
│   │   │   ├── Swapchain.hpp/cpp
│   │   ├── rendering
│   │   │   ├── CommandManager.hpp/cpp
│   │   │   ├── Descriptors.hpp/cpp
│   │   │   ├── FrameBuffer.hpp/cpp
│   │   │   ├── Renderer.hpp/cpp
│   │   │   ├── RenderPass.hpp/cpp
│   │   │   ├── UniformBuffer.hpp/cpp
│   │   ├── resources
│   │   │   ├── Buffer.hpp/cpp
│   │   │   ├── Mesh.hpp/cpp
│   │   │   ├── Model.hpp/cpp
│   │   ├── scene
│   │   │   ├── Camera.hpp/cpp
│   │   │   ├── Scene.hpp/cpp
│   │   │   ├── Transform.hpp/cpp
│   │   ├── shaders
│   │   │   ├── shader.vert
│   │   │   └── shader.frag
│   │   ├── main.cpp
│   │   └── window.h/cpp
├── CMakeLists.txt
├── LICENSE.txt
└── README.md
```