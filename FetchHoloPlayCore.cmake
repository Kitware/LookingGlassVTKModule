include(FetchContent)

set(proj HoloPlayCore)
set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
FetchContent_Populate(${proj}
  SOURCE_DIR   ${EP_SOURCE_DIR}
  URL          https://www.paraview.org/files/dependencies/HoloPlayCore-0.1.1-Open-20200923.tar.gz
  URL_HASH MD5=b435316fa1f8454ba180e72608c3c28f
  QUIET
  )

set(${proj}_INCLUDE_DIR "${EP_SOURCE_DIR}/HoloPlayCore/include")
if(APPLE)
  set(_dir "macos")
  set(_prefix "lib")
  set(_ext "dylib")
elseif(UNIX)
  set(_dir "linux")
  set(_prefix "lib")
  set(_ext "so")
else()
  set(_dir "Win64")
  set(_prefix "")
  set(_ext "lib")
endif()
set(_library_dir "${EP_SOURCE_DIR}/HoloPlayCore/dylib/${_dir}")
set(${proj}_LIBRARY "${_library_dir}/${_prefix}HoloPlayCore.${_ext}")

message(STATUS "Remote - ${proj} [OK]")
message(STATUS "Remote - ${proj}_INCLUDE_DIR:${${proj}_INCLUDE_DIR}")
message(STATUS "Remote - ${proj}_LIBRARY:${${proj}_LIBRARY}")
