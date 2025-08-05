include_guard(GLOBAL)

set(DETECT_ARCH_C ${CMAKE_BINARY_DIR}/detect_arch.c)

file(WRITE ${DETECT_ARCH_C}
"#include \"${SOURCE_DIR}/qcommon/q_platform.h\"
#include <stdio.h>
int main()
{
    puts(ARCH_STRING);
    return 0;
}
")

try_run(RUN_EXITCODE COMPILE_SUCCESS
    ${CMAKE_BINARY_DIR} ${DETECT_ARCH_C}
    RUN_OUTPUT_VARIABLE ARCH)

string(STRIP ${ARCH} ARCH)

if(NOT COMPILE_SUCCESS OR RUN_EXITCODE OR NOT ARCH)
    message(FATAL_ERROR "Architecture detection failed")
endif()
