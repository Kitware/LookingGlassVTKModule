/*=========================================================================

  Copyright (c) 2020 Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkWin32LookingGlassRenderWindow
 * @brief   LookingGlass rendering window
 *
 * A drop in replacement on Windows for vtkOpenGLRenderWindow that will
 * render to a LookingGlass display
 */

#ifndef vtkWin32LookingGlassRenderWindow_h
#define vtkWin32LookingGlassRenderWindow_h

#include "vtkRenderingLookingGlassModule.h" // For export macro
#include "vtkWin32OpenGLRenderWindow.h"

class vtkLookingGlassInterface;

class VTKRENDERINGLOOKINGGLASS_EXPORT vtkWin32LookingGlassRenderWindow : public vtkWin32OpenGLRenderWindow
{
public:
  static vtkWin32LookingGlassRenderWindow* New();
  vtkTypeMacro(vtkWin32LookingGlassRenderWindow, vtkWin32OpenGLRenderWindow);

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
  vtkWin32LookingGlassRenderWindow();
  ~vtkWin32LookingGlassRenderWindow() override;

  vtkLookingGlassInterface* Interface;

  void DoStereoRender() override;
  bool InStereoRender;

private:
  vtkWin32LookingGlassRenderWindow(const vtkWin32LookingGlassRenderWindow&) = delete;
  void operator=(const vtkWin32LookingGlassRenderWindow&) = delete;
};

#endif
