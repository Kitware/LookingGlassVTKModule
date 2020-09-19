/*=========================================================================

  Copyright (c) 2020 Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkXLookingGlassRenderWindow
 * @brief   LookingGlass rendering window
 *
 * A drop in replacement on Windows for vtkOpenGLRenderWindow that will
 * render to a LookingGlass display
 */

#ifndef vtkXLookingGlassRenderWindow_h
#define vtkXLookingGlassRenderWindow_h

#include "vtkRenderingLookingGlassModule.h" // For export macro
#include "vtkXOpenGLRenderWindow.h"

class vtkLookingGlassInterface;

class VTKRENDERINGLOOKINGGLASS_EXPORT vtkXLookingGlassRenderWindow : public vtkXOpenGLRenderWindow
{
public:
  static vtkXLookingGlassRenderWindow* New();
  vtkTypeMacro(vtkXLookingGlassRenderWindow, vtkXOpenGLRenderWindow);

  // Get the LookingGlassInterface being used by this window.
  vtkGetObjectMacro(Interface, vtkLookingGlassInterface);

  /**
   * Get the size (width and height) of the rendering window.
   * We override so that durnig the render process we can return
   * a size of the render framebuffer as opposed to the final
   * buffer.
   */
  int* GetSize() VTK_SIZEHINT(2) override;

  /**
   * Free up any graphics resources associated with this window
   * a value of nullptr means the context may already be destroyed
   */
  void ReleaseGraphicsResources(vtkWindow*) override;

protected:
  vtkXLookingGlassRenderWindow();
  ~vtkXLookingGlassRenderWindow() override;

  vtkLookingGlassInterface* Interface;

  void DoStereoRender() override;
  bool InStereoRender;

private:
  vtkXLookingGlassRenderWindow(const vtkXLookingGlassRenderWindow&) = delete;
  void operator=(const vtkXLookingGlassRenderWindow&) = delete;
};

#endif
