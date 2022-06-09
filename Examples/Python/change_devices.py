#!/usr/bin/env python3

"""This example demonstrates changing device types

The code loops through each device type and writes out a quilt image for
each one.
"""

import vtk
from vtk import vtkRenderingLookingGlass

# Create a looking glass render window
renWin = vtkRenderingLookingGlass.vtkLookingGlassInterface.CreateLookingGlassRenderWindow()

# Declare a VTK rendering process, and add it to the window
ren = vtk.vtkRenderer()
renWin.AddRenderer(ren)

# Add a cone
cone = vtk.vtkConeSource()
cone.SetRadius(2)
cone.SetHeight(4)
cone.SetCenter(4, 4, 2)
cone.SetDirection(0, 0, 0)
coneMapper = vtk.vtkPolyDataMapper()
coneMapper.SetInputConnection(cone.GetOutputPort())
coneActor = vtk.vtkActor()
coneActor.SetMapper(coneMapper)
ren.AddActor(coneActor)

# Initialize the window
renWin.Initialize()
ren.ResetCamera()
ren.GetActiveCamera().SetViewAngle(30)

# Loop through each device type
for device_type in renWin.GetDeviceTypes():
    # Switch to the device
    renWin.SetDeviceType(device_type)

    # Must render before writing out the quilt
    renWin.Render()

    # The suffix encodes the tile pattern, such as "_qs5x9"
    # This allows software to automatically determine the tile pattern
    suffix = renWin.QuiltFileSuffix()

    # Write out the quilt
    renWin.SaveQuilt(f"quilt_{device_type}{suffix}.png")
