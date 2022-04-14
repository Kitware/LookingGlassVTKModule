/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkLookingGlassInterface.cxx

  Copyright (c) 2020 Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

This software uses content from HoloPlayCore, which is distributed under the
following license:

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

=========================================================================*/
#include "vtkLookingGlassInterface.h"

#include "HoloPlayCore.h"
#include "HoloPlayShadersOpen.h"

#include "vtkCamera.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLFramebufferObject.h"
#include "vtkOpenGLQuadHelper.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLShaderCache.h"
#include "vtkOpenGLState.h"
#include "vtkPNGWriter.h"
#include "vtkShaderProgram.h"
#include "vtkTextureObject.h"
#include "vtkVectorOperators.h"
#include "vtkWindowToImageFilter.h"

#include "vtk_glew.h"

#include "vtkRenderingOpenGLConfigure.h"

#ifdef WIN32
#include "vtkWin32LookingGlassRenderWindow.h"
#endif
#ifdef VTK_USE_X
#include "vtkXLookingGlassRenderWindow.h"
#endif
#ifdef VTK_USE_COCOA
#include "vtkCocoaLookingGlassRenderWindow.h"
#endif

#ifdef WIN32
// Use the MP4 writer on Windows
#include "vtkMP4Writer.h"
using MovieWriterClass = vtkMP4Writer;
static const char* MovieExtension = "mp4";
#else
// If not Windows, use FFMPEG if it is available
#if VTK_MODULE_ENABLE_VTK_IOFFMPEG
#include "vtkFFMPEGWriter.h"
using MovieWriterClass = vtkFFMPEGWriter;
static const char* MovieExtension = "avi";
#else
// Otherwise, use Ogg Theora.
#include "vtkOggTheoraWriter.h"
using MovieWriterClass = vtkOggTheoraWriter;
static const char* MovieExtension = "ogv";
#endif
#endif

//------------------------------------------------------------------------------
const char* vtkLookingGlassInterface::MovieFileExtension()
{
  return MovieExtension;
}

//------------------------------------------------------------------------------
vtkOpenGLRenderWindow* vtkLookingGlassInterface::CreateLookingGlassRenderWindow(int deviceIndex)
{
#ifdef WIN32
  auto renWin = vtkWin32LookingGlassRenderWindow::New();
#endif
#ifdef VTK_USE_X
  auto renWin = vtkXLookingGlassRenderWindow::New();
#endif
#ifdef VTK_USE_COCOA
  auto renWin = vtkCocoaLookingGlassRenderWindow::New();
#endif

  renWin->SetLGDeviceIndex(deviceIndex);

  return renWin;
}

vtkStandardNewMacro(vtkLookingGlassInterface);

//------------------------------------------------------------------------------
vtkLookingGlassInterface::vtkLookingGlassInterface()
  : Connected(false)
  , SavingQuilt(false)
  , DeviceIndex(0)
  , UseClippingLimits(false)
  , FarClippingLimit(1.2)
  , NearClippingLimit(0.8)
  , ViewAngle(30.0)
  , FinalBlend(nullptr)
  , QuiltBlend(nullptr)
  , Initialized(false)
  , RenderFramebuffer(nullptr)
  , QuiltFramebuffer(nullptr)
  , QuiltQuality(1)
  , QuiltExportMagnification(2)
  , IsRecording(false)
  , MovieWindowToImageFilter(nullptr)
  , MovieWriter(nullptr)
{
  this->DisplayPosition[0] = 0;
  this->DisplayPosition[1] = 0;
  this->DisplaySize[0] = 1280;
  this->DisplaySize[1] = 720;
  this->QuiltTexture = vtkTextureObject::New();
}

//------------------------------------------------------------------------------
vtkLookingGlassInterface::~vtkLookingGlassInterface()
{
  if (this->IsRecording)
  {
    this->StopRecordingQuilt();
  }

  if (this->RenderFramebuffer != nullptr)
  {
    vtkErrorMacro(<< "Render Framebuffer should have been deleted in "
                     "ReleaseGraphicsResources().");
  }
  if (this->QuiltFramebuffer != nullptr)
  {
    vtkErrorMacro(<< "QuiltFramebuffer should have been deleted in "
                     "ReleaseGraphicsResources().");
  }
  if (this->QuiltTexture != nullptr)
  {
    this->QuiltTexture->Delete();
    this->QuiltTexture = nullptr;
  }
  if (this->FinalBlend != nullptr)
  {
    delete this->FinalBlend;
    this->FinalBlend = nullptr;
  }
  if (this->QuiltBlend != nullptr)
  {
    delete this->QuiltBlend;
    this->QuiltBlend = nullptr;
  }

  if (this->MovieWindowToImageFilter != nullptr)
  {
    this->MovieWindowToImageFilter->Delete();
    this->MovieWindowToImageFilter = nullptr;
  }

  if (this->MovieWriter != nullptr)
  {
    this->MovieWriter->Delete();
    this->MovieWriter = nullptr;
  }

  // must tear down the message pipe before shut down the app
  if (this->Connected)
  {
    hpc_TeardownMessagePipe();
    this->Connected = false;
  }
}

vtkLookingGlassInterface::DeviceSettings::DeviceSettings(
  const std::string& name, int quiltWidth, int quiltHeight,
  int quiltTilesColumns, int quiltTilesRows)
{
  this->Name = name;
  this->QuiltSize[0] = quiltWidth;
  this->QuiltSize[1] = quiltHeight;
  this->QuiltTiles[0] = quiltTilesColumns;
  this->QuiltTiles[1] = quiltTilesRows;
}

std::map<std::string, vtkLookingGlassInterface::DeviceSettings>
vtkLookingGlassInterface::GetSettingsByDevice()
{

  static std::map<std::string, DeviceSettings> settingsByDevice = {};

  if (settingsByDevice.empty()) {
    settingsByDevice["standard"] = DeviceSettings(
      "8.9\" Looking Glass Display (Standard)",
      2048, 2048, // QuiltSize
      4, 8        // QuiltTiles
    );
    settingsByDevice["portrait"] = DeviceSettings(
      "7.9\" Looking Glass Display (Portrait)",
      3360, 3360, // QuiltSize
      8, 6        // QuiltTiles
    );
    settingsByDevice["large"] = DeviceSettings(
      "15.6\" Looking Glass Display (Large)",
      4096, 4096, // QuiltSize
      5, 9        // QuiltTiles
    );
    settingsByDevice["pro"] = DeviceSettings(
      "15.6\" Looking Glass Display (Pro)",
      4096, 4096, // QuiltSize
      5, 9        // QuiltTiles
    );
    settingsByDevice["8k"] = DeviceSettings(
      "30\" Looking Glass Display (8K)",
      4096 * 2, 4096 * 2, // QuiltSize
      5, 9                // QuiltTiles
    );
  }

  return settingsByDevice;
}

vtkLookingGlassInterface::DeviceSettings vtkLookingGlassInterface::GetSettingsForDevice(
  const std::string deviceType)
{
  return GetSettingsByDevice().at(deviceType);
}

vtkLookingGlassInterface::DeviceTypes vtkLookingGlassInterface::GetDevices()
{
  vtkLookingGlassInterface::DeviceTypes types;
  for (auto const device : GetSettingsByDevice())
  {
    types.push_back(std::make_pair(device.first, device.second.Name));
  }

  return types;
}

vtkOpenGLRenderWindow* vtkLookingGlassInterface::CreateSharedLookingGlassRenderWindow(
  vtkOpenGLRenderWindow* srcWin)
{
  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(vtkOpenGLRenderWindow::New());
  this->Initialize();
  renWin->SetSharedRenderWindow(srcWin);
  renWin->SetSize(this->DisplaySize[0], this->DisplaySize[1]);
  renWin->SetPosition(this->DisplayPosition[0], this->DisplayPosition[1]);
  renWin->BordersOff();

  return renWin;
}

bool vtkLookingGlassInterface::GetLookingGlassInfo()
{
  hpc_client_error errco = hpc_InitializeApp("VTK", hpc_LICENSE_NONCOMMERCIAL);
  if (errco)
  {
    std::string errstr;
    switch (errco)
    {
      case hpc_CLIERR_NOSERVICE:
        errstr = "HoloPlay Service not running";
        break;
      case hpc_CLIERR_SERIALIZEERR:
        errstr = "Client message could not be serialized";
        break;
      case hpc_CLIERR_VERSIONERR:
        errstr = "Incompatible version of HoloPlay Service";
        break;
      case hpc_CLIERR_PIPEERROR:
        errstr = "Interprocess pipe broken";
        break;
      case hpc_CLIERR_SENDTIMEOUT:
        errstr = "Interprocess pipe send timeout";
        break;
      case hpc_CLIERR_RECVTIMEOUT:
        errstr = "Interprocess pipe receive timeout";
        break;
      default:
        errstr = "Unknown error";
        break;
    }
    vtkErrorMacro("Client access error (code = " << errco << "): " << errstr);
    return false;
  }

  char buf[1000];
  hpc_GetHoloPlayCoreVersion(buf, 1000);
  vtkDebugMacro("HoloPlay Core version " << buf);
  hpc_GetHoloPlayServiceVersion(buf, 1000);
  vtkDebugMacro("HoloPlay Service version " << buf);
  int num_displays = hpc_GetNumDevices();
  vtkDebugMacro("connected device count: " << num_displays);
  if (num_displays < 1)
  {
    return false;
  }
  for (int i = 0; i < num_displays; ++i)
  {
    vtkDebugMacro("Device information for display %d:\n" << i);
    hpc_GetDeviceHDMIName(i, buf, 1000);
    vtkDebugMacro("\tDevice name: " << buf);
    hpc_GetDeviceType(i, buf, 1000);
    vtkDebugMacro("\tDevice type: " << buf);
    vtkDebugMacro("\nWindow parameters for display: " << i);
    vtkDebugMacro(
      "\tPosition: " << hpc_GetDevicePropertyWinX(i) << ", " << hpc_GetDevicePropertyWinY(i));
    vtkDebugMacro(
      "\tSize: " << hpc_GetDevicePropertyScreenW(i) << ", " << hpc_GetDevicePropertyScreenH(i));
    vtkDebugMacro("\tAspect ratio: " << hpc_GetDevicePropertyDisplayAspect(i));
    vtkDebugMacro("Shader uniforms for display " << i);
    vtkDebugMacro("\tpitch: " << hpc_GetDevicePropertyPitch(i));
    vtkDebugMacro("\ttilt: " << hpc_GetDevicePropertyTilt(i));
    vtkDebugMacro("\tcenter: " << hpc_GetDevicePropertyCenter(i));
    vtkDebugMacro("\tsubp: " << hpc_GetDevicePropertySubp(i));
    vtkDebugMacro("\tviewCone: " << hpc_GetDevicePropertyFloat(i, "/calibration/viewCone/value"));
    vtkDebugMacro("\tfringe: " << hpc_GetDevicePropertyFringe(i));
    vtkDebugMacro("\tRI: " << hpc_GetDevicePropertyRi(i)
                           << "\n \tBI: " << hpc_GetDevicePropertyBi(i)
                           << "\tinvView: " << hpc_GetDevicePropertyInvView(i));
  }

  return true;
}

void vtkLookingGlassInterface::SetupQuiltSettings(const DeviceSettings& settings)
{
  std::copy(settings.QuiltSize, settings.QuiltSize + 2, this->QuiltSize);
  std::copy(settings.QuiltTiles, settings.QuiltTiles + 2, this->QuiltTiles);
};

// set up the quilt settings
void vtkLookingGlassInterface::SetupQuiltSettings(int preset)
{
  // there are 3 presets:
  switch (preset)
  {
    case 0:
    { // standard
      auto deviceSettings = GetSettingsForDevice("standard");
      this->SetupQuiltSettings(deviceSettings);
    }
    break;
    default:
    case 1:
    { // hires - i assume this is large or pro?
      auto deviceSettings = GetSettingsForDevice("large");
      this->SetupQuiltSettings(deviceSettings);
    }
    break;
    case 2:
    { // 8k
      auto deviceSettings = GetSettingsForDevice("8k");
      this->SetupQuiltSettings(deviceSettings);
    }
    break;
  }
}

// set up quilt settings for a given device
void vtkLookingGlassInterface::SetupQuiltSettings(const std::string& deviceType)
{
  auto byDevice = this->GetSettingsByDevice();
  if (byDevice.count(deviceType))
  {
    auto deviceSettings = byDevice[deviceType];
    this->SetupQuiltSettings(deviceSettings);
  }
  else
  {
    // Issue warning and default to "large" device
    vtkWarningMacro(
      "Unrecognized device type: '" << deviceType << "', defaulting to setting for 'large' device");
    auto deviceSettings = GetSettingsForDevice("large");
    this->SetupQuiltSettings(deviceSettings);
  }
}

//------------------------------------------------------------------------------
// Initialize the rendering window, must be called first
void vtkLookingGlassInterface::Initialize(void)
{
  if (this->Initialized)
  {
    return;
  }

  if (!GetLookingGlassInfo())
  {
    // must tear down the message pipe before shut down the app
    hpc_TeardownMessagePipe();
    this->Connected = false;
  }
  else
  {
    this->Connected = true;

    // get the viewcone here, which is used as a const
    this->ViewAngle = hpc_GetDevicePropertyFloat(this->DeviceIndex, "/calibration/viewCone/value");

    // get the window coordinate from the uniform
    this->DisplaySize[0] = hpc_GetDevicePropertyScreenW(this->DeviceIndex);
    this->DisplaySize[1] = hpc_GetDevicePropertyScreenH(this->DeviceIndex);
    this->DisplayPosition[0] = hpc_GetDevicePropertyWinX(this->DeviceIndex);
    this->DisplayPosition[1] = hpc_GetDevicePropertyWinY(this->DeviceIndex);

    // get the device type if one hasn't been set
    if (this->DeviceType.empty())
    {
      char buf[100];
      hpc_GetDeviceType(this->DeviceIndex, buf, 100);
      this->DeviceType = buf;
    }
  }

  // If we still don't have a device type default to "large"
  if (this->DeviceType.empty())
  {
    vtkWarningMacro("No Looking Glass device attached defaulting to 'large'");
    this->DeviceType = "large";
  }

  this->SetupQuiltSettings(this->DeviceType);

  this->NumberOfTiles = this->QuiltTiles[0] * this->QuiltTiles[1];

  // compute the render size we need
  this->RenderSize[0] = this->QuiltSize[0] / this->QuiltTiles[0];
  this->RenderSize[1] = this->QuiltSize[1] / this->QuiltTiles[1];

  this->Initialized = true;
}

//=========================================================
// set up the camera with the view and the shader of the rendering object
void vtkLookingGlassInterface::AdjustCamera(vtkCamera* cam, int currentViewIndex)
{
  // The standard model Looking Glass screen is roughly 4.75" vertically. If we
  // assume the average viewing distance for a user sitting at their desk is
  // about 36", our field of view should be about 14°. There is no correct
  // answer, as it all depends on your expected user's distance from the Looking
  // Glass, but we've found the most success using this figure.
  // const float fov = vtkMath::RadiansFromDegrees(14.0f);
  // float cameraDistance = -cameraSize / tan(fov / 2.0f);

  int totalViews = this->QuiltTiles[0] * this->QuiltTiles[1];
  double offsetAngle = (currentViewIndex / (totalViews - 1.0f) - 0.5f) *
    vtkMath::RadiansFromDegrees(this->ViewAngle); // start at -viewCone * 0.5 and go
                                                  // up to viewCone * 0.5

  double cameraDistance = cam->GetDistance();

  double offset =
    cameraDistance * tan(offsetAngle); // calculate the offset that the camera should move

  vtkVector3d vup;
  cam->GetViewUp(vup.GetData());
  vtkVector3d vpn;
  cam->GetViewPlaneNormal(vpn.GetData());
  vtkVector3d vright = vup.Cross(vpn);
  vtkVector3d tmp;
  cam->GetPosition(tmp.GetData());
  tmp = tmp + vright * offset;
  cam->SetPosition(tmp.GetData());
  cam->GetFocalPoint(tmp.GetData());
  tmp = tmp + vright * offset;
  cam->SetFocalPoint(tmp.GetData());

  double aspectRatio = static_cast<double>(this->DisplaySize[0]) / this->DisplaySize[1];
  double camViewAngle = vtkMath::RadiansFromDegrees(cam->GetViewAngle());
  double winSize = aspectRatio * cameraDistance * tan(camViewAngle / 2.0);

  cam->SetWindowCenter(-offset / winSize, 0);
}

void vtkLookingGlassInterface::DrawLightField(
  vtkOpenGLRenderWindow* renWin, vtkTextureObject* copyTO)
{
  if (copyTO->GetHandle() != this->QuiltTexture->GetHandle())
  {
    copyTO->AssignToExistingTexture(this->QuiltTexture->GetHandle(), GL_TEXTURE_2D);
  }
  this->DrawLightFieldInternal(renWin, copyTO);
}

// draw the quilt onto the currently bound framebuffer
void vtkLookingGlassInterface::DrawLightField(vtkOpenGLRenderWindow* renWin)
{
  this->DrawLightFieldInternal(renWin, this->QuiltTexture);
}

void vtkLookingGlassInterface::DrawLightFieldInternal(
  vtkOpenGLRenderWindow* renWin, vtkTextureObject* tex)
{
  // Simple default vertex and fragment shaders
  static const std::string defaultVS =
    R"***(
    //VTK::System::Dec
    in vec4 ndCoordIn;
    in vec2 texCoordIn;
    out vec2 texCoords;
    void main()
    {
      gl_Position = ndCoordIn;
      texCoords = texCoordIn;
    }
  )***";

  static const std::string defaultFS =
    R"***(
      //VTK::System::Dec

      in vec2 texCoords;
      out vec4 fragColor;
      uniform sampler2D screenTex;
      void main()
      {
    		fragColor = vec4(texture(screenTex, texCoords.xy).rgb, 1.0);
      }
  )***";

  vtkOpenGLQuadHelper* blend = nullptr;

  if (this->SavingQuilt || !this->Connected)
  {
    // Use the QuiltBlend
    if (!this->QuiltBlend)
    {
      this->QuiltBlend = new vtkOpenGLQuadHelper(renWin, defaultVS.c_str(), defaultFS.c_str(), "");
    }
    else
    {
      renWin->GetShaderCache()->ReadyShaderProgram(this->QuiltBlend->Program);
    }
    blend = this->QuiltBlend;
  }
  else
  {
    // Use the FinalBlend
    if (!this->FinalBlend)
    {
      // just add the standard VTK header to the fragment shader
      std::string fshader = "//VTK::System::Dec\n\n";
      fshader += hpc_LightfieldFragShaderGLSL;
      this->FinalBlend = new vtkOpenGLQuadHelper(renWin, defaultVS.c_str(), fshader.c_str(), "");
    }
    else
    {
      renWin->GetShaderCache()->ReadyShaderProgram(this->FinalBlend->Program);
    }
    blend = this->FinalBlend;
  }

  if (blend->Program)
  {
    auto& prog = blend->Program;

    if (this->Connected)
    {
      // prog->SetUniformi("debug", 1);
      prog->SetUniformf("pitch", hpc_GetDevicePropertyPitch(this->DeviceIndex));
      prog->SetUniformf("tilt", hpc_GetDevicePropertyTilt(this->DeviceIndex));
      prog->SetUniformf("center", hpc_GetDevicePropertyCenter(this->DeviceIndex));
      prog->SetUniformi("invView", hpc_GetDevicePropertyInvView(this->DeviceIndex));
      prog->SetUniformi("quiltInvert", 0);
      prog->SetUniformf("subp", hpc_GetDevicePropertySubp(this->DeviceIndex));
      prog->SetUniformi("ri", hpc_GetDevicePropertyRi(this->DeviceIndex));
      prog->SetUniformi("bi", hpc_GetDevicePropertyBi(this->DeviceIndex));
      prog->SetUniformf("displayAspect", hpc_GetDevicePropertyDisplayAspect(this->DeviceIndex));
      prog->SetUniformf("quiltAspect", hpc_GetDevicePropertyDisplayAspect(this->DeviceIndex));
      prog->SetUniformi("overscan", 0);

      float tmp3[3];
      tmp3[0] = this->QuiltTiles[0];
      tmp3[1] = this->QuiltTiles[1];
      tmp3[2] = this->NumberOfTiles;
      prog->SetUniform3f("tile", tmp3);

      float tmp2[2];
      tmp2[0] = this->RenderSize[0] * this->QuiltTiles[0] / (float)this->QuiltSize[0];
      tmp2[1] = this->RenderSize[1] * this->QuiltTiles[1] / (float)this->QuiltSize[1];
      prog->SetUniform2f("viewPortion", tmp2);
    }

    renWin->GetState()->vtkglDepthMask(GL_FALSE);
    renWin->GetState()->vtkglDisable(GL_DEPTH_TEST);

    renWin->GetState()->vtkglViewport(0, 0, this->DisplaySize[0], this->DisplaySize[1]);
    renWin->GetState()->vtkglScissor(0, 0, this->DisplaySize[0], this->DisplaySize[1]);

    tex->Activate();
    prog->SetUniformi("screenTex", tex->GetTextureUnit());

    // draw the full screen quad using the special shader
    blend->Render();

    tex->Deactivate();

    renWin->GetState()->vtkglDepthMask(GL_TRUE);
  }
}

void vtkLookingGlassInterface::ReleaseGraphicsResources(vtkWindow* w)
{
  if (this->QuiltTexture && w)
  {
    this->QuiltTexture->ReleaseGraphicsResources(w);
  }
  if (this->RenderFramebuffer)
  {
    if (w)
    {
      this->RenderFramebuffer->ReleaseGraphicsResources(w);
    }
    this->RenderFramebuffer->UnRegister(this);
    this->RenderFramebuffer = nullptr;
  }
  if (this->QuiltFramebuffer)
  {
    if (w)
    {
      this->QuiltFramebuffer->ReleaseGraphicsResources(w);
    }
    this->QuiltFramebuffer->UnRegister(this);
    this->QuiltFramebuffer = nullptr;
  }
  if (this->FinalBlend)
  {
    delete this->FinalBlend;
    this->FinalBlend = nullptr;
  }
  if (this->QuiltBlend)
  {
    delete this->QuiltBlend;
    this->QuiltBlend = nullptr;
  }
}

// gets/creates the framebuffers
void vtkLookingGlassInterface::GetFramebuffers(vtkOpenGLRenderWindow* renWin,
  vtkOpenGLFramebufferObject*& renderFramebuffer, vtkOpenGLFramebufferObject*& quiltFramebuffer)
{
  auto ostate = renWin->GetState();

  if (this->QuiltFramebuffer == nullptr)
  {
    ostate->PushFramebufferBindings();
    this->RenderFramebuffer = vtkOpenGLFramebufferObject::New();
    this->RenderFramebuffer->SetContext(renWin);
    this->RenderFramebuffer->Bind();

    // verify that our multisample setting doe snot exceed the hardware
    int multiSamples = 0;
    if (renWin->GetMultiSamples())
    {
      int msamples = 0;
      ostate->vtkglGetIntegerv(GL_MAX_SAMPLES, &msamples);
      if (multiSamples > msamples)
      {
        multiSamples = msamples;
      }
      if (multiSamples == 1)
      {
        multiSamples = 0;
      }
    }
    this->RenderFramebuffer->PopulateFramebuffer(this->RenderSize[0], this->RenderSize[1],
      true,                 // textures
      1, VTK_UNSIGNED_CHAR, // 1 color buffer uchar
      true, 32,             // depth buffer
      multiSamples, renWin->GetStencilCapable() != 0 ? true : false);

    this->QuiltFramebuffer = vtkOpenGLFramebufferObject::New();
    this->QuiltFramebuffer->SetContext(renWin);
    this->QuiltFramebuffer->Bind();

    this->QuiltTexture->SetContext(renWin);
    this->QuiltTexture->Allocate2D(this->QuiltSize[0], this->QuiltSize[1], 4, VTK_UNSIGNED_CHAR);
    this->QuiltTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->QuiltTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->QuiltTexture->SetWrapS(vtkTextureObject::Repeat);
    this->QuiltTexture->SetWrapT(vtkTextureObject::Repeat);

    this->QuiltFramebuffer->AddColorAttachment(0, this->QuiltTexture);
    this->QuiltFramebuffer->ActivateDrawBuffer(0);
    this->QuiltFramebuffer->ActivateReadBuffer(0);

    ostate->PopFramebufferBindings();
  }

  // make sure the size is correct, nop if size is unchanged
  this->RenderFramebuffer->Resize(this->RenderSize[0], this->RenderSize[1]);
  this->QuiltFramebuffer->Resize(this->QuiltSize[0], this->QuiltSize[1]);

  renderFramebuffer = this->RenderFramebuffer;
  quiltFramebuffer = this->QuiltFramebuffer;
}

// Simply compute and return the position in the quilt for a tile
void vtkLookingGlassInterface::GetTilePosition(int tile, int pos[2])
{
  pos[0] = (tile % this->QuiltTiles[0]) * this->RenderSize[0];
  pos[1] = (tile / this->QuiltTiles[0]) * this->RenderSize[1];
}

void vtkLookingGlassInterface::SaveQuilt(vtkOpenGLRenderWindow* rw, const char* fileName)
{
  // Make a deep copy of the render window, render the quilt, and save it
  vtkNew<vtkWindowToImageFilter> filter;
  filter->ShouldRerenderOff();
  filter->SetInput(rw);

  // Increase magnification to produce higher resolution images
  // vtkWindowToImageFilter::SetScale() doesn't seem to do what we want...
  int prevDisplaySize[2] = { this->DisplaySize[0], this->DisplaySize[1] };

  this->DisplaySize[0] = this->QuiltExportMagnification * prevDisplaySize[0];
  this->DisplaySize[1] = this->QuiltExportMagnification * prevDisplaySize[1];
  rw->SetSize(this->DisplaySize);

  // Render once while saving the quilt. This will render the quilt image
  // on the render window. Then we will write out the image.
  // In ParaView, this will perform only the quad render via calling
  // DrawLightField(). It will not re-render the quilt images.
  this->SavingQuilt = true;
  rw->Render();
  this->SavingQuilt = false;

  vtkNew<vtkPNGWriter> writer;
  writer->SetFileName(fileName);
  writer->SetInputConnection(filter->GetOutputPort());
  writer->Write();

  // Restore previous resolution settings
  this->DisplaySize[0] = prevDisplaySize[0];
  this->DisplaySize[1] = prevDisplaySize[1];
  rw->SetSize(this->DisplaySize);

  // Render again for the correct LG display.
  rw->Render();
}

void vtkLookingGlassInterface::StartRecordingQuilt(vtkOpenGLRenderWindow* rw, const char* fileName)
{
  if (this->IsRecording)
  {
    // Already recording
    return;
  }

  // Create necessary objects
  if (!this->MovieWindowToImageFilter)
  {
    this->MovieWindowToImageFilter = vtkWindowToImageFilter::New();
    this->MovieWindowToImageFilter->ShouldRerenderOff();
    this->MovieWindowToImageFilter->ReadFrontBufferOff();
  }

  if (!this->MovieWriter)
  {
    this->MovieWriter = MovieWriterClass::New();
  }

  auto filter = this->MovieWindowToImageFilter;
  auto writer = this->MovieWriter;

  filter->SetInput(rw);
  filter->Update();

  writer->SetInputConnection(filter->GetOutputPort());
  writer->SetFileName(fileName);
  writer->Start();

  this->IsRecording = true;
}

void vtkLookingGlassInterface::WriteQuiltMovieFrame()
{
  if (!this->IsRecording)
  {
    return;
  }

  auto filter = this->MovieWindowToImageFilter;
  auto writer = this->MovieWriter;
  auto rw = filter->GetInput();

  // Increase magnification to produce higher resolution images
  // vtkWindowToImageFilter::SetScale() doesn't seem to do what we want...
  int prevDisplaySize[2] = { this->DisplaySize[0], this->DisplaySize[1] };

  this->DisplaySize[0] = this->QuiltExportMagnification * prevDisplaySize[0];
  this->DisplaySize[1] = this->QuiltExportMagnification * prevDisplaySize[1];
  rw->SetSize(this->DisplaySize);

  // Render once while saving the quilt. This will render the quilt image
  // on the render window. Then we will write out the image.
  // In ParaView, this will perform only the quad render via calling
  // DrawLightField(). It will not re-render the quilt images.
  this->SavingQuilt = true;
  rw->Render();
  this->SavingQuilt = false;

  filter->Modified();
  writer->Write();

  // Restore previous resolution settings
  this->DisplaySize[0] = prevDisplaySize[0];
  this->DisplaySize[1] = prevDisplaySize[1];
  rw->SetSize(this->DisplaySize);
}

void vtkLookingGlassInterface::StopRecordingQuilt()
{
  if (!this->IsRecording)
  {
    // Not recording...
    return;
  }

  auto filter = this->MovieWindowToImageFilter;
  auto writer = this->MovieWriter;
  auto rw = filter->GetInput();

  writer->End();

  this->IsRecording = false;
}
