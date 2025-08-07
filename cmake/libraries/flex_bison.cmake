#
# Workaround to get a recent version Flex and Bison executables in MacOS
# as Bison 2.3 shipped with MacOS doesn't work
#
if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(COMMAND brew --prefix bison RESULT_VARIABLE APP_RESULT OUTPUT_VARIABLE APP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (APP_RESULT EQUAL 0 AND EXISTS "${APP_PREFIX}")
        set(BISON_EXECUTABLE "${APP_PREFIX}/bin/bison")
    endif()

    execute_process(COMMAND brew --prefix flex RESULT_VARIABLE APP_RESULT OUTPUT_VARIABLE APP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (APP_RESULT EQUAL 0 AND EXISTS "${APP_PREFIX}")
        set(FLEX_EXECUTABLE "${APP_PREFIX}/bin/flex")
    endif()
endif()

find_package(BISON 3 REQUIRED)
find_package(FLEX REQUIRED)
