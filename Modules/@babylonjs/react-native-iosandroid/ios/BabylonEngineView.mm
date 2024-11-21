#import "BabylonEngineView.h"
#include "BabylonNativeInterop.h"

@implementation EngineView {
    MTKView* xrView;
}

- (instancetype)init {
    if (self = [super initWithFrame:CGRectZero device:MTLCreateSystemDefaultDevice()]) {
        super.translatesAutoresizingMaskIntoConstraints = false;
        super.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        super.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
    }
    return self;
}

- (void)setIsTransparentFlag:(NSNumber*)isTransparentFlag {
    BOOL isTransparent = [isTransparentFlag intValue] == 1;
    if(isTransparent){
        [self setOpaque:NO];
    } else {
        [self setOpaque:YES];
    }
    self.isTransparent = isTransparent;
}

- (void)setMSAA:(NSNumber*)value {
    [BabylonNativeInterop updateMSAA:value];
}

- (void)setBounds:(CGRect)bounds {
    [super setBounds:bounds];
    [BabylonNativeInterop updateView:self];
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [BabylonNativeInterop reportTouchEvent:self touches:touches event:event];
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [BabylonNativeInterop reportTouchEvent:self touches:touches event:event];
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [BabylonNativeInterop reportTouchEvent:self touches:touches event:event];
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [BabylonNativeInterop reportTouchEvent:self touches:touches event:event];
}

- (void)drawRect:(CGRect)rect {
    if ([BabylonNativeInterop isXRActive]) {
        if (!xrView) {
            xrView = [[MTKView alloc] initWithFrame:self.bounds device:self.device];
            xrView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
            xrView.userInteractionEnabled = false;
            [self addSubview:xrView];
            [BabylonNativeInterop updateXRView:xrView];
        }
    } else if (xrView) {
        [BabylonNativeInterop updateXRView:nil];
        [xrView removeFromSuperview];
        xrView = nil;
    }

    [BabylonNativeInterop renderView];
}

-(void)dealloc {
    [BabylonNativeInterop updateXRView:nil];
}

- (void)takeSnapshot {
    // We must take the screenshot on the main thread otherwise we might fail to get a valid handle on the view's image.
    dispatch_async(dispatch_get_main_queue(), ^{
        // Start the graphics context.
        UIGraphicsBeginImageContextWithOptions(self.bounds.size, YES /* opaque */, 0.0f);
        
        // Draw the current state of the view into the graphics context.
        [self drawViewHierarchyInRect:self.bounds afterScreenUpdates:NO];
        
        // Grab the image from the graphics context, and convert into a base64 encoded JPG.
        UIImage* capturedImage = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        NSData* jpgData = UIImageJPEGRepresentation(capturedImage, .8f);
        NSString* encodedData = [jpgData base64EncodedStringWithOptions:0];
        
        // Fire the onSnapshotDataReturned event if hooked up.
        if (self.onSnapshotDataReturned != nil) {
            self.onSnapshotDataReturned(@{ @"data":encodedData});
        }
    });
}

@end
