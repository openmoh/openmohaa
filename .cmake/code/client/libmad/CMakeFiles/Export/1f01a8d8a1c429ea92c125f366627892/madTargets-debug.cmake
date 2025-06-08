#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "mad::mad" for configuration "Debug"
set_property(TARGET mad::mad APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mad::mad PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/mad-dbg.lib"
  )

list(APPEND _cmake_import_check_targets mad::mad )
list(APPEND _cmake_import_check_files_for_mad::mad "${_IMPORT_PREFIX}/bin/mad-dbg.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
