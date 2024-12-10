FROM debian:bookworm

RUN apt update && apt install -y git clang cmake flex bison zlib1g-dev ninja-build

# Compile with clang
ENV CC=clang
ENV CXX=clang++

# Clone the repository
RUN mkdir -p /tmp/openmohaa/build \
    && cd /tmp/openmohaa \
    && git clone --depth 1 --single-branch -b main https://github.com/openmoh/openmohaa.git

# Build the project
RUN cd /tmp/openmohaa/build \
    && cmake -G "Ninja" -DBUILD_NO_CLIENT=1 -DCMAKE_BUILD_TYPE="RelWithDebInfo" -DTARGET_LOCAL_SYSTEM=1 -DCMAKE_INSTALL_PREFIX="/usr/local/games/openmohaa" /tmp/openmohaa/openmohaa \
    && cmake --build . \
    && cmake --install .

# Reset to the base debian image, will gain space
FROM debian:bookworm

# Copy the installed directory
COPY --from=0 /usr/local/games/openmohaa /usr/local/games/openmohaa

# MOHAA data (main, mainta and maintt) goes inside this directory
# mods can go inside a subdirectory called "mods"
VOLUME "/usr/local/share/mohaa"

# Create an user for starting the server
# This improves the overall security of the server instance
RUN useradd -m openmohaa
USER openmohaa

WORKDIR "/usr/local/share/mohaa"
ENTRYPOINT [ "/usr/local/games/openmohaa/lib/openmohaa/omohaaded", "+set", "fs_game", "mods" ]
