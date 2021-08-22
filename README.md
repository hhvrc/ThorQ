# ThorQ

![Ubuntu](https://github.com/ThorqDev/ThorQ/workflows/Ubuntu/badge.svg)
![Windows](https://github.com/ThorqDev/ThorQ/workflows/Windows/badge.svg)

OpenSource program to control a shock-collar remotely from anywhere in the world.

##### Why?
- I want to be domesticated UwU
- Why not

## Setup
Install Qt5 - https://www.qt.io/download
### Linux
```console
$ sudo apt install -y cmake build-essential libsodium-dev gcc-10 g++-10 libstdc++-10-dev libstdc++-10-doc libc6 libc6-dev mesa-common-dev libglu1-mesa-dev
$ git clone https://github.com/ThorqDev/ThorQ.git
$ cd ThorQ
$ git submodule update --init --recursive
$ mkdir build
$ cd build
$ cmake ..
$ make -j$((`nproc`+1))
```
### Windows
```powershell
> git clone https://github.com/ThorqDev/ThorQ.git
> cd ThorQ
> git submodule update --init --recursive
> cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release .
```
Open Visual Studio 2019

Select "Open Directory"

Choose the cloned directory, and click OK

CMake will now configure the project

Then do Ctrl+Shift+B, or click "Build all" to build

## Libraries used
| Use Case          | Name            | Home Link                                     | Download Windows                                | Apt                 |
| ----------------- | --------------- | --------------------------------------------- | ----------------------------------------------- | ------------------- |
| Framework         | Qt              | https://www.qt.io/                            | https://www.qt.io/download-qt-installer         | qt5-default         |
| Cryptography      | LibSodium       | https://libsodium.gitbook.io/doc/             | https://github.com/jedisct1/libsodium           | libsodium-dev       |
| VR Interaction    | OpenVR          | https://www.steamvr.com/en/                   | https://github.com/ValveSoftware/openvr         | libopenvr-dev       |
| Non-Qt printing   | {fmt}           | https://fmt.dev/latest/index.html             | https://github.com/fmtlib/fmt                   | libfmt-dev          |
