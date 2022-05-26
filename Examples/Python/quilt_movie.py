#!/usr/bin/env python3

"""This example demonstrates creating a quilt movie with Python

A quilt movie is a regular video file that can be loaded onto a looking glass
device via the HoloPlay Studio desktop application, in order to display a
holographic video on the device.

The quilt movie, however, needs to be in a format that HoloPlay Studio can
recognize, such as MP4. VTK will try to write to MP4 if it is available.
Otherwise, the file may need to be converted to MP4 externally before using
it with HoloPlay Studio.

Creating quilt movies can take some time because encoding the video frames
can be a slow process. This example demonstrates a rotating cone.
"""

import math

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
cone.SetDirection(0, 0, 1)
coneMapper = vtk.vtkPolyDataMapper()
coneMapper.SetInputConnection(cone.GetOutputPort())
coneActor = vtk.vtkActor()
coneActor.SetMapper(coneMapper)
ren.AddActor(coneActor)

# Initialize the window
renWin.Initialize()
ren.ResetCamera()
ren.GetActiveCamera().SetViewAngle(30)

# Get the movie extension. This will be "mp4" if it is available. Otherwise,
# it may be "avi" or "ogv".
ext = renWin.MovieFileExtension()

# Start recording
renWin.StartRecordingQuilt(f"quilt.{ext}")

# Change the cone direction. Each render will produce a frame in the file.
# This may be slow due to the time required to encode each frame into the
# video file.
num_frames = 30
for i in range(num_frames):
    x = i / num_frames * math.pi
    cone.SetDirection(-math.cos(x), 0, math.sin(x))
    renWin.Render()

# Finalize the file
renWin.StopRecordingQuilt()
