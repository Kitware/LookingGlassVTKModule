# Looking Glass Device Support

## Introduction

This module contains support for rendering into looking glass devices using
two main approaches. You can create a OS specific Looking Glass render
window and use it as you would a regular vtkRenderWindow. For an example of
this approach please see the TestDragon test in Testing/Cxx/TestDragon.cxx
The other approach is to use the vtkLookingGlassPass as you would normally
use a render pass in a renderer. For an example of that approach please see
Testing\Cxx\TestLookingGlassPass.cxx

## Build Requirements

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
then uncompress the HoloPlaceCoreSDK which is current in version 0.2.0 into
the `/src` directory.   The path to the HoloPlayCoreSDK include directory
would then be `/src/HoloPlayCore-0.2.0/HoloPlayCore/include`.  VTK can then
automatically find the include and appropriate dylib files during CMake
configuration of VTK,  otherwise you will have to specify the paths to
those files manually during CMake configuration.

## CMake Configuration of VTK

When configuring VTK using CMake, enable this module by setting
`VTK_MODULE_ENABLE_VTK_RenderingLookingGlass` to `YES` and specifying the
HoloPlayCore include dir and library to where you have installed the
HoloPlayCore SDK, if they are not automatically found.

## Compilation

With this module enabled in CMake, build VTK as usual for your platform. We
currently support Windows, OSX, and Linux platforms.

## Running VTK applications

Since the HoloPlayCoreSDK is distribued as shared libraries, their location
must be known when executing a VTK-based application if that version of
VTK was compiled with this module enabled.

We recommend adding the path to the appropriate shared lib to your system's
PATH/LD_LIBRARY_PATH variables.  They will then be automatically found
when running VTK applications that were compiled with this module enabled.

System-level access to the shared libs is also required when running the
Python wrapped VTK if this module was enabled when it was compiled.

## Developement/Bug Reports

Note that almost all of the key functionality is held in
vtkLookingGlassInterface which is used by both the render window classes and
the render pass implementations. This module should work, showing the quilt
image even if no hardware is present.

If you run into issues with this module please submit a bug report at
https://github.com/Kitware/LookingGlassVTKModule/issues

## License

Copyright (c) 2020 Kitware, Inc.
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

vtkLookingGlassInterface.cxx uses content from HoloPlayCore, which is
distributed under the following license:

The MIT License (MIT)

Copyright (c) 2015 Arthur Sonzogni

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
