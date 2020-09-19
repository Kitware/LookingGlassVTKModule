/*=========================================================================

  Copyright (c) 2020 Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkLookingGlassPass
 * @brief   Render qulited image for LookingGlass
 *
 * @sa
 * vtkRenderPass
 */

#ifndef vtkLookingGlassPass_h
#define vtkLookingGlassPass_h

#include "vtkRenderingLookingGlassModule.h" // For export macro
#include "vtkOpenGLRenderPass.h"

class vtkLookingGlassInterface;
class vtkOpenGLFramebufferObject;
class vtkOpenGLHelper;
class vtkOpenGLRenderWindow;
class vtkTextureObject;

class VTKRENDERINGLOOKINGGLASS_EXPORT vtkLookingGlassPass : public vtkOpenGLRenderPass
{
public:
  static vtkLookingGlassPass* New();
  vtkTypeMacro(vtkLookingGlassPass, vtkOpenGLRenderPass);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Perform rendering according to a render state \p s.
   * \pre s_exists: s!=0
   */
  void Render(const vtkRenderState* s) override;

  /**
   * Release graphics resources and ask components to release their own
   * resources.
   * \pre w_exists: w!=0
   */
  void ReleaseGraphicsResources(vtkWindow* w) override;

  //@{
  /**
   * Delegate for rendering the image to be processed.
   * If it is NULL, nothing will be rendered and a warning will be emitted.
   * It is usually set to a vtkCameraPass or to a post-processing pass.
   * Initial value is a NULL pointer.
   */
  vtkGetObjectMacro(DelegatePass, vtkRenderPass);
  virtual void SetDelegatePass(vtkRenderPass* delegatePass);
  //@}

  // Get the LookingGlassInterface being used by this pass.
  // This is useful to set the position and size of the
  // render window.
  vtkGetObjectMacro(Interface, vtkLookingGlassInterface);

protected:
  /**
   * Default constructor. DelegatePass is set to NULL.
   */
  vtkLookingGlassPass();

  /**
   * Destructor.
   */
  ~vtkLookingGlassPass() override;

  vtkRenderPass* DelegatePass;

  vtkLookingGlassInterface* Interface;

private:
  vtkLookingGlassPass(const vtkLookingGlassPass&) = delete;
  void operator=(const vtkLookingGlassPass&) = delete;
};

#endif
