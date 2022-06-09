/*=========================================================================

  Copyright (c) 2020 Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * This header includes code common to all OS specific RenderWindows
 */

#include "vtkLookingGlassInterface.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(className);

//------------------------------------------------------------------------------
className::className()
{
  InitializeInterface();
}

//------------------------------------------------------------------------------
void className::InitializeInterface()
{
  if (!this->Interface)
  {
    this->Interface = vtkLookingGlassInterface::New();
  }

  this->Interface->Initialize();
  this->Interface->GetDisplaySize(this->Size[0], this->Size[1]);
  this->Interface->GetDisplayPosition(this->Position[0], this->Position[1]);
  this->BordersOff();
#ifdef VTK_USE_COCOA
  this->FullScreenOn();
#endif
}

//------------------------------------------------------------------------------
className::~className()
{
  this->Finalize();
  this->Interface->Delete();
}

//------------------------------------------------------------------------------
void className::ReleaseGraphicsResources(vtkWindow* renWin)
{
  this->Interface->ReleaseGraphicsResources(renWin);
  this->Superclass::ReleaseGraphicsResources(renWin);
}

//------------------------------------------------------------------------------
// We override this method to render the tiles required by the looking glass
// display then render the resulting lightfield to the window
void className::DoStereoRender()
{
  this->StereoUpdate();

  int renderSize[2];
  this->Interface->GetRenderSize(renderSize);

  int origSize[2] = { this->Size[0], this->Size[1] };
  this->InStereoRender = true;
  this->Size[0] = renderSize[0];
  this->Size[1] = renderSize[1];

  this->Interface->RenderQuilt(this);

  this->InStereoRender = false;
  this->Size[0] = origSize[0];
  this->Size[1] = origSize[1];

  this->Interface->DrawLightField(this);
}

//------------------------------------------------------------------------------
// Get the current size of the window (or when rendering tiles the tile size)
int* className::GetSize(void)
{
  // when rendering to a tile we have a different size already set
  // just return the ivar unchanged
  if (this->InStereoRender)
  {
    return this->Size;
  }

  return this->Superclass::GetSize();
}

//------------------------------------------------------------------------------
void className::SetLGDeviceIndex(int index)
{
  this->Interface->SetDeviceIndex(index);
}

//------------------------------------------------------------------------------
void className::SaveQuilt(const char* fileName)
{
  this->Interface->SaveQuilt(fileName);
}

//------------------------------------------------------------------------------
void className::StartRecordingQuilt(const char* fileName)
{
  this->Interface->StartRecordingQuilt(fileName);
}

//------------------------------------------------------------------------------
void className::StopRecordingQuilt()
{
  this->Interface->StopRecordingQuilt();
}

//------------------------------------------------------------------------------
const char* className::MovieFileExtension()
{
  return vtkLookingGlassInterface::MovieFileExtension();
}

//------------------------------------------------------------------------------
std::string className::QuiltFileSuffix() const
{
  return this->Interface->QuiltFileSuffix();
}

//------------------------------------------------------------------------------
void className::SetUseClippingLimits(bool b)
{
  this->Interface->SetUseClippingLimits(b);
}

//------------------------------------------------------------------------------
bool className::GetUseClippingLimits() const
{
  return this->Interface->GetUseClippingLimits();
}

//------------------------------------------------------------------------------
void className::SetFarClippingLimit(double d)
{
  this->Interface->SetFarClippingLimit(d);
}

//------------------------------------------------------------------------------
double className::GetFarClippingLimit() const
{
  return this->Interface->GetFarClippingLimit();
}

//------------------------------------------------------------------------------
void className::SetNearClippingLimit(double d)
{
  this->Interface->SetNearClippingLimit(d);
}

//------------------------------------------------------------------------------
double className::GetNearClippingLimit() const
{
  return this->Interface->GetNearClippingLimit();
}

//------------------------------------------------------------------------------
bool className::IsRecordingQuilt() const
{
  return this->Interface->IsRecordingQuilt();
}

//------------------------------------------------------------------------------
void className::SetDeviceIndex(int i)
{
  this->Interface->SetDeviceIndex(i);
}

//------------------------------------------------------------------------------
int className::GetDeviceIndex() const
{
  return this->Interface->GetDeviceIndex();
}

//------------------------------------------------------------------------------
void className::SetDeviceType(const std::string& t)
{
  // Must re-build the interface from scratch
  if (this->Interface)
  {
    this->Interface->ReleaseGraphicsResources(this);
    this->Interface->Delete();
    this->Interface = nullptr;
  }

  this->Interface = vtkLookingGlassInterface::New();
  this->Interface->SetDeviceType(t);
  InitializeInterface();
}

//------------------------------------------------------------------------------
std::string className::GetDeviceType() const
{
  return this->Interface->GetDeviceType();
}

//------------------------------------------------------------------------------
std::vector<std::string> className::GetDeviceTypes()
{
  std::vector<std::string> types;
  for (const auto device : vtkLookingGlassInterface::GetDevices())
  {
    types.push_back(device.first);
  }

  return types;
}
