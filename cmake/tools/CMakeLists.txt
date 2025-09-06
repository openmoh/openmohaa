cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})
project(qvm_tools LANGUAGES C)

include(utils/set_output_dirs)

set(Q3ASM_SOURCES
    ${SOURCE_DIR}/tools/asm/q3asm.c
    ${SOURCE_DIR}/tools/asm/cmdlib.c
)

set(Q3LCC_SOURCES
    ${SOURCE_DIR}/tools/lcc/etc/lcc.c
    ${SOURCE_DIR}/tools/lcc/etc/bytecode.c
)

set(Q3RCC_SOURCES
    ${SOURCE_DIR}/tools/lcc/src/alloc.c
    ${SOURCE_DIR}/tools/lcc/src/bind.c
    ${SOURCE_DIR}/tools/lcc/src/bytecode.c
    ${SOURCE_DIR}/tools/lcc/src/dag.c
    ${SOURCE_DIR}/tools/lcc/src/decl.c
    ${SOURCE_DIR}/tools/lcc/src/enode.c
    ${SOURCE_DIR}/tools/lcc/src/error.c
    ${SOURCE_DIR}/tools/lcc/src/event.c
    ${SOURCE_DIR}/tools/lcc/src/expr.c
    ${SOURCE_DIR}/tools/lcc/src/gen.c
    ${SOURCE_DIR}/tools/lcc/src/init.c
    ${SOURCE_DIR}/tools/lcc/src/inits.c
    ${SOURCE_DIR}/tools/lcc/src/input.c
    ${SOURCE_DIR}/tools/lcc/src/lex.c
    ${SOURCE_DIR}/tools/lcc/src/list.c
    ${SOURCE_DIR}/tools/lcc/src/main.c
    ${SOURCE_DIR}/tools/lcc/src/null.c
    ${SOURCE_DIR}/tools/lcc/src/output.c
    ${SOURCE_DIR}/tools/lcc/src/prof.c
    ${SOURCE_DIR}/tools/lcc/src/profio.c
    ${SOURCE_DIR}/tools/lcc/src/simp.c
    ${SOURCE_DIR}/tools/lcc/src/stmt.c
    ${SOURCE_DIR}/tools/lcc/src/string.c
    ${SOURCE_DIR}/tools/lcc/src/sym.c
    ${SOURCE_DIR}/tools/lcc/src/symbolic.c
    ${SOURCE_DIR}/tools/lcc/src/trace.c
    ${SOURCE_DIR}/tools/lcc/src/tree.c
    ${SOURCE_DIR}/tools/lcc/src/types.c
)

set(Q3RCC_DAGCHECK_SOURCE ${SOURCE_DIR}/tools/lcc/src/dagcheck.md)

set(Q3CPP_SOURCES
    ${SOURCE_DIR}/tools/lcc/cpp/cpp.c
    ${SOURCE_DIR}/tools/lcc/cpp/lex.c
    ${SOURCE_DIR}/tools/lcc/cpp/nlist.c
    ${SOURCE_DIR}/tools/lcc/cpp/tokens.c
    ${SOURCE_DIR}/tools/lcc/cpp/macro.c
    ${SOURCE_DIR}/tools/lcc/cpp/eval.c
    ${SOURCE_DIR}/tools/lcc/cpp/include.c
    ${SOURCE_DIR}/tools/lcc/cpp/hideset.c
    ${SOURCE_DIR}/tools/lcc/cpp/getopt.c
    ${SOURCE_DIR}/tools/lcc/cpp/unix.c
)

set(LBURG_SOURCES
    ${SOURCE_DIR}/tools/lcc/lburg/lburg.c
    ${SOURCE_DIR}/tools/lcc/lburg/gram.c
)

add_executable(q3asm ${Q3ASM_SOURCES})
set_output_dirs(q3asm)
add_executable(q3lcc ${Q3LCC_SOURCES})
set_output_dirs(q3lcc)
add_dependencies(q3lcc q3rcc q3cpp)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "^(Apple)?Clang$")
    add_compile_options(-fno-strict-aliasing -Wno-unused-result
        -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/wd4311 /wd4312)
endif()

add_executable(lburg ${LBURG_SOURCES})
set_output_dirs(lburg)
set(DAGCHECK_C ${CMAKE_BINARY_DIR}/dagcheck.c)
add_custom_command(
    OUTPUT ${DAGCHECK_C}
    COMMAND lburg ${Q3RCC_DAGCHECK_SOURCE} ${DAGCHECK_C}
    DEPENDS lburg ${Q3RCC_DAGCHECK_SOURCE})

add_executable(q3rcc ${Q3RCC_SOURCES} ${DAGCHECK_C})
set_output_dirs(q3rcc)
target_include_directories(q3rcc PRIVATE ${SOURCE_DIR}/tools/lcc/src)

add_executable(q3cpp ${Q3CPP_SOURCES})
set_output_dirs(q3cpp)
