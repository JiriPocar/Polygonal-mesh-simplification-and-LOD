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

## TODO's

- [x] Basic Vulkan setup
- [x] Resizable window via ```glfw```
- [x] Model loading via ```tinygltf```
- [x] Rotate and show model + basic camera movement 
- [x] Fix window resizing with loaded model (far plane)
- [x] Basic ```ImGui``` integration
- [x] Performance statistics via ```ImGui```
- [x] Dynamic model loading via ```ImGui```
- [x] Bounding box compute (autoscaling of a model)
- [x] Connect mouse movement to camera rotation
- [ ] Setup data structures for LOD manipulation
- [ ] Simplification algorithms for LOD generation
	- [ ] Quadric Error Metrics
	- [ ] Edge Collapse
	- [ ] Vertex Clustering
	- [ ] Progressive Meshes
	- [ ] Own implementation of simplification algorithms
- [ ] Demo application / applications
	- [ ] LOD switching
	- [ ] Performance comparison
	- [ ] Visual comparison
	- [ ] Moving scene(s) with dynamic LOD adjustment