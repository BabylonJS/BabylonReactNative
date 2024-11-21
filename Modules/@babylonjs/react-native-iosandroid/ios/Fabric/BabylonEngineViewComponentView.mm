#ifdef RCT_NEW_ARCH_ENABLED

#import "BabylonEngineViewComponentView.h"
#include <Foundation/Foundation.h>
#include <UIKit/UIKit.h>
#include "react/renderer/componentregistry/ComponentDescriptorProvider.h"

#import <react/renderer/components/BabylonReactNative/ComponentDescriptors.h>
#import <react/renderer/components/BabylonReactNative/EventEmitters.h>
#import <react/renderer/components/BabylonReactNative/Props.h>
#import <react/renderer/components/BabylonReactNative/RCTComponentViewHelpers.h>

#import "RCTFabricComponentsPlugins.h"

#import "BabylonEngineView.h"
#import "BabylonNativeInterop.h"

using namespace facebook::react;

@interface BabylonEngineViewComponentView () <RCTEngineViewViewProtocol>
@end

@implementation BabylonEngineViewComponentView {
  EngineView* _engineView;
}

+ (ComponentDescriptorProvider)componentDescriptorProvider {
  return concreteComponentDescriptorProvider<EngineViewComponentDescriptor>();
}

- (instancetype)initWithFrame:(CGRect)frame {
  if (self = [super initWithFrame:frame]) {
    static const auto defaultProps = std::make_shared<const EngineViewProps>();
    _props = defaultProps;
    _engineView = [[EngineView alloc] init];
    _engineView.translatesAutoresizingMaskIntoConstraints = false;
    self.contentView = _engineView;
    
    [NSLayoutConstraint activateConstraints:@[
      [_engineView.topAnchor constraintEqualToAnchor:self.topAnchor],
      [_engineView.leftAnchor constraintEqualToAnchor:self.leftAnchor],
      [_engineView.bottomAnchor constraintEqualToAnchor:self.bottomAnchor],
      [_engineView.rightAnchor constraintEqualToAnchor:self.rightAnchor],
    ]];
  }
  return self;
}

- (void)updateProps:(Props::Shared const &)props oldProps:(Props::Shared const &)oldProps
{
  const auto &oldViewProps = *std::static_pointer_cast<EngineViewProps const>(_props);
  const auto &newViewProps = *std::static_pointer_cast<EngineViewProps const>(props);
  
  if (oldViewProps.isTransparent != newViewProps.isTransparent) {
    [_engineView setIsTransparentFlag:[NSNumber numberWithBool:newViewProps.isTransparent]];
  }
  
  if (oldViewProps.antiAliasing != newViewProps.antiAliasing) {
    [_engineView setMSAA:[NSNumber numberWithInt:newViewProps.antiAliasing]];
  }
  
  [super updateProps:props oldProps:oldProps];
}

- (void)prepareForRecycle
{
  [super prepareForRecycle];
  _engineView = nil;
}

- (void)takeSnapshot {
  [_engineView takeSnapshot];
}

@end

Class<RCTComponentViewProtocol> EngineViewCls(void) {
  return BabylonEngineViewComponentView.class;
}

#endif
