# Level of detail in huge scenes

## Requirements

- Vulkan SDK [found here](https://vulkan.lunarg.com/sdk/home). Make sure to restart Visual Studio after the installation.

## Project structure

```
BP Pocarovsky
├── assets
├── external
├── src
│   ├── common
│   │   ├── core
│   │   │   ├── Device.hpp/cpp
│   │   │   ├── Instance.hpp/cpp
│   │   │   ├── Pipeline.hpp/cpp
│   │   │   ├── SpiralPipline.hpp/cpp
│   │   │   └── Swapchain.hpp/cpp
│   │   ├── rendering
│   │   │   ├── CommandManager.hpp/cpp
│   │   │   ├── Descriptors.hpp/cpp
│   │   │   ├── FrameBuffer.hpp/cpp
│   │   │   ├── Renderer.hpp/cpp
│   │   │   ├── SpiralRenderer.hpp/cpp
│   │   │   ├── RenderPass.hpp/cpp
│   │   │   └── UniformBuffer.hpp/cpp
│   │   ├── resources
│   │   │   ├── Buffer.hpp/cpp
│   │   │   ├── DualModel.hpp/cpp
│   │   │   ├── Model.hpp/cpp
│   │   │   └── Textures.hpp/cpp
│   │   ├── scene
│   │   │   ├── Camera.hpp/cpp
│   │   │   ├── Scene.hpp/cpp
│   │   │   ├── SpiralScene.hpp/cpp
│   │   │   └── Transform.hpp/cpp
│   │   ├── simplification
│   │   │   ├── Simplificator.hpp/cpp
│   │   │   ├── utils
│   │   │   │    ├── Geometry.hpp/cpp
│   │   │   │    └── Topology.hpp/cpp
│   │   │   └── algorithms
│   │   │        ├── Naive.hpp/cpp
│   │   │        ├── QEM.hpp/cpp
│   │   │        ├── VertexClustering.hpp/cpp
│   │   │        └── VertexDecimation.hpp/cpp
│   │   ├── ui
│   │   │   └── ui.hpp/cpp
│   │   └── window.h/cpp
│   └── apps
│       ├── demo
│       │   ├── shaders
│       │   │   ├── frag.spv
│       │   │   └── vert.spv
│       │   ├── CMakeLists.txt
│       │   └── main.cpp
│       └── simplificator
│           ├── shaders
│           │   ├── frag.spv
│           │   └── vert.spv
│           ├── CMakeLists.txt
│           └── main.cpp
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
- [x] Implement split-screen render for original and simplified model  
- [x] Setup data structures for LOD manipulation
- [ ] Simplification algorithms for LOD generation
	- [x] Quadric Error Metrics
		- [x] Research
		- [ ] Optimized 
		- [x] Compute optimal position 
	- [x] Vertex Clustering
		- [x] Research
		- [x] Optimized
	- [x] Floating-cell Clustering
		- [x] Research
		- [ ] Optimized
		- [x] Implementation
	- [x] Naive simplification
		- [ ] Optimized
	- [ ] Vertex decimation
		- [x] Research
		- [ ] Implementation
		- [ ] Triangulating research
		- [ ] Optimized
	- [ ] Progressive Meshes
		- [ ] Research
- [ ] Demo application
	- [x] Remake project structure for multiple demo apps support 
	- [x] CPU LOD switching
		- [x] Switch LOD levels based on distance from the main camera
		- [ ] Switch LOD levels based on screen size of the model
	- [ ] GPU LOD switching
		- [ ] Switch LOD levels based on distance from the main camera 
	- [ ] Implement popping effect reduction techniques
		- [ ] Cross fading
	- [ ] Performance comparison
		- [ ] Save performance statistics to file
		- [ ] Create graphs using ```python```, ```matplotlib```, ...
	- [ ] Visual comparison
	- [ ] Moving scene(s) with dynamic LOD adjustment
		- [x] Spiral scene with moving parts
		- [ ] Simple scene with car driving through "mountains"
		
- [ ] General TODO's
	- [x] Texture loading
	- [ ] UV interpolation for texture coordinates in simplified models
	- [ ] QEM optimization via priority queue
	- [ ] Extended QEM with texture coordinates and normals
	- [ ] Pick a set of testing models
	- [ ] UI improvements
		- [x] Debug (result) window
		- [ ] Spiral scene - LOD threshold controls
		- [x] Enable / disable UI option
	- [ ] Performance research

# Known issues

- [FIXED] On first run, the application might crash when loading a model. Restarting the application resolves the issue.

# Assets table

| Asset  | Source |
| ------------- | ------------- |
| Duck  | [Khronos repository](https://github.com/KhronosGroup/glTF-Sample-Models/tree/main/2.0/Duck/glTF)  |
| x | y |