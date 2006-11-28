#import "MacGLideSettingsController.h"

#include <sdk2_glide.h>
#include "GlideSettings_FSp.h"

GlideSettingsFSp userConfig;

void GlideMsg(const char* message, ...)
{
	// Don't log anything
}
OSErr loadUserConfig(const char* fileName)
{
	// Select default setting file
	OSErr err = userConfig.init(fileName);
	if (err == noErr)
	{
		err = userConfig.load();
	}
	return err;
}

@implementation MacGLideSettingsController

- (void)init
{
	[self init:"Defaults"];
	return;
}

- (void)init:(const char*) fileName
{
	OSErr err = loadUserConfig(fileName);
	if (err == noErr)
	{
		// init controls
		[self updateButton:useUseApplicationSpecificSettings boolValue:userConfig.UseApplicationSpecificSettings];
		[self updateButton:resolution ulongValue:userConfig.Resolution];
		[self updateButton:refreshRate ulongValue:userConfig.MonitorRefreshRate];
		[self updateButton:gammaBias floatValue:userConfig.GammaBias];
		[self updateButton:fullSceneAnitaliasingLevel ulongValue:userConfig.FullSceneAntiAliasing];
		[self updateButton:anisotropicFilteringLevel ulongValue:userConfig.AnisotropylLevel];
		[self updateButton:mipmapping ulongValue:userConfig.Mipmapping];
		[self updateButton:autoEnableGameSpecificSettings boolValue:userConfig.AutoEnableGameSpecificSettings];
		[self updateButton:boardType ulongValue:userConfig.BoardType];
		[self updateButton:framebufferMemory ulongValue:userConfig.FrameBufferMemorySize];
		[self updateButton:textureMemory ulongValue:userConfig.TextureMemorySize];
		[self updateButton:textureUnits ulongValue:userConfig.GlideTextureUnits];
		[self updateButton:noSplash boolValue:userConfig.NoSplash];
		[self updateButton:showShamelessPlug boolValue:userConfig.ShamelessPlug];
	}
}

- (void) updateButton:(id) source boolValue:(bool) value
{
	[source setState:(value ? NSOnState : NSOffState)];
}

- (void) updateButton:(id) source ulongValue:(unsigned long) value
{
	[source setIntValue: value];
}

- (void) updateButton:(id) source floatValue:(GLfloat) value
{
	[source setFloatValue: value];
}

- (void) updateSetting:(id) source boolSetting:(bool&)setting;
{
	setting = ([source state] == NSOnState) ? true : false;
	[self saveSettings];
}

- (void) updateSetting:(id) source ulongSetting:(unsigned long&)setting;
{
	setting = [source intValue];
	[self saveSettings];
}

- (void) updateSetting:(id) source floatSetting:(GLfloat&)setting;
{
	setting = [source floatValue];
	[self saveSettings];
}

- (GlideSettings::IOErr) saveSettings
{
	GlideSettings::IOErr err = userConfig.save();
	return err;
}

// Settings selection
- (IBAction)setUseApplicationSpecificSettings:(id)sender
{
	[self updateSetting:sender boolSetting:userConfig.UseApplicationSpecificSettings];
}

- (IBAction)setSettingsName:(id)sender
{
	[self saveSettings];
	NSString* s = [sender titleOfSelectedItem]; 
	[self init:[s UTF8String]];
}

// Appearence / Monitor
- (IBAction)setResolution:(id)sender
{
	[self updateSetting:sender ulongSetting:userConfig.Resolution];
}

- (IBAction)setRefreshRate:(id)sender
{
	[self updateSetting:sender ulongSetting:userConfig.MonitorRefreshRate];
}

- (IBAction)setGammaBias:(id)sender
{
	[self updateSetting:sender floatSetting:userConfig.GammaBias];
}

- (IBAction)setFullSceneAnitaliasingLevel:(id)sender
{
	[self updateSetting:sender ulongSetting:userConfig.FullSceneAntiAliasing];
}


// OpenGL
- (IBAction)setMipmapping:(id)sender
{
	[self updateSetting:sender boolSetting:userConfig.Mipmapping];
}

- (IBAction)setAnisotropicFilteringLevel:(id)sender
{
	[self updateSetting:sender ulongSetting:userConfig.AnisotropylLevel];
}


- (IBAction)setAutoEnableGameSpecificSettings:(id)sender
{
	[self updateSetting:sender boolSetting:userConfig.AutoEnableGameSpecificSettings];
}


// 3Dfx
- (IBAction)setBoardType:(id)sender
{
	unsigned long bt = userConfig.BoardType;
	[self updateSetting:sender ulongSetting:bt];
	userConfig.BoardType = static_cast<OpenGLideBoardType>(bt);
}

- (IBAction)setFramebufferMemory:(id)sender
{
	[self updateSetting:sender ulongSetting:userConfig.FrameBufferMemorySize];
}

- (IBAction)setTextureMemory:(id)sender
{
	[self updateSetting:sender ulongSetting:userConfig.TextureMemorySize];
}

- (IBAction)setTextureUnits:(id)sender
{
	[self updateSetting:sender ulongSetting:userConfig.GlideTextureUnits];
}

- (IBAction)setShowShamelessPlug:(id)sender
{
	[self updateSetting:sender boolSetting:userConfig.ShamelessPlug];
}

- (IBAction)setNoSplash:(id)sender
{
	[self updateSetting:sender boolSetting:userConfig.NoSplash];
}


@end
