#import "MacGLidePanel.h"

#include <sdk2_glide.h>
#include "GlideSettings_iostream.h"

GlideSettingsIOStream UserConfig;

void GlideMsg(const char *message, ...)
{
	// Do nothing here as we don't need the logging output
}

@implementation MacGLidePanel

- (IBAction)toggleTextureSmoothing:(id)sender
{
	assert(false);
}

- (IBAction)toggleMipmapping:(id)sender
{
	UserConfig.EnableMipMaps = true;
}

- (IBAction)toogleUseGameSpecificOptimisations:(id)sender
{
	assert(false);
}

@end
