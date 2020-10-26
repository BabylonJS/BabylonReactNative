cmake_minimum_required(VERSION 3.13.2)

project(ReactNativeBabylon)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
# Note: we are only supporting UWP applications for the time being
set(BABYLON_NATIVE_PLATFORM "WindowsStore")
set(REACTNATIVE_DIR_CMAKE "${CMAKE_CURRENT_LIST_DIR}/../node_modules/react-native")

# TODO: unclear whether UWP supports JSI
# Note: iOS has to hard copy JSI for things to work
# Configure Babylon Native to use JSI
set(NAPI_JAVASCRIPT_ENGINE "JSI" CACHE STRING "The JavaScript engine to power N-API")
add_subdirectory(${REACTNATIVE_DIR_CMAKE}/ReactCommon/jsi/jsi ${CMAKE_CURRENT_BINARY_DIR}/jsi)
target_include_directories(jsi INTERFACE ${REACTNATIVE_DIR_CMAKE}/ReactCommon/jsi)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/src/")

# TODO: not sure if this is what we should use
# it looks like iOS relies on the reactnative library, android relies on libturbomodulesjsijni.so
add_library(reactnative INTERFACE)
target_include_directories(reactnative INTERFACE ${REACTNATIVE_DIR_CMAKE}/ReactCommon/callinvoker)

# TODO: setup create BABYLON_NATIVE_DIR for WindowsStore
# TODO: its also unclear whether CMAKE_WINDOWS_ARCH_ABI is a valid variable
set(BABYLON_NATIVE_DIR "${CMAKE_CURRENT_LIST_DIR}/../submodules/BabylonNative")
add_subdirectory(${BABYLON_NATIVE_DIR} ${BABYLON_NATIVE_DIR}/build/uwp_${CMAKE_WINDOWS_ARCH_ABI}/)

set(BABYLON_REACT_NATIVE_SHARED_DIR "${CMAKE_CURRENT_LIST_DIR}/../shared")
add_subdirectory(${BABYLON_REACT_NATIVE_SHARED_DIR} ${CMAKE_CURRENT_BINARY_DIR}/shared)

add_library(BabylonNative
    ${CMAKE_CURRENT_LIST_DIR}/src/BabylonNative.h
    ${CMAKE_CURRENT_LIST_DIR}/src/BabylonNative.cpp)

target_include_directories(BabylonNative PUBLIC c/src)

target_link_libraries(BabylonNative
    arcana
    Graphics
    jsi
    reactnative
    BabylonReactNativeShared
    JsRuntime
    NativeWindow
    NativeEngine
    NativeInput
    NativeXr
    Window
    XMLHttpRequest)

warnings_as_errors(BabylonNative)
target_compile_definitions(BabylonNative PRIVATE UNICODE)
target_compile_definitions(BabylonNative PRIVATE _UNICODE)
target_compile_options(BabylonNative PRIVATE /ZW)
target_compile_options(BabylonNative PRIVATE /await)

# TODO: c++ 17 is not set up correctly after running this generation