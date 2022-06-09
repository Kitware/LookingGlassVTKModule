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

  /**
   * Set index of the Looking Glass device on which this window should appear.
   */
  void SetLGDeviceIndex(int index);

  /**
   * Save a quilt to a PNG file
   */
  void SaveQuilt(const char* fileName);

  /**
   * Start recording a quilt
   */
  void StartRecordingQuilt(const char* fileName);

  /**
   * Stop recording a quilt
   */
  void StopRecordingQuilt();

  /**
   * Get the movie extension that should be used for quilt movies
   */
  static const char* MovieFileExtension();

  /**
   * Get the quilt file suffix as a string. The suffix encodes the number of
   * tiles in the width and the height. For example, if the quilt file name
   * is "quilt_qs5x9.png", the suffix is "_qs5x9", and it means that the quilt
   * is 5 tiles wide and 9 tiles high.
   */
  std::string QuiltFileSuffix() const;

  /**
   * Turn on/off use of near and far clipping limits.
   */
  void SetUseClippingLimits(bool b);

  /**
   * Turn on/off use of near and far clipping limits.
   */
  bool GetUseClippingLimits() const;
  vtkBooleanMacro(UseClippingLimits, bool);

  /**
   * Set/Get limit for the ratio of the far clipping plane to the focal
   * distance. This is a mechanism to limit parallex and resulting
   * ghosting when using the looking glass display. The typical value
   * should be around 1.2.
   */
  void SetFarClippingLimit(double d);

  /**
   * Set/Get limit for the ratio of the far clipping plane to the focal
   * distance. This is a mechanism to limit parallex and resulting
   * ghosting when using the looking glass display. The typical value
   * should be around 1.2.
   */
  double GetFarClippingLimit() const;

  /**
   * Set/Get limit for the ratio of the near clipping plane to the focal
   * distance. This is a mechanism to limit parallex and resulting
   * ghosting when using the looking glass display. The typical value
   * should be around 0.8.
   */
  void SetNearClippingLimit(double d);

  /**
   * Set/Get limit for the ratio of the near clipping plane to the focal
   * distance. This is a mechanism to limit parallex and resulting
   * ghosting when using the looking glass display. The typical value
   * should be around 0.8.
   */
  double GetNearClippingLimit() const;

  /**
   * Check if a movie quilt is currently being recorded.
   */
  bool IsRecordingQuilt() const;

  /**
   * Set/Get which LookingGlass device to use. DeviceIndex starts at 0 and
   * increases.
   */
  void SetDeviceIndex(int i);

  /**
   * Set/Get which LookingGlass device to use. DeviceIndex starts at 0 and
   * increases.
   */
  int GetDeviceIndex() const;

  /**
   * Set/Get which LookingGlass device type to target. This allows a quilt to be
   * generated for a device that is not connected in the future.
   */
  void SetDeviceType(const std::string &t);

  /**
   * Set/Get which LookingGlass device type to target. This allows a quilt to be
   * generated for a device that is not connected in the future.
   */
  std::string GetDeviceType() const;

  /**
   * Returns a vector of available device types.
   */
  static std::vector<std::string> GetDeviceTypes();

protected:
  vtkXLookingGlassRenderWindow();
  ~vtkXLookingGlassRenderWindow() override;

  vtkLookingGlassInterface* Interface = nullptr;

  void InitializeInterface();

  void DoStereoRender() override;
  bool InStereoRender;

private:
  vtkXLookingGlassRenderWindow(const vtkXLookingGlassRenderWindow&) = delete;
  void operator=(const vtkXLookingGlassRenderWindow&) = delete;
};

#endif
