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

#include "vtkDeprecation.h"
#include "vtkObject.h"
#include "vtkRenderingLookingGlassModule.h" // For export macro
#include <map>
#include <vector>

#include <functional>

class vtkCamera;
class vtkGenericMovieWriter;
class vtkImageData;
class vtkOpenGLFramebufferObject;
class vtkOpenGLQuadHelper;
class vtkOpenGLRenderWindow;
class vtkRendererCollection;
class vtkTextureObject;
class vtkWindow;

class VTKRENDERINGLOOKINGGLASS_EXPORT vtkLookingGlassInterface : public vtkObject
{
public:
  static vtkLookingGlassInterface* New();
  vtkTypeMacro(vtkLookingGlassInterface, vtkObject);

  // create an OpenGLRenderWindow suitable for looking glass
  // just a convenience method to handle the OS specific subclasses
  // in a generic manner. The deviceIndex argument specifies the
  // index of the LookingGlass device on which the window should be placed
  // (presently supported only on macOS).
  static vtkOpenGLRenderWindow* CreateLookingGlassRenderWindow(int deviceIndex = 0);

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
  VTK_DEPRECATED_IN_9_2_0("Quality is now based on device type.")
  vtkSetMacro(QuiltQuality, int);
  VTK_DEPRECATED_IN_9_2_0("Quality is now based on device type.")
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
   * Set/Get which LookingGlass device type to target. This allows a quilt to be
   * generated for a device that is not connected in the future.
   */
  vtkSetMacro(DeviceType, std::string);
  vtkGetMacro(DeviceType, std::string);
  //@}

  //@{
  /**
   * Turn on/off use of near and far clipping limits.
   */
  vtkSetMacro(UseClippingLimits, bool);
  vtkGetMacro(UseClippingLimits, bool);
  vtkBooleanMacro(UseClippingLimits, bool);
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

  // Render the quilt using the provided render window.
  // The `renderers` argument defaults to all renderers on the render window,
  // but can be provided to use only a subset of the renderers.
  // The `renderFunc` is an optional function to use for rendering instead of
  // the `Render()` function on the renderers. This is important when custom
  // rendering is required, such as in a vtkRenderPass, like the
  // vtkLookingGlassPass.
  // Note that you may need to modify the size of the render window to be that
  // of the vtkLookingGlassInterface::GetRenderSize() before calling this
  // function.
  void RenderQuilt(vtkOpenGLRenderWindow* rw, vtkRendererCollection* renderers = nullptr,
    std::function<void(void)>* renderFunc = nullptr);

  /**
   * Save the quilt currently displayed in the render window as a PNG file.
   * The quilt can be loaded into HoloPlay Studio to run the Looking Glass
   * device in stand-alone mode.
   */
  void SaveQuilt(const char* fileName);

  VTK_DEPRECATED_IN_9_2_0("The render window argument is no longer used and should be removed")
  void SaveQuilt(vtkOpenGLRenderWindow* rw, const char* fileName) { this->SaveQuilt(fileName); }

  /**
   * Get the extension of the movie file that will be written if the
   * user records a video quilt.
   * This will be "mp4", "avi", or "ogg", depending on what is available
   * from the VTK build.
   * MP4 will be used if it is available, since it is the only one that
   * HoloPlay Studio can read in.
   * If one of the other formats are used, the user will have to use external
   * software to convert it to a format that HoloPlay Studio can read.
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
   * Check if the quilt is currently being recorded.
   */
  bool IsRecordingQuilt() const { return this->IsRecording; }

  /**
   * Start recording the quilt from the render window display and save it as
   * a movie file. WriteQuiltMovieFrame() should be called each time a
   * frame should be written. This happens automatically each time `RenderQuilt()`
   * is called, which results in a frame being written for every render. But
   * WriteQuiltMovieFrame() may be called additional times to write extra frames.
   *
   * The quilt can be loaded into HoloPlay Studio to run the Looking Glass
   * device in stand-alone mode, although the user may need to convert the
   * video file into a format that HoloPlay Studio can read (such as MP4).
   */
  void StartRecordingQuilt(const char* fileName);

  VTK_DEPRECATED_IN_9_2_0("The render window argument is no longer used and should be removed")
  void StartRecordingQuilt(vtkOpenGLRenderWindow* rw, const char* fileName)
  {
    this->StartRecordingQuilt(fileName);
  }

  /**
   * Write a frame to the movie file. StartRecordingQuilt() must have been
   * called previously. This function is called automatically in
   * `RenderQuilt()` while a movie is being recorded, but it can be called
   * additional times to write extra frames.
   */
  void WriteQuiltMovieFrame();

  /**
   * Stop recording the quilt to finish the movie file.
   *
   * The quilt can be loaded into HoloPlay Studio to run the Looking Glass
   * device in stand-alone mode, although the user may need to convert the
   * video file into a format that HoloPlay Studio can read.
   */
  void StopRecordingQuilt();

  using DeviceTypes = std::vector<std::pair<std::string, std::string>>;

  /**
   * Returns vector of pairs with the available device types.
   */
  static DeviceTypes GetDevices();

protected:
  /**
   * struct to hold device specfic settings.
   */
  struct DeviceSettings
  {
    DeviceSettings(const std::string& name, int quiltWidth, int quiltHeight,
                   int quiltTilesColumns, int quiltTilesRows);
    DeviceSettings() = default;
    int QuiltSize[2];
    int QuiltTiles[2];
    std::string Name;
  };

  vtkLookingGlassInterface();
  ~vtkLookingGlassInterface() override;

  bool GetLookingGlassInfo();

  VTK_DEPRECATED_IN_9_2_0("Quality is now based on device type.")
  void SetupQuiltSettings(int i);

  /**
   * Setup quilt settings based on device type.
   */
  void SetupQuiltSettings(const std::string& deviceType);

  /**
   * Setup quilt settings based on device type.
   */
  void SetupQuiltSettings(const DeviceSettings& deviceSettings);

  /**
   * Get the device setting by device type.
   */
  static std::map<std::string, DeviceSettings> GetSettingsByDevice();

  /**
   * Get the device setting for a give device.
   */
  static DeviceSettings GetSettingsForDevice(const std::string deviceType);

  /**
   * Graphics resources.
   */
  vtkOpenGLFramebufferObject* RenderFramebuffer;
  vtkOpenGLFramebufferObject* QuiltFramebuffer;
  vtkTextureObject* QuiltTexture;
  vtkOpenGLQuadHelper* FinalBlend;
  vtkOpenGLQuadHelper* QuiltBlend;

  // with multiple LookingGlass which one to use. Defaults to the first.
  int DeviceIndex;

  // the device type, used to setup quilt settings, default to large
  std::string DeviceType;

  bool UseClippingLimits;
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

  // Are we recording a movie
  bool IsRecording;

  // For recording a movie
  vtkImageData* MovieImageBuffer;
  vtkImageData* MovieImageData;
  vtkGenericMovieWriter* MovieWriter;

  void DrawLightFieldInternal(vtkOpenGLRenderWindow* renWin, vtkTextureObject* tex);

private:
  vtkLookingGlassInterface(const vtkLookingGlassInterface&) = delete;
  void operator=(const vtkLookingGlassInterface&) = delete;
};

#endif
