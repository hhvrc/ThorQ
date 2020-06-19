# CollarControl

Server and Client to control a shock-collar remotely from anywhere in the world.

##### Why?
- I want to be domesticated UwU
- Why not

## Setup
### Linux
```console
$ git clone https://github.com/hhvrc/TestingRep.git
$ cd TestingRep
$ git submodule init
$ git submodule update
$ mkdir build
$ cd build
$ cmake ..
$ make -4
```
### Windows
```powershell
> git clone https://github.com/hhvrc/TestingRep.git
> cd TestingRep
> git submodule init
> git submodule update
```
Open Visual Studio 2019

Select "Open Directory"

Choose the cloned directory, and click OK

CMake will now configure the project

Then do Ctrl+Shift+B, or click "Build all" to build

## Libraries used
| Use Case | Name | Link |
| ------ | ------ | ------ |
| Networking | ENet | https://github.com/zpl-c/enet |
| Encryption | Botan | https://github.com/randombit/botan |
| GUI | Dear ImGui | https://github.com/ocornut/imgui |
| OpenGL Library | GLFW | https://github.com/glfw/glfw |
| OpenGL Loading Library | GLAD | https://github.com/Dav1dde/glad |
