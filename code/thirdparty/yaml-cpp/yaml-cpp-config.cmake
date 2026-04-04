# - Config file for the yaml-cpp package
# It defines the following variables
#  YAML_CPP_INCLUDE_DIR       - include directory
#  YAML_CPP_LIBRARY_DIR       - directory containing libraries
#  YAML_CPP_SHARED_LIBS_BUILT - whether we have built shared libraries or not
#  YAML_CPP_LIBRARIES         - libraries to link against


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was yaml-cpp-config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

set_and_check(YAML_CPP_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
set_and_check(YAML_CPP_LIBRARY_DIR "${PACKAGE_PREFIX_DIR}/lib")

# Are we building shared libraries?
set(YAML_CPP_SHARED_LIBS_BUILT OFF)

# Our library dependencies (contains definitions for IMPORTED targets)
include("${CMAKE_CURRENT_LIST_DIR}/yaml-cpp-targets.cmake")

# These are IMPORTED targets created by yaml-cpp-targets.cmake
set(YAML_CPP_LIBRARIES "yaml-cpp::yaml-cpp")

# Protect against multiple inclusion, which would fail when already imported targets are added once more.
if(NOT TARGET yaml-cpp)
  add_library(yaml-cpp INTERFACE IMPORTED) 
  target_link_libraries(yaml-cpp INTERFACE yaml-cpp::yaml-cpp) 
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.17)
    set_target_properties(yaml-cpp PROPERTIES 
      DEPRECATION "The target yaml-cpp is deprecated and will be removed in version 0.10.0. Use the yaml-cpp::yaml-cpp target instead."
    )
  endif()
endif()

check_required_components(yaml-cpp)
