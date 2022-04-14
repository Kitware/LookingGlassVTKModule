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
#include "vtkRenderingLookingGlassModule.h" // For export macro
#include <map>
#include <vector>

class vtkCamera;
class vtkGenericMovieWriter;
class vtkOpenGLFramebufferObject;
class vtkOpenGLQuadHelper;
class vtkOpenGLRenderWindow;
class vtkTextureObject;
class vtkWindow;
class vtkWindowToImageFilter;

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
  vtkSetMacro(QuiltQuality, int);
  vtkGetMacro(QuiltQuality, int);
  //@}

  //@{
  /**
   * Set/Get the magnification used for quilt images/movies. Higher
   * magnification will produce higher resolution files.
   * The default setting is 2.
   */
  vtkSetClampMacro(QuiltExportMagnification, int, 1, VTK_INT_MAX);
  vtkGetMacro(QuiltExportMagnification, int);
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

  /**
   * Save the quilt currently displayed in the render window as a PNG file.
   * The quilt can be loaded into HoloPlay Studio to run the Looking Glass
   * device in stand-alone mode.
   *
   * This will re-render the render window twice: once to render the quilt
   * image that will be written to the PNG file, and once to
   * restore the render window back to its original state.
   */
  void SaveQuilt(vtkOpenGLRenderWindow* rw, const char* fileName);

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
   * Check if the quilt is currently being recorded.
   */
  bool IsRecordingQuilt() const { return this->IsRecording; }

  /**
   * Start recording the quilt from the render window display and save it as
   * a movie file. WriteQuiltMovieFrame() should be called each time a
   * frame should be written. This will preferably happen immediately
   * before rendering to the device.
   *
   * The quilt can be loaded into HoloPlay Studio to run the Looking Glass
   * device in stand-alone mode, although the user may need to convert the
   * video file into a format that HoloPlay Studio can read.
   */
  void StartRecordingQuilt(vtkOpenGLRenderWindow* rw, const char* fileName);

  /**
   * Write a frame to the movie file. StartRecordingQuilt() must have been
   * called previously. This will re-render the render window with
   * the quilt displayed on it for saving the frame. Thus, it is preferable
   * to render immediately after writing a movie frame, so the quilt
   * will not be displayed on the device.
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

  // The magnification factor for exporting quilt images/movies.
  // Higher magnification produces higher resolution files.
  int QuiltExportMagnification;

  // has the interface been initialized
  bool Initialized;

  // are we connected to a looking glass device
  bool Connected;

  // Are we saving a quilt
  bool SavingQuilt;

  // Are we recording a movie
  bool IsRecording;

  // For recording a movie
  vtkWindowToImageFilter* MovieWindowToImageFilter;
  vtkGenericMovieWriter* MovieWriter;

  void DrawLightFieldInternal(vtkOpenGLRenderWindow* renWin, vtkTextureObject* tex);

private:
  vtkLookingGlassInterface(const vtkLookingGlassInterface&) = delete;
  void operator=(const vtkLookingGlassInterface&) = delete;
};

#endif
