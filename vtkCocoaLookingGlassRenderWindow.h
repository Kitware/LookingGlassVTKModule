/*=========================================================================

  Copyright (c) 2020 Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkCocoaLookingGlassRenderWindow
 * @brief   LookingGlass rendering window
 *
 * A drop in replacement on Windows for vtkOpenGLRenderWindow that will
 * render to a LookingGlass display
 */

#ifndef vtkCocoaLookingGlassRenderWindow_h
#define vtkCocoaLookingGlassRenderWindow_h

#include "vtkCocoaRenderWindow.h"
#include "vtkRenderingLookingGlassModule.h" // For export macro

class vtkLookingGlassInterface;

class VTKRENDERINGLOOKINGGLASS_EXPORT vtkCocoaLookingGlassRenderWindow : public vtkCocoaRenderWindow
{
public:
  static vtkCocoaLookingGlassRenderWindow* New();
  vtkTypeMacro(vtkCocoaLookingGlassRenderWindow, vtkCocoaRenderWindow);

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
  vtkCocoaLookingGlassRenderWindow();
  ~vtkCocoaLookingGlassRenderWindow() override;

  vtkLookingGlassInterface* Interface;

  void DoStereoRender() override;
  bool InStereoRender;

private:
  vtkCocoaLookingGlassRenderWindow(const vtkCocoaLookingGlassRenderWindow&) = delete;
  void operator=(const vtkCocoaLookingGlassRenderWindow&) = delete;
};

#endif
