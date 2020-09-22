# Looking Glass Device Support

## Introduction

This module contains support for rendering into looking glass devices using
two main approaches. You can create a OS specific Looking Glass render
window and use it as you would a regular vtkRenderWindow. For an example of
this approach please see the TestDragon test in Testing/Cxx/TestDragon.cxx
The other approach is to use the vtkLookingGlassPass as you would normally
use a render pass in a renderer. For an example of that approach please see
Testing\Cxx\TestLookingGlassPass.cxx

## Building

When building VTK turn on the remote module by setting
VTK_MODULE_ENABLE_VTK_RenderingLookingGlass to YES and specifying the
HoloPlayCore include dir and library to where you have installed the
HoloPlayCore SDK.

At this point build VTK as usual for your platform. We currently support
Windows, OSX, and Linux platforms.

## Developement/Bug Reports

Note that almost all of the key functionality is held in
vtkLookingGlassInterface which is used by both the render window classes and
the render pass implementations. This module should work, showing the quilt
image even if no hardware is present.

If you run into issues with this module please contact
ken.martin@kitware.com

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
