"""Generate a camera animation of the head and neck example

Running this script generates a video quilt file that can be
played on a Looking Glass device via HoloPlay Studio. While the
script is running, the latest frame generated can be seen on the
device. However, due to computing constraints, the time between the
frames may be significantly longer than that of the video file that
is produced.

Note that HoloPlay Studio requires the video file to be in MP4
or WEBM format. If VTK writes out the video in a different format,
it may need to be converted. Here's an example of a conversion
using ffmpeg:

ffmpeg -i input_file.ogv -c:v libx265 -crf 10 output_file.mp4

Where `libx265` specifies to use the h.265 video codec, and `crf`
specifies the video quality (where 0 is lossless and 51 is the lowest
quality).
"""

from pathlib import Path
import signal

import numpy as np

import vtk

from vtk import vtkRenderingLookingGlass

# Kill the program when ctrl-c is used
signal.signal(signal.SIGINT, signal.SIG_DFL)

data_url = "https://data.kitware.com/api/v1/file/62daaa69bddec9d0c4bfd42a/download"
data_file = "head_and_neck_ct.vtk"

if not Path(data_file).exists():
    print(f"Downloading {data_file}...")

    # Download the data
    import urllib.request
    urllib.request.urlretrieve(data_url, data_file)

    print("Download complete")


# Volumetric shadow settings
volumetric_scattering_blending = 2
global_illumination_reach = 0.2

# Read the data
reader = vtk.vtkStructuredPointsReader()
reader.SetFileName(data_file)
reader.Update()
image = reader.GetOutput()

ren = vtk.vtkRenderer()
ren.SetBackground(0, 0, 0)

# renWin = vtk.vtkRenderWindow()
# renWin.SetSize(800, 800)
renWin = vtkRenderingLookingGlass.vtkLookingGlassInterface.CreateLookingGlassRenderWindow()

if renWin.GetDeviceType() == "standard":
    # This looks better on large settings
    renWin.SetDeviceType("large")

renWin.AddRenderer(ren)

volume_prop = vtk.vtkVolumeProperty()
volume_prop.ShadeOn()
volume_prop.SetScalarOpacityUnitDistance(1.5)
volume_prop.SetInterpolationType(vtk.VTK_LINEAR_INTERPOLATION)

# volume_prop.SetDiffuse(1.0)
# volume_prop.SetAmbient(0.3)
# volume_prop.SetSpecular(0.1)
# volume_prop.SetSpecularPower(2.0)

color = vtk.vtkColorTransferFunction()
color.SetColorSpaceToDiverging()
color.AllowDuplicateScalarsOn()
color.SetRange(-2864.0, 3231.0)
color.AddRGBPoint(66.0, 0.529412, 0.156863, 0.145098)
color.AddRGBPoint(124.133, 0.541176, 0.168627, 0.168627)
color.AddRGBPoint(144.24, 0.882353, 0.603922, 0.290196)
color.AddRGBPoint(167.027, 0.74902, 0.662745, 0.662745)
color.AddRGBPoint(189.814, 0.839216, 0.643137, 0.47451)
color.AddRGBPoint(217.963, 1.0, 0.937033, 0.954531)
color.AddRGBPoint(409.412, 0.521569, 0.529412, 0.6)
color.AddRGBPoint(3231.0, 0.827451, 0.658824, 1.0)
volume_prop.SetColor(color)

opacity = vtk.vtkPiecewiseFunction()
opacity.AllowDuplicateScalarsOn()
opacity.AddPoint(-2864.0, 0.0)
opacity.AddPoint(-136.837, 0.0)
opacity.AddPoint(75.8777, 0.0)
opacity.AddPoint(81.2394, 0.180531)
opacity.AddPoint(89.282, 0.339324)
opacity.AddPoint(102.686, 0.439615)
opacity.AddPoint(155.059, 0.536765)
opacity.AddPoint(249.496, 0.661765)
opacity.AddPoint(489.881, 0.860294)
opacity.AddPoint(3231.0, 0.875)
volume_prop.SetScalarOpacity(opacity)

volume = vtk.vtkVolume()
volume.SetProperty(volume_prop)

mapper = vtk.vtkGPUVolumeRayCastMapper()
mapper.SetInputData(image)
mapper.SetBlendModeToComposite()
mapper.UseJitteringOn()
mapper.SetVolumetricScatteringBlending(volumetric_scattering_blending)
mapper.SetGlobalIlluminationReach(global_illumination_reach)
volume.SetMapper(mapper)

# Set up the lights
ren.ClearLights()
ren.RemoveAllLights()

light1 = vtk.vtkLight()
light1.SetPosition(1168.3321375771598, 1103.6856851906775, 1332.4963456361356)
light1.SetFocalPoint(-0.8162294972326031, 98.8382341239513, 679.6689345111414)

light2 = vtk.vtkLight()
light2.SetPositional(True)
light2.SetPosition(-1775.4343728259555, 1468.4250542292532, 2617.078008761073)
light2.SetFocalPoint(1.3923130086732531, 96.00624836304287, 679.096642063474)
light2.SetConeAngle(41.4)

light3 = vtk.vtkLight()
light3.SetLightType(vtk.VTK_LIGHT_TYPE_HEADLIGHT)
light3.SetIntensity(0.1)

light4 = vtk.vtkLight()
light4.SetPosition(10.192943271554737, 1032.6546874457033, 534.6410709339551)
light4.SetFocalPoint(-0.8925092249604016, 98.88633737379182, 679.6079704046107)

for light in (light1, light2, light3, light4):
    ren.AddLight(light)

# Adjust the camera
camera = ren.GetActiveCamera()
camera.SetPosition(39.28837393041376, 753.6814804302966, 614.2672454604013)
camera.SetFocalPoint(6.708621478744433, 164.7269185790301, 675.9562549433067)
camera.SetViewUp(0, 0, 1)


def rotate_camera_xy(angle):
    angle = np.radians(angle)

    pos = np.array(camera.GetPosition())
    fp = np.array(camera.GetFocalPoint())

    # Translate so that the focal point is centered
    pos -= fp

    # Perform the rotation
    s = np.sin(angle)
    c = np.cos(angle)

    x = pos[0] * c - pos[1] * s
    y = pos[0] * s + pos[1] * c

    pos[0] = x
    pos[1] = y

    # Translate back
    pos += fp

    camera.SetPosition(pos)


ren.AddActor(volume)

suffix = renWin.QuiltFileSuffix()
ext = renWin.MovieFileExtension()
renWin.StartRecordingQuilt(f"head_and_neck{suffix}.{ext}")

angle_range = (-60, 60)
num_frames = 100

step_size = (angle_range[1] - angle_range[0]) / (num_frames - 1)

# First, rotate to the first angle
rotate_camera_xy(angle_range[0])
renWin.Render()

# Now step through the rest of the angles
for _ in range(num_frames):
    rotate_camera_xy(step_size)
    renWin.Render()

# Now, go backwards
for _ in range(num_frames):
    rotate_camera_xy(-step_size)
    renWin.Render()

renWin.StopRecordingQuilt()
