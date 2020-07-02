#!/bin/sh

mkdir -p iOS/Build
pushd iOS/Build
cmake -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../../Apps/Playground/node_modules/@babylonjs/react-native/submodules/BabylonNative/Dependencies/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64COMBINED -DENABLE_ARC=0 -DDEPLOYMENT_TARGET=12 -DENABLE_GLSLANG_BINARIES=OFF -DSPIRV_CROSS_CLI=OFF ..
xcodebuild -sdk iphoneos -configuration Release -project ReactNativeBabylon.xcodeproj -scheme BabylonNative build CODE_SIGNING_ALLOWED=NO
xcodebuild -sdk iphonesimulator -configuration Release -project ReactNativeBabylon.xcodeproj -scheme BabylonNative build CODE_SIGNING_ALLOWED=NO
popd

mkdir -p Assembled
cp ../Apps/Playground/node_modules/@babylonjs/react-native/package.json Assembled
cp ../Apps/Playground/node_modules/@babylonjs/react-native/README.md Assembled
cp ../Apps/Playground/node_modules/@babylonjs/react-native/*.ts* Assembled
cp react-native-babylon.podspec Assembled

mkdir -p Assembled/ios
cp ../Apps/Playground/node_modules/@babylonjs/react-native/ios/*.h Assembled/ios
cp ../Apps/Playground/node_modules/@babylonjs/react-native/ios/*.mm Assembled/ios
# This xcodeproj is garbage that we don't need in the package, but `pod install` ignores the package if it doesn't contain at least one xcodeproj. 🤷🏼‍♂️
cp -R iOS/Build/ReactNativeBabylon.xcodeproj Assembled/ios

mkdir -p Assembled/ios/libs
cp iOS/Build/Release-iphoneos/*.a Assembled/ios/libs
