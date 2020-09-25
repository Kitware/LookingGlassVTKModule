/*=========================================================================

  Copyright (c) 2020 Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkLookingGlassInterface
 * @brief   Class to interface with LookingGlass hardware.
 *
 * This class contains a collection of methods to fascilitate
 * rendering to a LookingGlass display.
 */

#ifndef vtkLookingGlassInterface_h
#define vtkLookingGlassInterface_h

#include "vtkRenderingLookingGlassModule.h" // For export macro
#include "vtkObject.h"

class vtkCamera;
class vtkOpenGLFramebufferObject;
class vtkOpenGLQuadHelper;
class vtkOpenGLRenderWindow;
class vtkTextureObject;
class vtkWindow;

class VTKRENDERINGLOOKINGGLASS_EXPORT vtkLookingGlassInterface : public vtkObject
{
public:
  static vtkLookingGlassInterface* New();
  vtkTypeMacro(vtkLookingGlassInterface, vtkObject);

  // create an OpenGLRenderWindow suitable for looking glass
  // just a convenience method to handle the OS specific subclasses
  // in a generic manner.
  static vtkOpenGLRenderWindow* CreateLookingGlassRenderWindow();

  // Get the display posiiton for the looking glass device
  vtkGetVector2Macro(DisplayPosition, int);

  // Get the size in pixels of the looking glass device
  vtkGetVector2Macro(DisplaySize, int);

  // get the size of the images that should be rendered to the quilt
  vtkGetVector2Macro(RenderSize, int);

  // get the size of the quilt to be used
  vtkGetVector2Macro(QuiltSize, int);

  // get the number of tiles on the quilt in the X, Y
  vtkGetVector2Macro(QuiltTiles, int);

  // Get the total number of tiles for the quilt
  // Just QuiltTiles[0] * QuiltTiles[1]
  vtkGetMacro(NumberOfTiles, int);

  vtkGetMacro(ViewAngle, double);

  // Return the position in pixels in the quilt for that tile
  void GetTilePosition(int tile, int pos[2]);

  // Must be called first
  void Initialize();

  // Adjust a camera's settings to be correct for the view of the specified
  // tile. The camera passed in should have your centered view. It will be
  // modified to correspond to the view for that provided tile.
  void AdjustCamera(vtkCamera* cam, int tile);

  // Render the Quilt (LightField) to the currently bound Framebuffer
  void DrawLightField(vtkOpenGLRenderWindow* rw);

  // Render the Quilt (LightField) to the currently bound Framebuffer by using a
  // provided texture. This is for the case where you are using two
  // opengl context's and using the texture handle to pass between them
  void DrawLightField(vtkOpenGLRenderWindow* rw, vtkTextureObject* copyTO);

  // Get, and create if needed, framebuffers to be used for rendering and
  // constructing the quilt. These will have sizes based on the LookingGlass
  // settings.
  void GetFramebuffers(vtkOpenGLRenderWindow* rw, vtkOpenGLFramebufferObject*& renderFramebuffer,
    vtkOpenGLFramebufferObject*& quiltFramebuffer);

  /**
   * Release graphics resources and ask components to release their own
   * resources.
   * \pre w_exists: w!=0
   */
  void ReleaseGraphicsResources(vtkWindow* w);

  //@{
  /**
   * Set/Get the quality of the quilt to use. 0 is the lowest setting
   * and 2 is the highest. Higher settings require more texture memory.
   * The default setting is 1.
   */
  vtkSetMacro(QuiltQuality, int);
  vtkGetMacro(QuiltQuality, int);
  //@}

  //@{
  /**
   * Set/Get which LookingGlass device to use. DeviceIndex starts at 0 and
   * increases.
   */
  vtkSetMacro(DeviceIndex, int);
  vtkGetMacro(DeviceIndex, int);
  //@}

  //@{
  /**
   * Set/Get limit for the ratio of the far clipping plane to the focal
   * distance. This is a mechanism to limit parallex and resulting
   * ghosting when using the looking glass display. The typical value
   * should be around 1.2.
   */
  vtkSetMacro(FarClippingLimit, double);
  vtkGetMacro(FarClippingLimit, double);
  //@}

  //@{
  /**
   * Set/Get limit for the ratio of the near clipping plane to the focal
   * distance. This is a mechanism to limit parallex and resulting
   * ghosting when using the looking glass display. The typical value
   * should be around 0.8.
   */
  vtkSetMacro(NearClippingLimit, double);
  vtkGetMacro(NearClippingLimit, double);
  //@}

  // helper method to return a window set to share the opengl lists with
  // the provided window. Such as when you want a desktop window and a
  // looking glass window to mirror it.
  vtkOpenGLRenderWindow* CreateSharedLookingGlassRenderWindow(vtkOpenGLRenderWindow* srcWin);

protected:
  vtkLookingGlassInterface();
  ~vtkLookingGlassInterface() override;

  bool GetLookingGlassInfo();
  void SetupQuiltSettings(int i);

  /**
   * Graphics resources.
   */
  vtkOpenGLFramebufferObject* RenderFramebuffer;
  vtkOpenGLFramebufferObject* QuiltFramebuffer;
  vtkTextureObject* QuiltTexture;
  vtkOpenGLQuadHelper* FinalBlend;

  // with multiple LookingGlass which one to use. Defaults to the first.
  int DeviceIndex;

  double FarClippingLimit;
  double NearClippingLimit;

  // see the respective Get methods for descriptions
  int DisplayPosition[2];
  int DisplaySize[2];
  double ViewAngle;
  int QuiltSize[2];
  int QuiltTiles[2];
  int RenderSize[2];
  int NumberOfTiles;

  int QuiltQuality;

  // has the interface been initialized
  bool Initialized;

  // are we connected to a looking glass device
  bool Connected;

  void DrawLightFieldInternal(vtkOpenGLRenderWindow* renWin, vtkTextureObject* tex);

private:
  vtkLookingGlassInterface(const vtkLookingGlassInterface&) = delete;
  void operator=(const vtkLookingGlassInterface&) = delete;
};

#endif
