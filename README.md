# Looking Glass Device Support

## Introduction

The LookingGlassVTKModule provides support for rendering VTK scenes into
looking glass devices. Both Python bindings (via the
[`vtk-lookingglass` package on pip](https://pypi.org/project/vtk-lookingglass/))
and C++ builds are supported.

## Python Support
The simplest way to get started is by running `pip install vtk-lookingglass`
and then trying out examples in the `Examples/Python` directory. This should
work on all major operating systems, and for most modern versions of Python.

In fact, many [VTK Python Examples](https://kitware.github.io/vtk-examples/site/Python/) can be easily adapted
to render to the Looking Glass render window by simply swapping out the
example's render window with one created via
`vtkLookingGlassInterface.CreateLookingGlassRenderWindow()`.

### Cinematic Rendering

Some notable cinematic rendering examples are available in the `Examples/Python/cinematic_rendering`
directory. These use VTK's new light scattering model to create realistic
volumetric renderings of medical data. An example video is provided below.

[![Cinematic Rendering Example](https://res.cloudinary.com/marcomontalbano/image/upload/v1664546921/video_to_markdown/images/youtube--9FC3auNbxnY-c05b58ac6eb4c4700831b2b3070cd403.jpg)](https://www.youtube.com/watch?v=9FC3auNbxnY&ab_channel=StephenAylward "Cinematic Rendering Example")

## C++ Support
For C++ support, there are two main approaches.
You can create a OS specific Looking Glass render window and use it as you
would a regular vtkRenderWindow. For an example of this approach please see
the TestDragon test in `Testing/Cxx/TestDragon.cxx`. The other approach is to
use the vtkLookingGlassPass as you would normally use a render pass in a
renderer. For an example of that approach please see
`Testing\Cxx\TestLookingGlassPass.cxx`.

### Build Requirements

Building this modules requires the Looking Glass Factory's
`HoloPlayCoreSDK`. You must request a copy of the HoloPlayCoreSDK
directly from Looking Glass Factory.

The SDK is described
[here](https://docs.lookingglassfactory.com/holoplay-core/holoplay-core-sdk).

You can request access to their SDK
[here](https://lookingglassfactory.com/software#holoplay-core).

Access is granted nearly instantaneously.  Their SDK includes dylibs
(shared libraries) for MacOS, Linux, and Windows (32 and 64bit).

We recommend installing their SDK at the same level as the
VTK source directory, so that it can be automatically found by VTK during
compilation.  For example, if VTK souce is in `/src/VTK` on your system,
then uncompress the HoloPlaceCoreSDK (which is currently at version 0.2.0) into
the `/src` directory.   The path to the HoloPlayCoreSDK include directory
would then be `/src/HoloPlayCore-0.2.0/HoloPlayCoreSDK-master/HoloPlayCore/include`.
VTK can then automatically find the include and appropriate dylib files
during CMake configuration of VTK,  otherwise you will have to specify
the paths to those files manually during CMake configuration.

### CMake Configuration of VTK

When configuring VTK using CMake, enable this remote module and its dependencies by setting
1. VTK_MODULE_ENABLE_VTK_RenderingLookingGlass to YES
2. VTK_USE_VIDEO_FOR_WINDOWS to ON
3. VTK_USE_MICROSOFT_MEDIA_FOUNDATION to ON

You must also specify
1. HoloPlayCore_INCLUDE_DIR
2. HoloPlayCore_LIBRARY

based on where you have installed the HoloPlayCore SDK, if they are not
automatically found.

### Compilation

With this module enabled in CMake, build VTK as usual for your platform. We
currently support Windows, OSX, and Linux platforms.

### Running VTK applications

Since the HoloPlayCoreSDK is distribued as shared libraries, their location
must be known when executing a VTK-based application if that version of
VTK was compiled with this module enabled.

We recommend adding the path to the appropriate shared lib to your system's
PATH/LD_LIBRARY_PATH variables.  They will then be automatically found
when running VTK applications that were compiled with this module enabled.

System-level access to the shared libs is also required when running the
Python wrapped VTK if this module was enabled when it was compiled.

### Rendering to a display and generating Quilts

The key functionality of this module is held in the `vtkLookingGlassInterface`
class.  It is used by the render window classes and by the render pass
implementations. This module is even capable of creating distributable quilt
images even if no Looking Glass hardware is present.

### Building and running the C++ tests

In order to build and run the C++ tests, this module must be built from
within VTK (see [the required CMake settings](#cmake-configuration-of-vtk))
with the additional CMake flag `-DVTK_BUILD_TESTING=ON`.

After building, within the build directory, the tests should be located
in `./bin/vtkLookingGlassCxxTests`, and the test data should be located in
`./ExternalData/Testing/`. The tests may be executed like the following:

```bash
./bin/vtkLookingGlassCxxTests TestDragon -D ./ExternalData/Testing/ -I
./bin/vtkLookingGlassCxxTests TestLookingGlassPass -D ./ExternalData/Testing/ -I
```

Where the `-D` flag provides the location of the test data directory,
and the `-I` indiciates that the test should run interactively (otherwise,
the application will render once and close immediately).

## Developement/Bug Reports

If you run into issues with this module please submit a bug report at
https://github.com/Kitware/LookingGlassVTKModule/issues

## License

See LICENSE file.
