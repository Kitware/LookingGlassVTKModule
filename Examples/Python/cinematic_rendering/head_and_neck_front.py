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

renWin = vtkRenderingLookingGlass.vtkLookingGlassInterface.CreateLookingGlassRenderWindow()

if renWin.GetDeviceType() == "standard":
    # This looks better on large settings
    renWin.SetDeviceType("large")

renWin.AddRenderer(ren)

iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

# The desired update rate is for a single render of the renderer.
# For the looking glass, however, there will be one render for each
# tile. Thus, we should modify the desired update rate to be the
# previous update rate * the number of tiles, in order to improve
# render rate during interaction.
num_tiles = np.prod(renWin.GetInterface().GetQuiltTiles())
iren.SetDesiredUpdateRate(iren.GetDesiredUpdateRate() * num_tiles)

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
camera.SetViewUp(0.01802597816737277, 0.10317003294046821, 0.9945003813041864)

ren.AddActor(volume)

# Set up the interactor style
iren_style = vtk.vtkInteractorStyleTrackballCamera()
iren.SetInteractorStyle(iren_style)

def adjust_resolution(mapper, event):
    interactive = volume.GetAllocatedRenderTime() < 1
    mapper.SetUseJittering(not interactive)
    volume_prop.SetShade(not interactive)

mapper.AddObserver("VolumeMapperRenderStartEvent", adjust_resolution)

print("Starting rendering. Click display window and press 'q' to exit")

iren.Start()
