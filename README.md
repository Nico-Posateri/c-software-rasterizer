# C Software Rasterizer

A simple C software for real-time rasterization, featuring models made in Blender.

![cube-steps](https://github.com/Nico-Posateri/c-software-rasterizer/assets/141705409/4f530095-8d02-4f2c-9747-43af5d66acf9)
<sup>A cube showcasing a wireframe volume, backface culling, shaded faces, affine texture mapping, perspective corrected textures, and correctly inverted UVs. The texture featured is an unused asset produced by **Rare Limited** for *Banjo Tooie* on the N64.</sup>

![jet-steps](https://github.com/Nico-Posateri/c-software-rasterizer/assets/141705409/89d56d30-f8f9-4136-a446-62b205fac922)
<sup>A jet showcasing a more complex, textured mesh rotating. F22 model and texture created by Artem Katrych.</sup>

![jet-clip](https://github.com/Nico-Posateri/c-software-rasterizer/assets/141705409/2e09ad29-9f37-4e38-988c-6d4a6294d991)
<sup>A jet showcasing the implementation of clipping, camera manipulation, and a custom render resolution for mid-90s flair.</sup>

<sup>A Kamurochō meal showcasing the implementation of multiple textured meshes. Ramen has never looked so good.</sup>

> [!NOTE]
> I am planning on implementing parallel and subpixel rasterization before uploading this project, as well as support for multiple meshes.

## Directory Information

This engine uses [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-2.28.5), as well as [elanthis' upng](https://github.com/elanthis/upng), a png decoder for custom textures. By default, the program will render a static cube with no texture.

The following is a list of key inputs and their functions:
- **esc** - Closes the program
- **w** - Moves the camera forward
- **a** - Rotates the camera left
- **s** - Moves the camera backward
- **d** - Rotates the camera right
- &#8593; - Raises the camera
- &#8595; - Lowers the camera
- **c** - Culls backfaces
- **r** - Renders backfaces
- **1** - Renders the mesh wireframe with vertices
- **2** - Renders the mesh wireframe
- **3** - Renders the mesh with filled faces
- **4** - Renders the mesh wireframe with filled faces
- **5** - Renders the mesh wireframe with filled faces and vertices
- **6** - Renders the mesh textured
- **7** - Renders the mesh textured with a wireframe
- **8** - Renders the mesh textured with a wireframe and vertices

## Additional Information

[Computer Graphics Programming course](https://pikuma.com/courses/learn-3d-computer-graphics-programming) taught by [Gustavo Pezzi](https://github.com/gustavopezzi).
