# Try config package first (vcpkg / CMake package installs)
find_package(unofficial-sodium CONFIG QUIET)

add_library(sodium_lib INTERFACE)

if(TARGET unofficial-sodium::sodium)
    target_link_libraries(sodium_lib INTERFACE unofficial-sodium::sodium)
elseif(TARGET unofficial::sodium::sodium)
    target_link_libraries(sodium_lib INTERFACE unofficial::sodium::sodium)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)
    target_link_libraries(sodium_lib INTERFACE PkgConfig::SODIUM)
endif()