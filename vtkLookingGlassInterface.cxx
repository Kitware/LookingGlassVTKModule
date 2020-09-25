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
#include "vtkObjectFactory.h"
#include "vtkOpenGLFramebufferObject.h"
#include "vtkOpenGLQuadHelper.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLShaderCache.h"
#include "vtkOpenGLState.h"
#include "vtkShaderProgram.h"
#include "vtkTextureObject.h"
#include "vtkVectorOperators.h"

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

//------------------------------------------------------------------------------
vtkOpenGLRenderWindow* vtkLookingGlassInterface::CreateLookingGlassRenderWindow()
{
#ifdef WIN32
  return vtkWin32LookingGlassRenderWindow::New();
#endif
#ifdef VTK_USE_X
  return vtkXLookingGlassRenderWindow::New();
#endif
#ifdef VTK_USE_COCOA
  return vtkCocoaLookingGlassRenderWindow::New();
#endif
}

vtkStandardNewMacro(vtkLookingGlassInterface);

//------------------------------------------------------------------------------
vtkLookingGlassInterface::vtkLookingGlassInterface()
  : Connected(false)
  , DeviceIndex(0)
  , UseClippingLimits(false)
  , FarClippingLimit(1.2)
  , NearClippingLimit(0.8)
  , ViewAngle(30.0)
  , FinalBlend(nullptr)
  , Initialized(false)
  , RenderFramebuffer(nullptr)
  , QuiltFramebuffer(nullptr)
  , QuiltQuality(1)
{
  this->DisplayPosition[0] = 0;
  this->DisplayPosition[1] = 0;
  this->DisplaySize[0] = 1280;
  this->DisplaySize[1] = 720;
  this->QuiltSize[0] = 4096;
  this->QuiltSize[1] = 4096;
  this->QuiltTiles[0] = 5;
  this->QuiltTiles[1] = 9;
  this->QuiltTexture = vtkTextureObject::New();
}

//------------------------------------------------------------------------------
vtkLookingGlassInterface::~vtkLookingGlassInterface()
{
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

  // must tear down the message pipe before shut down the app
  if (this->Connected)
  {
    hpc_TeardownMessagePipe();
    this->Connected = false;
  }
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

  // #ifdef VTK_USE_COCOA
  //   renWin->FullScreenOn();
  // #endif

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

// set up the quilt settings
void vtkLookingGlassInterface::SetupQuiltSettings(int preset)
{
  // there are 3 presets:
  switch (preset)
  {
    case 0: // standard
      this->QuiltSize[0] = 2048;
      this->QuiltSize[1] = 2048;
      this->QuiltTiles[0] = 4;
      this->QuiltTiles[1] = 8;
      break;
    default:
    case 1: // hires
      this->QuiltSize[0] = 4096;
      this->QuiltSize[1] = 4096;
      this->QuiltTiles[0] = 5;
      this->QuiltTiles[1] = 9;
      break;
    case 2: // 8k
      this->QuiltSize[0] = 4096 * 2;
      this->QuiltSize[1] = 4096 * 2;
      this->QuiltTiles[0] = 5;
      this->QuiltTiles[1] = 9;
      break;
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
  }

  this->SetupQuiltSettings(this->QuiltQuality);

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
  if (!this->FinalBlend)
  {
    // use a simple vertex shader
    std::string vshader =
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

    // just add the standard VTK header to the fragment shader
    std::string fshader = "//VTK::System::Dec\n\n";

    if (this->Connected)
    {
      fshader += hpc_LightfieldFragShaderGLSL;
    }
    else
    {
      fshader +=
        R"***(
          in vec2 texCoords;
          out vec4 fragColor;
          uniform sampler2D screenTex;
          void main()
          {
        		fragColor = vec4(texture(screenTex, texCoords.xy).rgb, 1.0);
          }
      )***";
    }
    this->FinalBlend = new vtkOpenGLQuadHelper(renWin, vshader.c_str(), fshader.c_str(), "");
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->FinalBlend->Program);
  }

  if (this->FinalBlend->Program)
  {
    auto& prog = this->FinalBlend->Program;

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
    this->FinalBlend->Render();

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
