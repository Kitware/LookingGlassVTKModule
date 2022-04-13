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

#include "vtkLookingGlassInterface.h"
#include "vtkOpenGLError.h"
#include "vtkOpenGLFramebufferObject.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkRenderState.h"
#include "vtkRenderer.h"

vtkStandardNewMacro(vtkLookingGlassPass);

vtkCxxSetObjectMacro(vtkLookingGlassPass, DelegatePass, vtkRenderPass);

//------------------------------------------------------------------------------
vtkLookingGlassPass::vtkLookingGlassPass()
  : DelegatePass(nullptr)
{
  this->Interface = vtkLookingGlassInterface::New();
  this->Interface->Initialize();
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

  if (this->DelegatePass == nullptr)
  {
    vtkWarningMacro(<< " no delegate.");
    return;
  }

  this->Interface->Initialize();
  vtkOpenGLFramebufferObject* renderFramebuffer;
  vtkOpenGLFramebufferObject* quiltFramebuffer;
  this->Interface->GetFramebuffers(renWin, renderFramebuffer, quiltFramebuffer);

  s2.SetFrameBuffer(renderFramebuffer);
  std::function<void(void)> renderFunc = [this, &s2]() { this->DelegatePass->Render(&s2); };

  this->Interface->RenderQuilt(renWin, &renderFunc);
  this->Interface->DrawLightField(renWin);

  vtkOpenGLCheckErrorMacro("failed after Render");
}
