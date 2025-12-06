# Another game engine try
- 2D space game inspired by Destination Sol [\[website\]](https://destinationsol.org/)[\[steam (free)\]](https://store.steampowered.com/app/342980/Destination_Sol/)[\[github\]](https://github.com/MovingBlocks/DestinationSol)

# Build and run
Requirements:
- CMake
- [GLFW3](https://www.glfw.org/download)
- [GLM](https://github.com/g-truc/glm?tab=readme-ov-file#cmake-using-fetchcontent)
- [spdlog](https://github.com/gabime/spdlog?tab=readme-ov-file#package-managers)
- any drivers supporting opengl 3.3

```sh
git clone git@github.com:litarin1/turned.git
cd turned
mkdir build
cd build
cmake ../
cmake --build . && ./main
```
Use ninja to build:
```sh
mkdir build_ninja
cd build_ninja
cmake ../ -G Ninja
cd ../
cmake --build build_ninja && ./build_ninja/main
```
 See also [BACKLOG.md](BACKLOG.md)
