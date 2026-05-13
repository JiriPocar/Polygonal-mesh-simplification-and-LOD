# Polygonal mesh simplification and rendering optimization using Level of Detail techniques

## Foreword
This repository contains the implementation of a bachelor's thesis at **Brno University of Technology, Faculty of Information Technology** focused on polygonal mesh
simplification and rendering optimization using Level of Detail (LOD) techniques. The thesis is supervised by **Ing. Jan Pečiva, Ph.D.**

The project is developed in ```C++``` using the ```Vulkan API``` for rendering and includes various simplification
algorithms, an application for polygonal mesh simplification and an application for benchmarking
the effects of Level of Detail techniques and GPU-driven rendering.

- Once published, the thesis in Czech can be found [here](https://www.vut.cz/studenti/zav-prace/detail/172919).
- The project is published in the **Vulkan-FIT** repository, which can be found [here](https://github.com/Vulkan-FIT).


## Simplificator
The simplification application (```Simplificator```) can be freely used for mesh simplification.

![SIMPLIFICATOR](images/simplificator.PNG)

### Features
- Simple ```glTF``` model loading with a **single** base texture.
- Per-mesh simplification using various algorithms and metrics:
	- ```QEM```
	- ```Vertex Clustering```
	- ```Floating Cell Clustering```
	- ```Vertex Decimation```
	- ```Naive approaches``` (for comparison purposes)
- Wireframe mode and first person camera for better visualization of the results.
- Hausdorff distance and MSE (Mean Squared Error) metrics for geometric quality evaluation.
- Interactive GUI for adjusting simplification parameters and visualizing the results.
- Flat shading or smooth shading of the simplified model.
- Export of the simplified model to a simple ```.obj``` file and loading it back in for further simplifications.

### Limitations and known issues
- ```Simplificator``` works best with manifold meshes for geometric simplification.
- For textured models, only ```QEM``` algorithm is currently supported via ```Lock UV Seams```
and ```Simplify with UV Seams``` options. If there are no UV seams in the textured model, the simplification
may not preserve the texture coordinates properly.
- Hausdorff distance and MSE metrics are not optimized and may take longer for complex models.
- Damaged models (ie. models with scattered meshes) are not supported by most of the simplification algoritms
and may create holes in the mesh during simplification proccess.
- In rare cases, some algorithms may produce non-optimal geometric results.
- In some cases, visual artifacts with triangle normals can occur.


## Spiral scene
The benchmarking application (```SpiralApp```) is designed to demonstrate the performance benefits
of LOD techniques and GPU-driven rendering. Focused on the geometry through-put of hardware configurations

![SPIRAL](images/spiral.PNG)

### Features
- Rendering of a large number of instances with different LOD levels based on the camera distance.
- Customizable spiral parameters.
- Scene metadata display.
- Customizable LOD configuration for instances of the model on the spiral.
- GPU-driven rendering using ```compute shader``` for instance spiral positions and LOD selection.
- Wireframe mode with instances colored by their LOD level for better visualization of the LOD distribution.
- Auto-calibrated benchmark setup for performance evaluation of different hardware configurations.
- Semi-automated graph plotting of the benchmark results.

### Limitations and known issues
- As of now, the AMD graphics cards provide higher instance performance in the benchmarks than NVIDIA ones. This is most likely due to architecture (see [RDNA2's NGG](https://timur.hu/blog/2022/what-is-ngg), which most likely does a shader culling). Tested on several different hardware configurations.
- Instance positioning can become unstable when changing the spiral parameters and not resetting the animation time.
- The displayed *Drawn triangles* statistic while choosing the GPU modes is a quite precise estimation, but estimation nevertheless, and can slightly differ from the actual number of triangles drawn by the GPU.


## Controls
- ```N``` lock  the camera
- ```M``` unlock the camera
	- ```WASD``` movement
	- ```SHIFT``` move down
	- ```SPACE``` move up
	- ```DOWN ARROW``` descrease camera speed
	- ```UP ARROW``` increase camera speed
- ```U``` disable UI
- ```ESC``` exit the application

# Requirements

- Windows 10 or later.
- Visual Studio 2022 or later with MSVC compiler that supports C++17 and CMake extension.
- Vulkan-compatible GPU and drivers.
- ```Vulkan SDK``` [found here](https://vulkan.lunarg.com/sdk/home). Make sure to restart Visual Studio after the installation.
- ```CMake``` (version 3.10.2 or later) for building the project
- *(optional)* Python and ```matplotlib```, ```pandas``` and ```numpy``` libraries for performance plotting, if desired.

# Building the project

Make sure to see [requirements](#requirements)

## In Visual Studio
1. Clone the repository.
2. Open the cloned repository in Visual Studio.
3. Wait for Visual Studio to automatically generate the CMake cache.
4. The `Simplificator` and `SpiralApp` applications are separate CMake targets and you can select which one to build in the Visual Studio.
5. Once building is complete, you can run the executables directly from Visual Studio or from the build directory.
6. You can add more 3D models to the output `/assets` directory, or add them to the source `/assets` directory and re-configure CMake to copy them over to the output directory.

## In command line
1. Open ```cmd```
2. Navigate to the project folder
3. Run ```cmake -S . -B build -G "Visual Studio 17 2022" -A x64``` (or other Visual Studio release as such)
4. Run ```cmake --build build --config Release```
5. Navigate to ```build/src/apps``` from where you should see ```spiral/``` and ```simplificator``` folders
6. Run the ```.exe``` for the application to run

If you don't want to bother with compiling, you can simply download the pre-compiled binaries from the [Releases](https://github.com/JiriPocar/Polygonal-mesh-simplification-and-LOD/releases) page.

# Used libraries
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- [GLFW](https://github.com/glfw/glfw)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [TinyGLTF](https://github.com/syoyo/tinygltf)
- [GLM](https://github.com/g-truc/glm)

# Acknowledgements

- During the development, a large language model *Google Gemini 3* was used for programming tasks, namely tracking rather difficult bugs, refactoring and optimization suggestions and general help with the used libraries. GitHub Copilot was used for code completions and syntax corrections.
- The Vulkan abstraction layer (```core/```, ```rendering/``` and ```resources/```) is based on the public Vulkan tutorials, namely Alexander Overvoorde's [Vulkan Tutorial](https://vulkan-tutorial.com/) and Victor Blanco's [Vulkan Guide](https://vkguide.dev/).
- Other specific resources, if used, are either specified in the headers of the source (```.cpp```) files or in the adapted parts of the code itself. 

# Assets table
Several models were used for testing the simplification algorithms and benchmarking the rendering performance. Below is a table listing the assets and their sources.

| Asset | Source | Author | Licence |Note |
| ------------- | ------------- | --------- | ---- | ---- |
| Duck  | [Khronos repository](https://github.com/KhronosGroup/glTF-Sample-Models/tree/main/2.0/Duck/glTF)|KhronosGroup| ```SCEA 1.0```| Textured with UV seams |
| Suzanne | [Kronos repository](https://github.com/KhronosGroup/glTF-Sample-Models/tree/main/2.0/Suzanne) |Norbert Nopper|```none (donated to the repository by Norbert Nopper)```| Polygon soup with texture|
| Stanford bunny | [SketchFab](https://sketchfab.com/3d-models/stanford-bunny-43f266d6cd6e4c6888b9943557528c0f)|darwinsenior|```CC BY 4.0```| Polygon soup |
| Happy Buddha | [SketchFab](https://sketchfab.com/3d-models/happy-buddha-stanford-5f2a444ff26c4a3bb194f6d79502ee54) |3D graphics 101|```CC BY-NC 4.0```| Partly damaged mesh |
| Skull | [SketchFab](https://sketchfab.com/3d-models/skull-downloadable-1a9db900738d44298b0bc59f68123393) |martinjario|```CC BY 4.0```| Textured with uv seams |
| Male Body | [SketchFab](https://sketchfab.com/3d-models/male-body-base-mesh-highpoly-9311f4f8fa1a4fe4bb0027ff7e8fd795) |Mandrake|```CC BY 4.0```| Fixed in Blender|
| Boot | [SketchFab](https://sketchfab.com/3d-models/caterpillar-work-boot-d551ce74dcd24528a05cbb0f4b7434d7) |inciprocal| ```CC BY 4.0```| Stripped off of the 4k PBR textures |
| sphere | Exported from Blender |```none```| ```none``` | Partly damaged mesh |

# Citation
If you find this code or the benchmark results useful in your academic or professional work, please consider citing the thesis as follows:

```
@thesis{pocarovsky:2026:thesis,
	author		= {Počarovský, Jiří},
	title		= {Polygonal Mesh Simplification and Rendering Optimization
				   Using Level of Detail Techniques},
	address		= {Brno},
	school		= {Brno University of Technology, Faculty of Information Technology},
	year		= {2026},
	type		= {Bachelor's thesis},
	supervisor	= {Ing. Jan Pečiva, Ph.D.},
	url			= {https://www.vut.cz/studenti/zav-prace/detail/172919}
}

@misc{pocarovsky:2026:program,
	author			= {Počarovský, Jiří},
	title			= {Polygonal Mesh Simplification and Rendering Optimization
				       Using Level of Detail Techniques - Code Repository},
	year			= {2026},
	month			= {5},
	url				= {https://github.com/JiriPocar/Polygonal-mesh-simplification-and-LOD}
	howpublished	= {online}
}
```
