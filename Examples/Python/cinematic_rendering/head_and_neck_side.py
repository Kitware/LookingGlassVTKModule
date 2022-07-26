from pathlib import Path
import signal
import sys

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
volume_prop.SetScalarOpacityUnitDistance(1.1509873813643359)
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
light1.SetPosition(664.1753334478092, -764.1332325516312, 1531.7763222661922)
light1.SetFocalPoint(2.3145678210753973, 107.51311161468337, 685.2954455758255)

light2 = vtk.vtkLight()
light2.SetPositional(True)
light2.SetPosition(1256.510929377065, 167.79381993643014, 1782.3720818721454)
light2.SetFocalPoint(-6.485778011067196, 103.69246185935256, 685.3122540216165)
light2.SetConeAngle(48)

light3 = vtk.vtkLight()
light3.SetLightType(vtk.VTK_LIGHT_TYPE_HEADLIGHT)

light4 = vtk.vtkLight()
light4.SetPosition(4262.688089791584, 897.4946909198911, 3644.5979620782014)
light4.SetFocalPoint(-5.6980060278888836, 105.2549118197128, 684.840396121795)

for light in (light1, light2, light3, light4):
    ren.AddLight(light)

# Adjust the camera
camera = ren.GetActiveCamera()
camera.SetPosition(583.0964911648575, 428.79195498984876, 661.5251868954026)
camera.SetFocalPoint(46.636364907324946, 124.35708927120413, 675.7867746525995)
camera.SetViewUp(0.044156836035224814, -0.031033174795286715, 0.998542495787477)

ren.AddActor(volume)

print("Starting rendering. Click display window and press 'q' to exit")

renWin.Render()
iren.Start()
