#pragma once
#import <UIKit/UIKit.h>
#import <MetalKit/MetalKit.h>
#import <React/RCTBridge.h>
#import <React/RCTComponent.h>

@interface EngineView : MTKView

@property (nonatomic, copy) RCTDirectEventBlock onSnapshotDataReturned;
@property (nonatomic, assign) BOOL isTransparent;
@property (nonatomic, assign) NSNumber* antiAliasing;

- (void)setMSAA:(NSNumber*)value;
- (void)takeSnapshot;
- (void)setIsTransparentFlag:(NSNumber*)isTransparentFlag;

@end
