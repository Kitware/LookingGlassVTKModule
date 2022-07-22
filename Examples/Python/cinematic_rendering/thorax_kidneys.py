from pathlib import Path
import signal
import sys

import vtk

from vtk import vtkRenderingLookingGlass

# Kill the program when ctrl-c is used
signal.signal(signal.SIGINT, signal.SIG_DFL)

data_url = "https://data.kitware.com/api/v1/file/62d9672ebddec9d0c4af738a/download"
data_file = "thorax_kidneys.vti"

if not Path(data_file).exists():
    print(f"Downloading data to {data_file}")

    # Download the data
    import urllib.request
    urllib.request.urlretrieve(data_url, data_file)

    print("Download complete")

# Volumetric shadow settings
volumetric_scattering_blending = 2
global_illumination_reach = 0.35964

# Read the data
reader = vtk.vtkXMLImageDataReader()
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

iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

# Interaction is slow for this example, since it is in high res,
# so just disable it via vtkInteractorStyleUser.
iren_style = vtk.vtkInteractorStyleUser()
iren.SetInteractorStyle(iren_style)

# Use q to quit
def on_key_press(style, event):
    if style.GetKeySym() == 'q':
        sys.exit()

iren_style.AddObserver("KeyPressEvent", on_key_press)

volume_prop = vtk.vtkVolumeProperty()
volume_prop.ShadeOn()
volume_prop.SetScalarOpacityUnitDistance(1)
volume_prop.SetInterpolationType(vtk.VTK_LINEAR_INTERPOLATION)

volume_prop.SetDiffuse(1.0)
volume_prop.SetAmbient(0.3)
volume_prop.SetSpecular(0.1)
volume_prop.SetSpecularPower(2.0)

color = vtk.vtkColorTransferFunction()
color.SetColorSpaceToDiverging()
color.AllowDuplicateScalarsOn()
color.SetRange(66.1394, 1648.0055)
color.AddRGBPoint(66.1394, 0.231373, 0.298039, 0.752941)
color.AddRGBPoint(153.329, 0.678431, 0.4, 0.4)
color.AddRGBPoint(223.911, 0.865003, 0.865003, 0.865003)
color.AddRGBPoint(1540.06, 0.901961, 0.847059, 0.811765)
color.AddRGBPoint(1648.01, 0.705882, 0.0156863, 0.14902)
volume_prop.SetColor(color)

opacity = vtk.vtkPiecewiseFunction()
opacity.AllowDuplicateScalarsOn()
opacity.AddPoint(66.1394, 0)
opacity.AddPoint(165.784, 0)
opacity.AddPoint(165.784, 0.244565)
opacity.AddPoint(423.201, 0.266304)
opacity.AddPoint(423.201, 0.842391)
opacity.AddPoint(1648.01, 1)
volume_prop.SetScalarOpacity(opacity);

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
light1.SetIntensity(0.6)
light1.SetPosition(-222.0339, -32.5699, 37.5177)
light1.SetFocalPoint(-118.2287, -51.1928, 103.8297)

light2 = vtk.vtkLight()
light2.SetIntensity(0.7)
light2.SetPositional(True)
light2.SetPosition(28.0062, 146.3112, -360.1248)
light2.SetFocalPoint(122.8112, 66.2097, -143.411)
light2.SetConeAngle(48.6)

# Ambient light is a headlight
light3 = vtk.vtkLight()
light3.SetLightType(vtk.VTK_LIGHT_TYPE_HEADLIGHT)
light3.SetIntensity(0.2)

for light in (light1, light2, light3):
    ren.AddLight(light)

# Adjust the camera
camera = ren.GetActiveCamera()
camera.SetPosition(278.224, 890.781, -28.4133)
camera.SetFocalPoint(265.61, 273.136, 129.072)
camera.SetViewUp(-0.00327096, -0.247008, -0.969008)

ren.AddActor(volume)

print("Starting rendering. Click display window and press 'q' to exit")

renWin.Render()
iren.Start()
