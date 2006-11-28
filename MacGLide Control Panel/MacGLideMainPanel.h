/* MacGLideMainPanel */

#import <Cocoa/Cocoa.h>
#import "MacGLideSettingsController.h"

@interface MacGLideMainPanel : NSWindow
{
	IBOutlet MacGLideSettingsController* controller;
}
// - (id)initWithContentRect:(NSRect)contentRect styleMask:(unsigned int)styleMask backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation;
@end
