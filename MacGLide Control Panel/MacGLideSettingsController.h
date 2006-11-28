/* MacGLideSettingsController */

#import <Cocoa/Cocoa.h>

#include "GlideSettings.h"

@interface MacGLideSettingsController : NSObject
{
	IBOutlet NSButton *useUseApplicationSpecificSettings;
	IBOutlet NSPopUpButton *settingsName;

	IBOutlet NSPopUpButton *resolution;
	IBOutlet NSPopUpButton *refreshRate;
	IBOutlet NSSlider *gammaBias;
	IBOutlet NSSlider *fullSceneAnitaliasingLevel;

	IBOutlet NSSlider *anisotropicFilteringLevel;
	IBOutlet NSButton *mipmapping;
	IBOutlet NSButton *autoEnableGameSpecificSettings;

	IBOutlet NSPopUpButton *boardType;
	IBOutlet NSPopUpButton *framebufferMemory;
	IBOutlet NSPopUpButton *textureMemory;
	IBOutlet NSPopUpButton *textureUnits;
	IBOutlet NSButton *noSplash;
	IBOutlet NSButton *showShamelessPlug;
}

- (void)init;
- (void)init:(const char*)fileName;
- (void) updateButton:(id)source boolValue:(bool) value;
- (void) updateButton:(id)source ulongValue:(unsigned long) value;
- (void) updateButton:(id)source floatValue:(GLfloat) value;
- (void) updateSetting:(id)source boolSetting:(bool&) setting;
- (void) updateSetting:(id)source ulongSetting:(unsigned long&) setting;
- (void) updateSetting:(id)source floatSetting:(GLfloat&) setting;
- (GlideSettings::IOErr)saveSettings;

// Settings selection
- (IBAction)setUseApplicationSpecificSettings:(id)sender;
- (IBAction)setSettingsName:(id)sender;

// Appearence / Monitor
- (IBAction)setResolution:(id)sender;
- (IBAction)setRefreshRate:(id)sender;
- (IBAction)setGammaBias:(id)sender;
- (IBAction)setFullSceneAnitaliasingLevel:(id)sender;

// OpenGL
- (IBAction)setMipmapping:(id)sender;
- (IBAction)setAnisotropicFilteringLevel:(id)sender;
// OpenGL auto adjust features
- (IBAction)setAutoEnableGameSpecificSettings:(id)sender;

// 3Dfx
- (IBAction)setBoardType:(id)sender;
- (IBAction)setFramebufferMemory:(id)sender;
- (IBAction)setTextureMemory:(id)sender;
- (IBAction)setTextureUnits:(id)sender;
- (IBAction)setShowShamelessPlug:(id)sender;
- (IBAction)setNoSplash:(id)sender;


@end
