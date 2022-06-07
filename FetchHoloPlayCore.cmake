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
  set(_rt_ext "dylib")
elseif(UNIX)
  set(_dir "linux")
  set(_prefix "lib")
  set(_ext "so")
  set(_rt_ext "so")
else()
  set(_dir "Win64")
  set(_prefix "")
  set(_ext "lib")
  set(_rt_ext "dll")
endif()
set(_library_dir "${EP_SOURCE_DIR}/HoloPlayCore/dylib/${_dir}")
set(${proj}_LIBRARY "${_library_dir}/${_prefix}HoloPlayCore.${_ext}")
set(${proj}_RUNTIME_LIBRARY "${_library_dir}/${_prefix}HoloPlayCore.${_rt_ext}")

message(STATUS "Remote - ${proj} [OK]")
message(STATUS "Remote - ${proj}_INCLUDE_DIR:${${proj}_INCLUDE_DIR}")
message(STATUS "Remote - ${proj}_LIBRARY:${${proj}_LIBRARY}")
message(STATUS "Remote - ${proj}_RUNTIME_LIBRARY:${${proj}_RUNTIME_LIBRARY}")
