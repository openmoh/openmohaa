FROM debian:11

# install dev tools
RUN apt update && apt install -y ninja-build cmake make clang-16 lld-16 gcc-10 g++-10 flex bison
# install some important libraries like GL
RUN apt install -y --no-install-recommends ncurses-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev libxxf86vm-dev libxxf86dga-dev libsdl2-dev

WORKDIR /usr/src/openmohaa/
