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
    dispatch_async(dispatch_get_main_queue(), ^{
        UIGraphicsBeginImageContextWithOptions(self.bounds.size, YES, 0.0f);
        [self drawViewHierarchyInRect:self.bounds afterScreenUpdates:YES];
        UIImage *uiImage = UIGraphicsGetImageFromCurrentImageContext();
        NSData *jpgData = UIImageJPEGRepresentation(uiImage, 100.0f);
        NSString *encodedData = [jpgData base64EncodedStringWithOptions:0];
        if (self.onSnapshotDataReturned != nil && encodedData != nil) {
            self.onSnapshotDataReturned(@{ @"data":encodedData});
        }
        
        UIGraphicsEndImageContext();
    });
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
