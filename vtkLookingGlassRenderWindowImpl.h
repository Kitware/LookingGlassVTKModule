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
  this->Interface = vtkLookingGlassInterface::New();
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
