# C Software Rasterizer

A simple C software for real-time rasterization, featuring models made in Blender.

![cube-steps](https://github.com/Nico-Posateri/c-software-rasterizer/assets/141705409/4f530095-8d02-4f2c-9747-43af5d66acf9)
<sup>A cube showcasing a wireframe volume, backface culling, shaded faces, affine texture mapping, perspective corrected textures, and correctly inverted UVs. The texture featured is an unused asset produced by **Rare Limited** for *Banjo Tooie* on the N64.</sup>

![jet-steps](https://github.com/Nico-Posateri/c-software-rasterizer/assets/141705409/89d56d30-f8f9-4136-a446-62b205fac922)
<sup>A jet showcasing a more complex, textured mesh rotating. F22 model and texture created by Artem Katrych.</sup>

> [!NOTE]
> I am planning on implementing parallel and subpixel rasterization before uploading this project.

## Directory Information

This engine uses [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-2.28.5), as well as [elanthis' upng](https://github.com/elanthis/upng), a png decoder for the custom jet texture created by Artem Katrych.

## Additional Information

[Computer Graphics Programming course](https://pikuma.com/courses/learn-3d-computer-graphics-programming) taught by [Gustavo Pezzi](https://github.com/gustavopezzi).
