/*=========================================================================

  Copyright (c) 2020 Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkLookingGlassPass.h"
#include "vtkObjectFactory.h"
#include <cassert>

#include "vtkCamera.h"
#include "vtkLookingGlassInterface.h"
#include "vtkOpenGLError.h"
#include "vtkOpenGLFramebufferObject.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLShaderCache.h"
#include "vtkOpenGLState.h"
#include "vtkOpenGLVertexArrayObject.h"
#include "vtkRenderState.h"
#include "vtkRenderer.h"
#include "vtkShaderProgram.h"
#include "vtkTextureObject.h"

#include "vtkOpenGLHelper.h"

vtkStandardNewMacro(vtkLookingGlassPass);

vtkCxxSetObjectMacro(vtkLookingGlassPass, DelegatePass, vtkRenderPass);

//------------------------------------------------------------------------------
vtkLookingGlassPass::vtkLookingGlassPass()
  : DelegatePass(nullptr)
{
  this->Interface = vtkLookingGlassInterface::New();
}

//------------------------------------------------------------------------------
vtkLookingGlassPass::~vtkLookingGlassPass()
{
  this->Interface->Delete();
  if (this->DelegatePass != nullptr)
  {
    this->DelegatePass->Delete();
  }
}

void vtkLookingGlassPass::ReleaseGraphicsResources(vtkWindow* w)
{
  assert("pre: w_exists" && w != nullptr);

  this->Interface->ReleaseGraphicsResources(w);
  this->Superclass::ReleaseGraphicsResources(w);
}

//------------------------------------------------------------------------------
void vtkLookingGlassPass::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
// Description:
// Perform rendering according to a render state \p s.
// \pre s_exists: s!=0
void vtkLookingGlassPass::Render(const vtkRenderState* s)
{
  assert("pre: s_exists" && s != nullptr);

  vtkOpenGLClearErrorMacro();

  this->NumberOfRenderedProps = 0;

  vtkRenderer* r = s->GetRenderer();
  vtkRenderState s2(r);
  s2.SetPropArrayAndCount(s->GetPropArray(), s->GetPropArrayCount());

  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(r->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();

  if (this->DelegatePass == nullptr)
  {
    vtkWarningMacro(<< " no delegate.");
    return;
  }

  this->Interface->Initialize();
  vtkOpenGLFramebufferObject* renderFramebuffer;
  vtkOpenGLFramebufferObject* quiltFramebuffer;
  this->Interface->GetFramebuffers(renWin, renderFramebuffer, quiltFramebuffer);

  vtkCamera* savedCamera = r->GetActiveCamera();
  savedCamera->Register(this);
  vtkCamera* newCamera = vtkCamera::New();
  r->SetActiveCamera(newCamera);

  ostate->PushFramebufferBindings();
  renderFramebuffer->Bind(GL_READ_FRAMEBUFFER);
  s2.SetFrameBuffer(renderFramebuffer);

  int renderSize[2];
  this->Interface->GetRenderSize(renderSize);

  int tcount = this->Interface->GetNumberOfTiles();

  for (int tile = 0; tile < tcount; ++tile)
  {
    // adjust camera
    newCamera->DeepCopy(savedCamera);
    this->Interface->AdjustCamera(newCamera, tile);

    // render
    renderFramebuffer->Bind(GL_DRAW_FRAMEBUFFER);
    ostate->vtkglViewport(0, 0, renderSize[0], renderSize[1]);
    ostate->vtkglScissor(0, 0, renderSize[0], renderSize[1]);
    this->DelegatePass->Render(&s2);

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
  newCamera->Delete();
  r->SetActiveCamera(savedCamera);
  savedCamera->UnRegister(this);

  this->Interface->DrawLightField(renWin);

  // ostate->PushReadFramebufferBinding();
  // quiltFramebuffer->Bind(GL_READ_FRAMEBUFFER);
  // glBlitFramebuffer(
  //   0, 0, qsize[0], qsize[1], 0, 0, dsize[0], dsize[1], GL_COLOR_BUFFER_BIT, GL_LINEAR);
  // ostate->PopReadFramebufferBinding();

  vtkOpenGLCheckErrorMacro("failed after Render");
}
