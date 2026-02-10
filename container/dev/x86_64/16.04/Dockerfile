FROM ubuntu:16.04

# install dev tools
RUN apt update && apt install -y ninja-build clang-5.0 lld-5.0 gcc-5 g++-5 flex bison
# install some important libraries like SDL
RUN DEBIAN_FRONTEND=noninteractive apt install -y --no-install-recommends libsdl2-dev

RUN apt install -y wget \
  && cd /tmp \
  && wget https://github.com/Kitware/CMake/releases/download/v4.2.3/cmake-4.2.3-linux-x86_64.tar.gz \
  && tar -xvf cmake-* \
  && cp cmake-*/bin/cmake /usr/bin/cmake \
  && cp -r /tmp/cmake-*/share /usr/ \
  && rm -rf cmake-* \
  && apt remove -y wget \
  && cmake --version


WORKDIR /usr/src/openmohaa/
