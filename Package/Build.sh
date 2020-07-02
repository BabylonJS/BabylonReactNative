#!/bin/sh

mkdir -p iOS/Build
pushd iOS/Build
cmake -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../../Apps/Playground/node_modules/@babylonjs/react-native/submodules/BabylonNative/Dependencies/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64COMBINED -DENABLE_ARC=0 -DDEPLOYMENT_TARGET=12 -DENABLE_GLSLANG_BINARIES=OFF -DSPIRV_CROSS_CLI=OFF ..
xcodebuild -sdk iphoneos -configuration Release -project ReactNativeBabylon.xcodeproj -scheme BabylonNative build CODE_SIGNING_ALLOWED=NO
xcodebuild -sdk iphonesimulator -configuration Release -project ReactNativeBabylon.xcodeproj -scheme BabylonNative build CODE_SIGNING_ALLOWED=NO
popd

pushd ../Apps/Playground/android
pwd
./gradlew babylonjs_react-native:assembleRelease
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

# Merge each supported lib architecture into a single "universal library" that can be used with any of the supported architectures.
mkdir -p Assembled/ios/libs
for lib in iOS/Build/Release-iphoneos/*.a
do
    lipo -create $lib iOS/Build/Release-iphonesimulator/$(basename $lib) -output Assembled/ios/libs/$(basename $lib)
done


mkdir -p Assembled/android
cp Android/build.gradle Assembled/android

mkdir -p Assembled/android/src/main
cp ../Apps/Playground/node_modules/@babylonjs/react-native/android/src/main/AndroidManifest.xml Assembled/android/src/main

mkdir -p Assembled/android/src/main/java
cp -R ../Apps/Playground/node_modules/@babylonjs/react-native/android/src/main/java Assembled/android/src/main
cp -R ../Apps/Playground/node_modules/@babylonjs/react-native/android/build/intermediates/library_and_local_jars_jni/release/jni Assembled/android/src/main
mv Assembled/android/src/main/jni Assembled/android/src/main/jniLibs
