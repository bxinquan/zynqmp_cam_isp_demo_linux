#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "XRT::xilinxopencl" for configuration "Release"
set_property(TARGET XRT::xilinxopencl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(XRT::xilinxopencl PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "XRT::xrt++;XRT::xrt_coreutil;Boost::filesystem;Boost::system"
  IMPORTED_LOCATION_RELEASE "/usr/lib/libxilinxopencl.so.2.13.0"
  IMPORTED_SONAME_RELEASE "libxilinxopencl.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS XRT::xilinxopencl )
list(APPEND _IMPORT_CHECK_FILES_FOR_XRT::xilinxopencl "/usr/lib/libxilinxopencl.so.2.13.0" )

# Import target "XRT::xrt++" for configuration "Release"
set_property(TARGET XRT::xrt++ APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(XRT::xrt++ PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "XRT::xrt_coreutil;Boost::filesystem;Boost::system"
  IMPORTED_LOCATION_RELEASE "/usr/lib/libxrt++.so.2.13.0"
  IMPORTED_SONAME_RELEASE "libxrt++.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS XRT::xrt++ )
list(APPEND _IMPORT_CHECK_FILES_FOR_XRT::xrt++ "/usr/lib/libxrt++.so.2.13.0" )

# Import target "XRT::xrt_coreutil" for configuration "Release"
set_property(TARGET XRT::xrt_coreutil APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(XRT::xrt_coreutil PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "Boost::filesystem;Boost::system"
  IMPORTED_LOCATION_RELEASE "/usr/lib/libxrt_coreutil.so.2.13.0"
  IMPORTED_SONAME_RELEASE "libxrt_coreutil.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS XRT::xrt_coreutil )
list(APPEND _IMPORT_CHECK_FILES_FOR_XRT::xrt_coreutil "/usr/lib/libxrt_coreutil.so.2.13.0" )

# Import target "XRT::xrt_core" for configuration "Release"
set_property(TARGET XRT::xrt_core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(XRT::xrt_core PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "XRT::xrt_coreutil;Boost::filesystem;Boost::system"
  IMPORTED_LOCATION_RELEASE "/usr/lib/libxrt_core.so.2.13.0"
  IMPORTED_SONAME_RELEASE "libxrt_core.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS XRT::xrt_core )
list(APPEND _IMPORT_CHECK_FILES_FOR_XRT::xrt_core "/usr/lib/libxrt_core.so.2.13.0" )

# Import target "XRT::xrt_hwemu" for configuration "Release"
set_property(TARGET XRT::xrt_hwemu APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(XRT::xrt_hwemu PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "XRT::xrt_coreutil;Boost::filesystem;Boost::system"
  IMPORTED_LOCATION_RELEASE "/usr/lib/libxrt_hwemu.so.2.13.0"
  IMPORTED_SONAME_RELEASE "libxrt_hwemu.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS XRT::xrt_hwemu )
list(APPEND _IMPORT_CHECK_FILES_FOR_XRT::xrt_hwemu "/usr/lib/libxrt_hwemu.so.2.13.0" )

# Import target "XRT::xrt_swemu" for configuration "Release"
set_property(TARGET XRT::xrt_swemu APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(XRT::xrt_swemu PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "Boost::filesystem;Boost::system;XRT::xrt_coreutil"
  IMPORTED_LOCATION_RELEASE "/usr/lib/libxrt_swemu.so.2.13.0"
  IMPORTED_SONAME_RELEASE "libxrt_swemu.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS XRT::xrt_swemu )
list(APPEND _IMPORT_CHECK_FILES_FOR_XRT::xrt_swemu "/usr/lib/libxrt_swemu.so.2.13.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
