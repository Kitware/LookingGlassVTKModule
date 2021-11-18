message(STATUS "LookingGlass: Building as a Remote VTK module")

set(classes
  vtkLookingGlassInterface
  vtkLookingGlassPass
)

# add OS specic render window implementation
if (VTK_USE_X)
  list(APPEND classes vtkXLookingGlassRenderWindow)
elseif (WIN32)
  list(APPEND classes vtkWin32LookingGlassRenderWindow)
  if (NOT VTK_USE_MICROSOFT_MEDIA_FOUNDATION OR NOT VTK_USE_VIDEO_FOR_WINDOWS)
    message(FATAL_ERROR
      "LookingGlass module requires"
       " VTK_USE_MICROSOFT_MEDIA_FOUNDATION and"
       " VTK_USE_VIDEO_FOR_WINODWS to be set to ON" )
  endif()
elseif (VTK_USE_COCOA)
  list(APPEND sources vtkCocoaLookingGlassRenderWindow.mm)
  list(APPEND headers vtkCocoaLookingGlassRenderWindow.h)
endif()


vtk_module_add_module(VTK::RenderingLookingGlass
  CLASSES ${classes}
  SOURCES ${sources}
  HEADERS ${headers}
)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

vtk_module_find_package(PACKAGE HoloPlayCore)
vtk_module_link(VTK::RenderingLookingGlass
    PUBLIC HoloPlayCore::HoloPlayCore)
if (APPLE)
  vtk_module_link(VTK::RenderingLookingGlass
    PRIVATE "-framework IOKit")
endif ()

# Copy the FindHoloPlayCore.cmake file to the build directory
if (DEFINED vtk_cmake_build_dir)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/FindHoloPlayCore.cmake"
    "${vtk_cmake_build_dir}/FindHoloPlayCore.cmake"
    COPYONLY)
endif ()

# Workaround limitation of VTK build-system preventing export module specific CMake variables
# in module config files.
if (DEFINED vtk_cmake_destination AND VTK_BINARY_DIR)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/FindHoloPlayCore.cmake"
    "${VTK_BINARY_DIR}/${vtk_cmake_destination}/FindHoloPlayCore.cmake"
    COPYONLY)
endif ()

if (DEFINED vtk_cmake_destination)
  install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/FindHoloPlayCore.cmake"
    DESTINATION "${vtk_cmake_destination}"
    COMPONENT   "development")
endif ()
