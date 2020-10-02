/*=========================================================================

  Copyright (c) 2020 Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCocoaLookingGlassRenderWindow.h"

#define className vtkCocoaLookingGlassRenderWindow
#include "vtkLookingGlassRenderWindowImpl.h"

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <IOKit/graphics/IOGraphicsLib.h>
#include "HoloPlayCore.h"

//------------------------------------------------------------------------------
void vtkCocoaLookingGlassRenderWindow::Initialize()
{
  // Limited to device 0 fow now
  char buf[1000];
  int deviceIndex = this->Interface->GetDeviceIndex();
  hpc_GetDeviceHDMIName(deviceIndex, buf, 1000);
  std::string deviceName(buf);

  // Default to display 1, assuming the LG display is the only auxilliary display
  int displayId = 1;

  // Explicitly look for the LG display
  int screenIndex = 0;
  NSArray *screens = [NSScreen screens];
  for (NSScreen *screen in screens)
  {
    int currentDisplayID = [[[screen deviceDescription] valueForKey:@"NSScreenNumber"] intValue];
    NSDictionary *deviceInfo =
      (NSDictionary *)CFBridgingRelease(IODisplayCreateInfoDictionary(CGDisplayIOServicePort(currentDisplayID),
                                                                      kIODisplayOnlyPreferredName));
    NSDictionary *localizedNames = [deviceInfo objectForKey:[NSString stringWithUTF8String:kDisplayProductName]];

    NSString *screenName = nil;
    if ([localizedNames count] > 0) {
      screenName = [localizedNames objectForKey:[[localizedNames allKeys] objectAtIndex:0]];
      std::string screenNameString = std::string([screenName UTF8String]);
      if (screenNameString == deviceName)
      {
        displayId = screenIndex;
        break;
      }
    }
    ++screenIndex;
  }
  this->SetDisplayId(&displayId);

  this->Superclass::Initialize();
}
