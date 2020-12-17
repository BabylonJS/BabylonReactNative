#import "BabylonNativeInterop.h"
#import "../shared/BabylonNative.h"

#import <React/RCTBridge+Private.h>
#import <jsi/jsi.h>

#import <Foundation/Foundation.h>

#import <memory>

using namespace facebook;

@interface RCTBridge (RCTTurboModule)
- (std::shared_ptr<facebook::react::CallInvoker>)jsCallInvoker;
@end

namespace {
    jsi::Runtime* GetJSIRuntime(RCTBridge* bridge) {
        RCTCxxBridge* cxxBridge = reinterpret_cast<RCTCxxBridge*>(bridge);
        return reinterpret_cast<jsi::Runtime*>(cxxBridge.runtime);
    }
}

@implementation BabylonNativeInterop

static NSMutableArray* activeTouches;

+ (void)initialize:(RCTBridge*)bridge {
    Babylon::Initialize(*GetJSIRuntime(bridge), bridge.jsCallInvoker);

    [[NSNotificationCenter defaultCenter] removeObserver:self
        name:RCTBridgeWillInvalidateModulesNotification
        object:bridge.parentBridge];

    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(onBridgeWillInvalidate:)
        name:RCTBridgeWillInvalidateModulesNotification
        object:bridge.parentBridge];
}

// NOTE: This happens during dev mode reload, when the JS engine is being shutdown and restarted.
+ (void)onBridgeWillInvalidate:(NSNotification*)notification
{
    Babylon::Deinitialize();
}

+ (void)updateView:(MTKView*)mtkView {
    const int width = static_cast<int>(mtkView.bounds.size.width * UIScreen.mainScreen.scale);
    const int height = static_cast<int>(mtkView.bounds.size.height * UIScreen.mainScreen.scale);
    if (width != 0 && height != 0) {
        Babylon::UpdateView((__bridge void*)mtkView, width, height);
    }
}

+ (void)reportTouchEvent:(MTKView*)mtkView touches:(NSSet<UITouch*>*)touches event:(UIEvent*)event {
    for (UITouch* touch in touches) {
        if (touch.view == mtkView) {
            const CGFloat scale = UIScreen.mainScreen.scale;
            const CGPoint pointerPosition = [touch locationInView:mtkView];
            const uint32_t x = static_cast<uint32_t>(pointerPosition.x * scale);
            const uint32_t y = static_cast<uint32_t>(pointerPosition.y * scale);

            switch (touch.phase) {
                case UITouchPhaseBegan: {
                    NSUInteger pointerId = [activeTouches indexOfObject:[NSNull null]];
                    if (pointerId == NSNotFound) {
                        pointerId = [activeTouches count];
                        [activeTouches addObject:touch];
                    } else {
                        [activeTouches replaceObjectAtIndex:pointerId withObject:touch];
                    }
                    Babylon::SetPointerButtonState(static_cast<uint32_t>(pointerId), 0, true, x, y);
                    break;
                }

                case UITouchPhaseMoved: {
                    NSUInteger pointerId = [activeTouches indexOfObject:touch];
                    Babylon::SetPointerPosition(static_cast<uint32_t>(pointerId), x, y);
                    break;
                }

                case UITouchPhaseEnded:
                case UITouchPhaseCancelled: {
                    NSUInteger pointerId = [activeTouches indexOfObject:touch];
                    [activeTouches replaceObjectAtIndex:pointerId withObject:[NSNull null]];
                    Babylon::SetPointerButtonState(static_cast<uint32_t>(pointerId), 0, false, x, y);
                    break;
                }

                default:
                    break;
            }
        }
    }
}

@end
