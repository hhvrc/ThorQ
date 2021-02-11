# ThorQ

![Ubuntu](https://github.com/hhvrc/ThorQ/workflows/Ubuntu/badge.svg)
![Windows](https://github.com/hhvrc/ThorQ/workflows/Windows/badge.svg)

Server and Client to control a shock-collar remotely from anywhere in the world.

##### Why?
- I want to be domesticated UwU
- Why not

## Setup
Install Qt5 - https://www.qt.io/download
### Linux
```console
$ sudo apt install cmake build-essential libsodium-dev gcc-10 g++-10 libstdc++-10-dev libstdc++-10-doc libc6 libc6-dev
$ git clone https://github.com/google/flatbuffers.git flatbuffers
$ cd flatbuffers
$ cmake -DCMAKE_BUILD_TYPE=Release .
$ sudo cmake --build . --target install --config Release --parallel $((`nproc`+1))
$ cd ..
$ git clone https://github.com/hhvrc/ThorQ.git
$ cd ThorQ
$ git submodule update --init --recursive
$ mkdir build
$ cd build
$ cmake ..
$ make -j$((`nproc`+1))
```
### Windows
```powershell
> git clone https://github.com/google/flatbuffers.git flatbuffers
> cd flatbuffers
> cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release .
> sudo cmake --build . --target install --config Release
> cd ..
> git clone https://github.com/hhvrc/ThorQ.git
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
| Networking        | ASIO            | https://think-async.com/Asio/                 | https://think-async.com/Asio/Download.html      |                     |
| Serialization     | FlatBuffers     | https://google.github.io/flatbuffers/         | https://github.com/google/flatbuffers           |                     |
| String formatting | {fmt}           | https://fmt.dev/latest/index.html             | https://github.com/fmtlib/fmt                   | libfmt-dev          |
| Cryptography      | LibSodium       | https://libsodium.gitbook.io/doc/             | https://github.com/jedisct1/libsodium           | libsodium-dev       |
| GUI               | Qt              | https://www.qt.io/                            | https://www.qt.io/download-qt-installer         | qt5-default         |
| VR UI             | OpenVR          | https://www.steamvr.com/en/                   | https://github.com/ValveSoftware/openvr         | libopenvr-dev       |
| Argument parsing  | cxxopts         | https://github.com/jarro2783/cxxopts          | https://github.com/jarro2783/cxxopts            |                     |
| Message queueing  | ConcurrentQueue | https://github.com/cameron314/concurrentqueue | https://github.com/cameron314/concurrentqueue   |                     |
| SQLite wrapper    | LSql            | https://github.com/hhvrc/LSql             | https://github.com/hhvrc/LSql               |                     |
