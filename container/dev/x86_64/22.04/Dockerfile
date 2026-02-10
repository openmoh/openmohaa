FROM ubuntu:22.04

# install dev tools
RUN apt update && apt install -y ninja-build cmake clang-15 lld-15 gcc-12 g++-12 flex bison
# install some important libraries like SDL
RUN apt install -y --no-install-recommends libsdl2-dev

WORKDIR /usr/src/openmohaa/
