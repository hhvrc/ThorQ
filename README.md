# CollarControl

Server and Client to control a shock-collar remotely from anywhere in the world.

##### Why?
- I want to be domesticated UwU
- Why not

## Setup
### Linux
```console
$ git clone https://github.com/hhvrc/CollarControl.git
$ cd CollarControl
$ git submodule update --init
$ mkdir build
$ cd build
$ cmake ..
$ make -4
```
### Windows
```powershell
> git clone https://github.com/hhvrc/CollarControl.git
> cd CollarControl
> git submodule update --init
```
Open Visual Studio 2019

Select "Open Directory"

Choose the cloned directory, and click OK

CMake will now configure the project

Then do Ctrl+Shift+B, or click "Build all" to build

## Libraries used
| Use Case         | Name            | Home Link                                     | Download Windows                                | Apt                 |
| ---------------- | --------------- | --------------------------------------------- | ----------------------------------------------- | ------------------- |
| Networking       | ENet            | http://enet.bespin.org/                       | https://github.com/zpl-c/enet                   |                     |
| Cryptography     | LibSodium       | https://libsodium.gitbook.io/doc/             | https://github.com/jedisct1/libsodium           | libsodium-dev       |
| GUI              | Qt              | https://www.qt.io/                            | https://www.qt.io/download-qt-installer         | qt5-default         |
| VR UI            | OpenVR          | https://www.steamvr.com/en/                   | https://github.com/ValveSoftware/openvr         | libopenvr-dev       |
| Message queueing | ConcurrentQueue | https://github.com/cameron314/concurrentqueue | https://github.com/cameron314/concurrentqueue   |                     |
