#cmake .. -DCMAKE_TOOLCHAIN_FILE=../morphos/morphos.cmake -DNO_MODERN_DMA=1
#ppc-morphos-strip openmohaa.ppc
#ppc-morphos-strip code/client/cgame/cgame.ppc.so
#ppc-morphos-strip code/server/fgame/game.ppc.so

set(TOOLCHAIN_OS MorphOS)
set(TOOLCHAIN_SYSTEM_INFO_FILE Platform/${TOOLCHAIN_OS})

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ppc)
set(TOOLCHAIN_PATH /gg)

include_directories("/gg/usr/local/include/SDL2" "/gg/usr/local/include")

add_compile_options(-noixemul -O2 -mcpu=750 -mtune=7450 -fomit-frame-pointer -ffast-math)

set(MORPHOS 1)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/ppc-morphos-gcc -noixemul -fno-asynchronous-unwind-tables)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/ppc-morphos-g++ -noixemul -fpermissive)
set(CMAKE_CPP_COMPILER ${TOOLCHAIN_PATH}/bin/ppc-morphos-cpp)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/ppc.morphos-gcc -c)
#set(CMAKE_VERBOSE_MAKEFILE TRUE)

set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -L/gg/usr/local/lib -lSDL2 -lGL -ldll")


