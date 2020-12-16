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

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(initialize) {
    [BabylonNativeInterop initialize:self.bridge];
    return nil;
}

@end
