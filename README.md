# Level of detail in huge scenes

## Requirements

- Vulkan SDK [found here](https://vulkan.lunarg.com/sdk/home). Make sure to restart Visual Studio after the installation.

## Project structure

```
── BP Pocarovsky
|   |── assets
│   ├── external
│   ├── src
│   │   ├── common
│	│   │   ├── core
│	│   │   │   ├── Device.hpp/cpp
│	│   │   │   ├── Instance.hpp/cpp
│	│   │   │   ├── Pipeline.hpp/cpp
│	│   │   │   ├── Swapchain.hpp/cpp
│	│   │   ├── rendering
│	│   │   │   ├── CommandManager.hpp/cpp
│	│   │   │   ├── Descriptors.hpp/cpp
│	│   │   │   ├── FrameBuffer.hpp/cpp
│	│   │   │   ├── Renderer.hpp/cpp
│	│   │   │   ├── RenderPass.hpp/cpp
│	│   │   │   ├── UniformBuffer.hpp/cpp
│	│   │   ├── resources
│	│   │   │   ├── Buffer.hpp/cpp
│	│   │   │   ├── DualModel.hpp/cpp
│	│   │   │   ├── Model.hpp/cpp
│	│   │   ├── scene
│	│   │   │   ├── Camera.hpp/cpp
│	│   │   │   ├── Scene.hpp/cpp
│	│   │   │   └── Transform.hpp/cpp
│	│   │   ├── simplification
│	│   │   │   ├── Simplificator.hpp/cpp
│	│   │   │   └── simplificationUtil.hpp/cpp
│	│   │   ├── ui
│	│   │   │   └── ui.hpp/cpp
│	│   │   ├── shaders
│	│   │   │   ├── shader.vert
│	│   │   │   └── shader.frag
│	│   │   └── window.h/cpp
│   │   ├── apps
│   │   │   │── demo
│   │   │   │   ├── main.cpp
│   │   │   │   └── CMakeLists.txt
│   │   │   └── simplificator
│   │   │       ├── main.cpp
│   │   │       └── CMakeLists.txt
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
		- [ ] TODO: add more vertex candidates OR compute optimal position 
	- [ ] Edge Collapse
	- [x] Vertex Clustering
		- [ ] TODO: can be optimized 
	- [x] Naive simplification
		- [ ] TODO: optimize this 
	- [ ] Progressive Meshes
	- [ ] Own implementation of simplification algorithms
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
		- [ ] (optional) Street tiles with multiple models
		- [x] Spiral scene with moving parts
		- [ ] (optional) Ant in a maze
		
- [ ] General TODO's
	- [ ] Texture loading and simplification integration of textures

# Known issues

- On first run, the application might crash when loading a model. Restarting the application resolves the issue.