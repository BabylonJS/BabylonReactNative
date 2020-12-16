#pragma once

#import <MetalKit/MetalKit.h>
#import <React/RCTBridge.h>

@interface BabylonNativeInterop2 : NSObject
+ (void)initialize:(RCTBridge*)bridge;
+ (void)updateView:(MTKView*)mtkView;
+ (void)reportTouchEvent:(MTKView*)mtkView touches:(NSSet<UITouch*>*)touches event:(UIEvent*)event;
@end

@interface BabylonNativeInterop : NSObject
+ (void)setView:(RCTBridge*)bridge jsRunLoop:(NSRunLoop*)jsRunLoop mktView:(MTKView*)mtkView;
+ (void)reportTouchEvent:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event;
+ (void)whenInitialized:(RCTBridge*)bridge resolve:(RCTPromiseResolveBlock)resolve;
+ (void)reset;
@end
