# OpenGL Desert Scene
![](https://github.com/tcollyer1/comp3016_cw2_desert/blob/main/media/preview.png)

In summary, this project is a desert scene created for COMP3016 coursework 2, written primarily using OpenGL, GLSL and C++, in Visual Studio 2022.

It simulates a small, procedurally generated desert, complete with lighting and audio incorporated into a day/night cycle, and 3 main sub-biomes - sand (main desert), oasis and grassy regions, each with differing properties (types of models etc.). The scene can be freely explored, with two different modes - fly and walk.

In order to launch, either build and run the code directly from .sln file in Visual Studio (2022), or build the code and find **Desert.exe** in the `\x64\Debug` directory (or Release). You can then freely move around the scene using the controls provided below. The project includes audio, so speakers/a headset is recommended.

> [!NOTE]
> The libraries listed further below must be located in `C:\Users\Public\OpenGL\include` for the headers, and `C:\Users\Public\OpenGL\lib` for library files.
> 
> `mesh.h`, `model.h`, `shader.h` and `shader_m.h` from LearnOpenGL must be located in `C:\Users\Public\OpenGL\include\learnopengl` alongside other libraries.

## Controls
| Control | Action |
| ----------- | ----------- |
| Move mouse | Look around |
| `W/A/S/D` | Movement |
| `Esc` | Close |
| `V` | Enable fly mode |
| `B` | Enable walk mode |
| `Shift` | Sprint (both fly/walk mode) |

## Overview & Code Structure
Many procedural terrain generation-style projects have been attempted using OpenGL - some of which are also deserts. However, the vast majority of the time the desert features nothing more than sandy dunes, or focuses only on a desert oasis. The goal with this project was to incorporate different elements of a desert, including not just sandy dunes but also grassy areas and multiple desert oasis. In addition, I wanted to experiment with both lighting and audio to learn how to create a slightly more immersive experience, so the base idea of having a day/night cycle was put in place to allow exploration in this area. It allowed me to learn a variety of new concepts - including normal mapping, the different stages of lighting (ambient, diffuse and specular), how to time animations within the render loop, how to use both 2D and 3D sound and more.

The code is structured using an object-oriented approach and is divided up into multiple classes.
- The `Display` class handles GLFW window creation and manipulation.
- The `ShaderInterface` class acts as a base class to handle common interaction with the shaders - primarily for sending light and camera information. `Light`, `ModelSet` and `Terrain` inherit from it.
- The `Terrain` class handles generating and drawing the terrain.
- The `Light` class handles generating, drawing and moving the light source around the scene.
- The `ModelSet` class keeps track of all the models on the scene and handles their drawing.
- The `Camera` class keeps information on the camera position, as well as handling mouse movement and user input, serving as a class to interface the user with the rest of the scene. It keeps a copy of the `Terrain` object so it can update it with the current user position for 3D audio purposes, and also to be able to get the camera's Y position depending on the terrain height for ground traversal.
- The `MVP` class manages the MVP matrix throughout runtime, including handling model transforms or view/projection updates.
- The `TerrainTexture` class is responsible for generating textures to be used on the terrain, and is used by the `Terrain` class.
- The `VAO`, `VBO` and `IBO` are all self-contained buffer classes used to manage vertex array objects, vertex buffer objects and index buffer objects. VBOs and IBOs can only be manipulated by a VAO. VAOs are used by other classes, such as `Terrain` and `Light` to store the vertices, indices, colours, texture coordinates of everything in the scene on the GPU.

## Sources & Libraries
### Libraries
- Assimp (model loading)
- LearnOpenGL for model & shader loading header files (see note above)
- GLAD
- GLFW3
- GLM
- irrKlang (audio)

### Resources and Models
- [ZapSplat](https://www.zapsplat.com) (audio)
- [TurboSquid](https://www.turbosquid.com) and [Sketchfab](https://sketchfab.com) (models)
  - [Palm tree model](https://www.turbosquid.com/3d-models/cordyline-indivisa-free-3d-model-2117614)
  - [Cactus model](https://sketchfab.com/3d-models/cactus-saguaro-d98fcb4869c6413f943fb127d60edb6e) (licence in `media/cactus`)
  - [Grass model](https://sketchfab.com/3d-models/grass-bushes-game-ready-asset-aa1fa43af9bf4228adfddb54ea123ad1) (licence in `media/grass`)

## Key Features
- Object-oriented design approach
- Different biomes
- Two different modes (fly/walk) for different traversal
- Model-view-projection implementation
- Full first person mouse & keyboard movement
- Procedurally generated terrain - including Perlin noise-generated height maps and noise used for model placement
- Fully textured terrain with automatically generated texture coordinates
- Multiple models (tree, cactus and grass) - in varying formats (.obj, .gltf) and textured
- BlinnPhong lighting - full ambient, diffuse and specular lighting is applied to both models using their normal maps, and also normal maps that are fully generated for the terrain
- Scene animations - day/night cycle. The light source will orbit the terrain to simulate the movement of the sun, which, depending on the time of day (the light position) will determine the colour of the lighting, the background colour of the scene and additionally the background sound effects which change continuously. This also influences the lighting of every other object on the scene.
- Audio - background audio changes dynamically throughout the scene depending on the time of day
  - Sound effects are also present in walk mode for the user's footsteps, and will change depending on the type of terrain being walked on
  - 3D audio is also utilised - one of the trees on the scene at random has a 3D bird sound effect that will pan depending on listening direction, and grow louder/quieter depending on the user's position

# Biomes
Since a plain desert alone is too unvaried, it was split into 3 main sub-biomes.

## Sand Biome
The sand biome is characterised by large expanses of plain sand - much like a real desert. This biome additionally features winding pathways, marked by a darker sand texture generated from turbulence noise - which uses the absolute values from a Perlin noise map to give this effect.

## Grassy Biome
This sub-biome is marked by a grassy texture and is found at high points on the terrain. This simulates the rarer grassy regions that can sometimes be found in a desert, and this is where cacti and grass can be found to populate these regions.

## Oasis Biome
These can be found at the low points on the terrain. They are characterised by the pools of water that can have both grass and trees surrounding their borders - but never inside the water.
