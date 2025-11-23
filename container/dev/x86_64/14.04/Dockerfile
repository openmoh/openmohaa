FROM ubuntu:14.04

# install dev tools
RUN apt update && apt install -y ninja-build cmake make clang-3.5 gcc-4.8 g++-4.8 flex bison
# install some important libraries like SDL
RUN apt install -y --no-install-recommends libsdl2-dev

WORKDIR /usr/src/openmohaa/
