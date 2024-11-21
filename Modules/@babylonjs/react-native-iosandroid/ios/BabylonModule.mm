#import "BabylonNativeInterop.h"

#import <React/RCTBridgeModule.h>
#import <ReactCommon/CallInvoker.h>
#import <React/RCTInvalidating.h>

#ifdef RCT_NEW_ARCH_ENABLED
#import <BabylonReactNative/BabylonReactNative.h>
#endif

#import <Foundation/Foundation.h>

@interface RCTBridge (CallInvoker)
- (std::shared_ptr<facebook::react::CallInvoker>)jsCallInvoker;
@end

@interface BabylonModule : NSObject <RCTBridgeModule, RCTInvalidating>
@end

#ifdef RCT_NEW_ARCH_ENABLED
@interface BabylonModule () <NativeBabylonModuleSpec>
@end
#endif // RCT_NEW_ARCH_ENABLED

@implementation BabylonModule

RCT_EXPORT_MODULE();

@synthesize bridge = _bridge;

RCT_EXPORT_METHOD(initialize:(RCTPromiseResolveBlock)resolve reject:(RCTPromiseRejectBlock)reject) {
    self.bridge.jsCallInvoker->invokeAsync([bridge{ self.bridge }, resolve]() {
        [BabylonNativeInterop initialize:bridge];
        resolve([NSNull null]);
    });
}


RCT_EXPORT_METHOD(resetView:(RCTPromiseResolveBlock)resolve reject:(RCTPromiseRejectBlock)reject) {
    dispatch_async(dispatch_get_main_queue(), ^{
        [BabylonNativeInterop resetView];
        resolve([NSNull null]);
    });
}

- (void)invalidate {
    [BabylonNativeInterop invalidate];
}

#ifdef RCT_NEW_ARCH_ENABLED
- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:(const facebook::react::ObjCTurboModule::InitParams &)params {
  return std::make_shared<facebook::react::NativeBabylonModuleSpecJSI>(params);
}
#endif // RCT_NEW_ARCH_ENABLED

@end
