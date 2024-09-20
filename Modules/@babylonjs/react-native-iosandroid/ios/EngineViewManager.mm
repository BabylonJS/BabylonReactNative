#import "BabylonNativeInterop.h"

#import <React/RCTViewManager.h>
#import <React/RCTUIManager.h>
#import <ReactCommon/CallInvoker.h>

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <MetalKit/MetalKit.h>
#import "BabylonEngineView.h"


#if !RCT_NEW_ARCH_ENABLED

@interface EngineViewManager : RCTViewManager
@end

@implementation EngineViewManager

RCT_CUSTOM_VIEW_PROPERTY(isTransparent, NSNumber*, EngineView){
    [view setIsTransparentFlag:json];
}

RCT_CUSTOM_VIEW_PROPERTY(antiAliasing, NSNumber*, EngineView){
    [view setMSAA:json];
}

RCT_EXPORT_MODULE(EngineViewNativeComponent)

RCT_EXPORT_VIEW_PROPERTY(onSnapshotDataReturned, RCTDirectEventBlock)

RCT_EXPORT_METHOD(takeSnapshot:(nonnull NSNumber*) reactTag) {
    // Marshal the takeSnapshot call to the appropriate EngineView.
    [self.bridge.uiManager addUIBlock:^(RCTUIManager *uiManager, NSDictionary<NSNumber*,UIView*>* viewRegistry) {
        EngineView* view = (EngineView*)viewRegistry[reactTag];
        if (!view || ![view isKindOfClass:[EngineView class]]) {
            return;
        }
        [view takeSnapshot];
    }];
}

- (UIView*)view {
    return [[EngineView alloc] init];
}

@end

#endif // !RCT_NEW_ARCH_ENABLED
