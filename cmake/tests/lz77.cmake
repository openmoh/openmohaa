#
# Unit tests
#

add_executable(test_lz77
    ${SOURCE_DIR}/corepp/tests/test_lz77.cpp
    ${SOURCE_DIR}/corepp/lz77.cpp
    ${SOURCE_DIR}/qcommon/q_shared.c
    ${SOURCE_DIR}/qcommon/common_light.c
)

target_link_libraries(test_lz77 INTERFACE testing)
add_test(NAME test_lz77 COMMAND test_lz77)
set_tests_properties(test_lz77 PROPERTIES TIMEOUT 15)
