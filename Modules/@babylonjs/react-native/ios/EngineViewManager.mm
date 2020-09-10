#import "BabylonNativeInterop.h"

#import <React/RCTViewManager.h>
#import <React/RCTUIManager.h>

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <MetalKit/MetalKit.h>

@interface EngineView : MTKView

@property (nonatomic, copy) RCTDirectEventBlock onSnapshotDataReturned;

@end

@implementation EngineView {
    RCTBridge* bridge;
    NSRunLoop* runLoop;
}

- (instancetype)init:(RCTBridge*)_bridge runLoop:(NSRunLoop*)_runLoop {
    if (self = [super initWithFrame:CGRectZero device:MTLCreateSystemDefaultDevice()]) {
        bridge = _bridge;
        runLoop = _runLoop;

        super.translatesAutoresizingMaskIntoConstraints = false;
        super.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        super.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
    }
    return self;
}

- (void)setBounds:(CGRect)bounds {
    [super setBounds:bounds];
    [BabylonNativeInterop setView:bridge jsRunLoop:runLoop mktView:self];
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [BabylonNativeInterop reportTouchEvent:touches withEvent:event];
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [BabylonNativeInterop reportTouchEvent:touches withEvent:event];
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [BabylonNativeInterop reportTouchEvent:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [BabylonNativeInterop reportTouchEvent:touches withEvent:event];
}

- (void)takeSnapshot {
    // Grab a reference to the currently presented drawable on us or a subview.
    id<MTLTexture> currentDrawable = nil;
    while (currentDrawable == nil) {
        if (self.subviews != nil && self.subviews.count > 0) {
            MTKView *xrView = (MTKView *)self.subviews[0];
            currentDrawable = [xrView.currentDrawable texture];
        }
        else {
            currentDrawable = [self.currentDrawable texture];
        }
    }
    
    int width = (int)[currentDrawable width];
    int height = (int)[currentDrawable height];
    int rowBytes = width * 4;
    int textureSize = width * height * 4;
    
    // Allocate the bitmap, and load in the bytes
    void* bitMap = malloc (textureSize);
    [currentDrawable getBytes:bitMap bytesPerRow:rowBytes fromRegion:MTLRegionMake2D(0, 0, width, height) mipmapLevel:0];
    
    // Create the CGImage representation.
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Little | kCGImageAlphaFirst;
    CGDataProviderRef provider = CGDataProviderCreateWithData(nil, bitMap, textureSize, nil);
    CGImageRef cgImageRef = CGImageCreate(width, height, 8, 32, rowBytes, colorSpace, bitmapInfo, provider, nil, true, (CGColorRenderingIntent)kCGRenderingIntentDefault);
    
    // Create the UIImage from the CG Image.
    UIImage *uiImage = [UIImage imageWithCGImage:cgImageRef];
    
    NSData *pngData = UIImagePNGRepresentation(uiImage);
    NSString *encodedData = [pngData base64EncodedStringWithOptions:0];
    if (self.onSnapshotDataReturned != nil && encodedData != nil) {
        self.onSnapshotDataReturned(@{ @"data":encodedData});
    }
    
    CFRelease(cgImageRef);
    free(bitMap);
}

@end


@interface EngineViewManager : RCTViewManager
@end

@implementation EngineViewManager {
    NSRunLoop* runLoop;
}

RCT_EXPORT_MODULE(EngineViewManager)

RCT_EXPORT_VIEW_PROPERTY(onSnapshotDataReturned, RCTDirectEventBlock)

RCT_EXPORT_METHOD(takeSnapshot:(nonnull NSNumber*) reactTag) {
    [self.bridge.uiManager addUIBlock:^(RCTUIManager *uiManager, NSDictionary<NSNumber *,UIView *> *viewRegistry) {
        EngineView *view = (EngineView *)viewRegistry[reactTag];
        if (!view || ![view isKindOfClass:[EngineView class]]) {
            RCTLogError(@"Cannot find EngineView with tag #%@", reactTag);
            return;
        }
        [view takeSnapshot];
    }];
}

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(setJSThread) {
    runLoop = [NSRunLoop currentRunLoop];
    return nil;
}

- (UIView*)view {
    return [[EngineView alloc] init:self.bridge runLoop:runLoop];
}

@end
