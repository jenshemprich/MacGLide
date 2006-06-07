/* MacGLidePanel */

#import <Cocoa/Cocoa.h>

@interface MacGLidePanel : NSPanel
{
}
- (IBAction)toggleTextureSmoothing:(id)sender;
- (IBAction)toggleMipmapping:(id)sender;
- (IBAction)toogleUseGameSpecificOptimisations:(id)sender;
@end
