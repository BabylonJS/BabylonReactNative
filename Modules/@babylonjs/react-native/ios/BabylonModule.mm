#import "BabylonNativeInterop.h"

#import <React/RCTBridgeModule.h>
#import <ReactCommon/CallInvoker.h>

#import <Foundation/Foundation.h>

@interface RCTBridge (RCTTurboModule)
- (std::shared_ptr<facebook::react::CallInvoker>)jsCallInvoker;
@end

@interface BabylonModule : NSObject <RCTBridgeModule>
@end

@implementation BabylonModule

RCT_EXPORT_MODULE();

@synthesize bridge = _bridge;

//- (instancetype)init
//{
//  self = [super init];
//  return self;
//}

// This would be called very early in EngineHook.js
RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(initialize2) {
    [BabylonNativeInterop2 initialize:self.bridge];
    return nil;
}

RCT_EXPORT_METHOD(initialize:(RCTPromiseResolveBlock)resolve reject:(RCTPromiseRejectBlock)reject) {
    [BabylonNativeInterop whenInitialized:self.bridge resolve:resolve];
}

RCT_EXPORT_METHOD(whenInitialized:(RCTPromiseResolveBlock)resolve reject:(RCTPromiseRejectBlock)reject) {
    [BabylonNativeInterop whenInitialized:self.bridge resolve:resolve];
}

RCT_EXPORT_METHOD(reset:(RCTPromiseResolveBlock)resolve reject:(RCTPromiseRejectBlock)reject) {
    self.bridge.jsCallInvoker->invokeAsync([resolve]() {
        [BabylonNativeInterop reset];
        resolve([NSNull null]);
    });
}

@end
