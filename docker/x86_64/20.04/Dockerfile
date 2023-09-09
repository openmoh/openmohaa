FROM ubuntu:20.04

# install dev tools
RUN apt update && apt install -y ninja-build cmake make clang-7 lld-7 gcc-7 g++-7 flex bison
# install some important libraries like SDL
RUN apt install -y --no-install-recommends libsdl2-dev

WORKDIR /usr/src/openmohaa/
