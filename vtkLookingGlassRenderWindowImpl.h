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

#include <vector>

#include "vtkCamera.h"
#include "vtkLookingGlassInterface.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLFramebufferObject.h"
#include "vtkOpenGLState.h"
#include "vtkRendererCollection.h"
#include "vtkRenderingOpenGLConfigure.h"

#include "vtk_glew.h"

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
  vtkCollectionSimpleIterator rsit;

  this->StereoUpdate();

  // loop over the tiles, render,and blit
  vtkOpenGLFramebufferObject* renderFramebuffer;
  vtkOpenGLFramebufferObject* quiltFramebuffer;
  this->Interface->GetFramebuffers(this, renderFramebuffer, quiltFramebuffer);

  auto ostate = this->GetState();

  ostate->PushFramebufferBindings();
  renderFramebuffer->Bind(GL_READ_FRAMEBUFFER);

  int renderSize[2];
  this->Interface->GetRenderSize(renderSize);

  int tcount = this->Interface->GetNumberOfTiles();

  // save the original camera settings
  vtkRenderer* aren;
  std::vector<vtkCamera*> Cameras;
  for (this->Renderers->InitTraversal(rsit); (aren = this->Renderers->GetNextRenderer(rsit));)
  {
    // Ugly piece of code - we need to know if the camera already
    // exists or not. If it does not yet exist, we must reset the
    // camera here - otherwise it will never be done (missing its
    // oppportunity to be reset in the Render method of the
    // vtkRenderer because it will already exist by that point...)
    if (!aren->IsActiveCameraCreated())
    {
      aren->ResetCamera();
    }
    auto oldCam = aren->GetActiveCamera();
    oldCam->SetLeftEye(1);
    oldCam->Register(this);
    Cameras.push_back(oldCam);
    vtkNew<vtkCamera> newCam;
    aren->SetActiveCamera(newCam);
  }

  // save the current size and temporarily set the new size to the render
  // framebuffer size
  int origSize[2] = { this->Size[0], this->Size[1] };
  this->InStereoRender = true;
  this->Size[0] = renderSize[0];
  this->Size[1] = renderSize[1];

  // loop over all the tiles and render then and blit them to the quilt
  for (int tile = 0; tile < tcount; ++tile)
  {
    renderFramebuffer->Bind(GL_DRAW_FRAMEBUFFER);
    ostate->vtkglViewport(0, 0, renderSize[0], renderSize[1]);
    ostate->vtkglScissor(0, 0, renderSize[0], renderSize[1]);

    {
      int count = 0;
      for (this->Renderers->InitTraversal(rsit); (aren = this->Renderers->GetNextRenderer(rsit));
           ++count)
      {
        // adjust camera
        vtkCamera* cam = aren->GetActiveCamera();
        cam->DeepCopy(Cameras[count]);
        this->Interface->AdjustCamera(cam, tile);

        // limit the clipping range to limit parallax
        double* cRange = cam->GetClippingRange();
        double cameraDistance = cam->GetDistance();

        double nearClippingLimit = this->Interface->GetNearClippingLimit();
        double farClippingLimit = this->Interface->GetFarClippingLimit();

        double newRange[2];
        newRange[0] = cRange[0];
        newRange[1] = cRange[1];
        if (cRange[0] < cameraDistance * nearClippingLimit)
        {
          newRange[0] = cameraDistance * nearClippingLimit;
        }
        if (cRange[1] > cameraDistance * farClippingLimit)
        {
          newRange[1] = cameraDistance * farClippingLimit;
        }
        cam->SetClippingRange(newRange);
      }
      this->Renderers->Render();
    }

    quiltFramebuffer->Bind(GL_DRAW_FRAMEBUFFER);

    int destPos[2];
    this->Interface->GetTilePosition(tile, destPos);

    // blit to quilt
    ostate->vtkglViewport(destPos[0], destPos[1], renderSize[0], renderSize[1]);
    ostate->vtkglScissor(destPos[0], destPos[1], renderSize[0], renderSize[1]);
    glBlitFramebuffer(0, 0, renderSize[0], renderSize[1], destPos[0], destPos[1],
      destPos[0] + renderSize[0], destPos[1] + renderSize[1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
  }
  ostate->PopFramebufferBindings();

  // rertore the original size
  this->InStereoRender = false;
  this->Size[0] = origSize[0];
  this->Size[1] = origSize[1];

  // restore the original camera settings
  int count = 0;
  for (this->Renderers->InitTraversal(rsit); (aren = this->Renderers->GetNextRenderer(rsit));
       ++count)
  {
    aren->SetActiveCamera(Cameras[count]);
    Cameras[count]->Delete();
  }

  // ostate->PushReadFramebufferBinding();
  // quiltFramebuffer->Bind(GL_READ_FRAMEBUFFER);
  // glBlitFramebuffer(
  //   0, 0, 4096, 4096, 0, 0, this->Size[0], this->Size[1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
  // ostate->PopReadFramebufferBinding();

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
