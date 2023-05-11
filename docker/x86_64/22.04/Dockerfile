FROM ubuntu:22.04

# install dev tools
RUN apt update && apt install -y ninja-build cmake make clang-15 lld-15 gcc-12 g++-12 flex bison
# install some important libraries
RUN apt install -y --no-install-recommends ncurses-dev

WORKDIR /usr/src/openmohaa/
#COPY CMakeLists.txt ./
#COPY src ./src
#COPY tests ./tests
#COPY include ./include
#
#RUN mkdir -p ./cmake && cd ./cmake \
#    && cmake -DCMAKE_C_COMPILER=clang-11 -DCMAKE_CXX_COMPILER=clang++-11 -DCMAKE_INSTALL_PREFIX=/usr/local/morfuse/engine -DCMAKE_BUILD_TYPE=Release -G Ninja ../ \
#    && ninja \
#    && cmake --install .
