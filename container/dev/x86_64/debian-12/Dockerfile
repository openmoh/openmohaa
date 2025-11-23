FROM debian:12

# install dev tools
RUN apt update && apt install -y ninja-build cmake make clang-19 lld-19 gcc-12 g++-12 flex bison
# install some important libraries like GL
RUN apt install -y --no-install-recommends ncurses-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev libxxf86vm-dev libxxf86dga-dev libsdl2-dev

WORKDIR /usr/src/openmohaa/
