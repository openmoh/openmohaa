FROM ubuntu:18.04

# install dev tools
RUN apt update && apt install -y ninja-build cmake make clang-6.0 lld-6.0 gcc-6 g++-6 flex bison
# install some important libraries like SDL
RUN apt install -y --no-install-recommends libsdl2-dev

WORKDIR /usr/src/openmohaa/
