#import "BabylonNativeInterop.h"

#import <React/RCTViewManager.h>
#import <React/RCTUIManager.h>
#import <ReactCommon/CallInvoker.h>

#import <Foundation/Foundation.h>

#import <MetalKit/MetalKit.h>

@interface EngineView : MTKView

@property (nonatomic, copy) RCTDirectEventBlock onSnapshotDataReturned;
@property (nonatomic, assign) BOOL isTransparent;
@property (nonatomic, assign) NSNumber* antiAliasing;

@end

@implementation EngineView {
    const RCTBridge* bridge;
    MTKView* xrView;
}

- (instancetype)init:(RCTBridge*)_bridge {
  #ifdef TARGET_OS_OSX
        const CGSize monitor = [[NSScreen mainScreen] frame].size;
        const CGSize appWin = [[NSApp mainWindow] frame].size;
        if (self = [super initWithFrame:CGRectMake((appWin.width - monitor.width) / 2, (appWin.height - monitor.height) / 2, monitor.width, monitor.height)
			device:MTLCreateSystemDefaultDevice()]) {
        bridge = _bridge;
        super.translatesAutoresizingMaskIntoConstraints = false;
        super.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        super.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
    }
    
	[self setBounds:CGRectMake((appWin.width - monitor.width) / 2, (appWin.height - monitor.height) / 2, monitor.width, monitor.height)];
  #else
	if (self = [super initWithFrame:CGRectZero device:MTLCreateSystemDefaultDevice()]) {
        bridge = _bridge;
        super.translatesAutoresizingMaskIntoConstraints = false;
        super.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        super.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
    }
  #endif
    return self;
}

- (void)setIsTransparentFlag:(NSNumber*)isTransparentFlag {
  #ifndef TARGET_OS_OSX
    BOOL isTransparent = [isTransparentFlag intValue] == 1;
    if(isTransparent){
        [self setOpaque:NO];
    } else {
        [self setOpaque:YES];
    }
    self.isTransparent = isTransparent;
  #endif
}

- (void)setMSAA:(NSNumber*)value {
    [BabylonNativeInterop updateMSAA:value];
}

- (void)setBounds:(CGRect)bounds {
    [super setBounds:bounds];
    [BabylonNativeInterop updateView:self];
}

#ifndef TARGET_OS_OSX
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
#endif

- (void)drawRect:(CGRect)rect {
  #ifndef TARGET_OS_OSX
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
  #endif
    [BabylonNativeInterop renderView];
}

- (void)dealloc {
    [BabylonNativeInterop updateXRView:nil];
}

- (void)takeSnapshot {
  #ifndef TARGET_OS_OSX
    // We must take the screenshot on the main thread otherwise we might fail to get a valid handle on the view's image.
    dispatch_async(dispatch_get_main_queue(), ^{
        // Start the graphics context.
        UIGraphicsBeginImageContextWithOptions(self.bounds.size, YES, 0.0f);
        
        // Draw the current state of the view into the graphics context.
        [self drawViewHierarchyInRect:self.bounds afterScreenUpdates:NO];
        
        // Grab the image from the graphics context, and convert into a base64 encoded JPG.
        NSImage* capturedImage = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        NSData* jpgData = UIImageJPEGRepresentation(capturedImage, .8f);
        NSString* encodedData = [jpgData base64EncodedStringWithOptions:0];
        
        // Fire the onSnapshotDataReturned event if hooked up.
        if (self.onSnapshotDataReturned != nil) {
            self.onSnapshotDataReturned(@{ @"data":encodedData});
        }
    });
  #endif
}

#ifdef TARGET_OS_OSX
- (void)layout {
		[super layout];
		const CGSize monitor = [[NSScreen mainScreen] frame].size;
		const CGSize appWin = [[NSApp mainWindow] frame].size;
		self.frame = CGRectMake((appWin.width - monitor.width) / 2, (appWin.height - monitor.height) / 2, monitor.width, monitor.height);
}
#endif

@end


@interface EngineViewManager : RCTViewManager
@end

@implementation EngineViewManager

RCT_CUSTOM_VIEW_PROPERTY(isTransparent, NSNumber*, EngineView){
    [view setIsTransparentFlag:json];
}

RCT_CUSTOM_VIEW_PROPERTY(antiAliasing, NSNumber*, EngineView){
    [view setMSAA:json];
}

RCT_EXPORT_MODULE(EngineViewManager)

RCT_EXPORT_VIEW_PROPERTY(onSnapshotDataReturned, RCTDirectEventBlock)

RCT_EXPORT_METHOD(takeSnapshot:(nonnull NSNumber*) reactTag) {
    // Marshal the takeSnapshot call to the appropriate EngineView.
    [self.bridge.uiManager addUIBlock:^(RCTUIManager *uiManager, NSDictionary<NSNumber*,NSView*>* viewRegistry) {
        EngineView* view = (EngineView*)viewRegistry[reactTag];
        if (!view || ![view isKindOfClass:[EngineView class]]) {
            return;
        }
        [view takeSnapshot];
    }];
}

- (NSView*)view {
    return [[EngineView alloc] init:self.bridge];
}

@end
