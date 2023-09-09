FROM ubuntu:16.04

# install dev tools
RUN apt update && apt install -y ninja-build cmake make clang-5.0 lld-5.0 gcc-5 g++-5 flex bison
# install some important libraries like SDL
RUN apt install -y --no-install-recommends libsdl2-dev

WORKDIR /usr/src/openmohaa/
