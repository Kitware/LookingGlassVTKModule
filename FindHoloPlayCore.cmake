find_path(HoloPlayCore_INCLUDE_DIR
  NAMES HoloPlayCore.h
  DOC "HoloPlayCore.h include directory")
mark_as_advanced(HoloPlayCore_INCLUDE_DIR)
find_library(HoloPlayCore_LIBRARY
  NAMES HoloPlayCore
  DOC "HoloPlayCore library")
mark_as_advanced(HoloPlayCore_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HoloPlayCore
  REQUIRED_VARS HoloPlayCore_LIBRARY HoloPlayCore_INCLUDE_DIR
)

if (HoloPlayCore_FOUND)
  set(HoloPlayCore_INCLUDE_DIRS "${HoloPlayCore_INCLUDE_DIR}")
  set(HoloPlayCore_LIBRARIES "${HoloPlayCore_LIBRARY}")

  if (NOT TARGET HoloPlayCore::HoloPlayCore)
    add_library(HoloPlayCore::HoloPlayCore UNKNOWN IMPORTED)
    set_target_properties(HoloPlayCore::HoloPlayCore PROPERTIES
      IMPORTED_LOCATION "${HoloPlayCore_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${HoloPlayCore_INCLUDE_DIR}")
  endif ()
endif ()
