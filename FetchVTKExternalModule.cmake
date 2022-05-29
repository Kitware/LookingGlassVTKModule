include(FetchContent)

set(proj VTKExternalModule)
if (FETCH_${proj}_INSTALL_LOCATION)
  # The install location can be specified
  set(EP_SOURCE_DIR "${FETCH_${proj}_INSTALL_LOCATION}")
else()
  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
endif()

FetchContent_Populate(${proj}
  SOURCE_DIR     ${EP_SOURCE_DIR}
  GIT_REPOSITORY https://github.com/cjh1/VTKExternalModule
  GIT_TAG        wheel_build
  QUIET
  )

message(STATUS "Remote - ${proj} [OK]")

set(VTKExternalModule_SOURCE_DIR ${EP_SOURCE_DIR})
message(STATUS "Remote - VTKExternalModule_SOURCE_DIR:${VTKExternalModule_SOURCE_DIR}")
