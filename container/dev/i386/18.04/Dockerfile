FROM --platform=linux/i386 ubuntu:18.04

# install dev tools
RUN apt update && apt install -y ninja-build cmake make clang-9 lld-9 gcc-8 g++-8 flex bison
RUN apt update && apt install --no-install-recommends -y libsdl2-dev

WORKDIR /usr/src/openmohaa/
