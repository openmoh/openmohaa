# Docker

docker run --rm -ti -v E:\Src\openmoh\openmohaa:/usr/src/openmohaa openmohaa/builder.20_04

Run with
```sh
mkdir .cmake
cd .cmake
export CC=clang++-15
export CXX=clang++-15
cmake -G Ninja ../
ninja
```
