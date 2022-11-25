#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "openjp2" for configuration ""
set_property(TARGET openjp2 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(openjp2 PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_NOCONFIG "m"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libopenjp2.so.2.4.0"
  IMPORTED_SONAME_NOCONFIG "libopenjp2.so.7"
  )

list(APPEND _IMPORT_CHECK_TARGETS openjp2 )
list(APPEND _IMPORT_CHECK_FILES_FOR_openjp2 "${_IMPORT_PREFIX}/lib/libopenjp2.so.2.4.0" )

# Import target "openjp2_static" for configuration ""
set_property(TARGET openjp2_static APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(openjp2_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libopenjp2.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS openjp2_static )
list(APPEND _IMPORT_CHECK_FILES_FOR_openjp2_static "${_IMPORT_PREFIX}/lib/libopenjp2.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
