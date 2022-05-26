#!/usr/bin/env python3

"""This example demonstrates creating a quilt image with Python

A quilt image is a regular image file that can be loaded onto a looking glass
device via the HoloPlay Studio desktop application, in order to display a
holographic image on the device.
"""

import vtk

# Create a looking glass render window
renWin = vtk.vtkLookingGlassInterface.CreateLookingGlassRenderWindow()

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

# Must render before writing out the quilt
renWin.Render()

# Write out the quilt
renWin.SaveQuilt("quilt.png")
