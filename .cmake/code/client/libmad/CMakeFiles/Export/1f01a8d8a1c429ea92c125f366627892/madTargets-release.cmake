#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "mad::mad" for configuration "Release"
set_property(TARGET mad::mad APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(mad::mad PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/mad.lib"
  )

list(APPEND _cmake_import_check_targets mad::mad )
list(APPEND _cmake_import_check_files_for_mad::mad "${_IMPORT_PREFIX}/bin/mad.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
