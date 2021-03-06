cmake_minimum_required(VERSION 3.13.2)
project(ReactNativeBabylon)
include(${CMAKE_CURRENT_LIST_DIR}/../shared/CMakeLists.txt)

set(BABYLON_NATIVE_PLATFORM "Apple")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -x objective-c++")

# configure Babylon Native to use JSI
set(NAPI_JAVASCRIPT_ENGINE "JSI" CACHE STRING "The JavaScript engine to power N-API")

get_filename_component(REACTNATIVE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../react-native" ABSOLUTE)
if(NOT EXISTS ${REACTNATIVE_DIR})
    get_filename_component(REACTNATIVE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../Apps/Playground/node_modules/react-native" ABSOLUTE)
endif()

# The CMake generator for XCode produces relative paths to source files. Additionally, when an XCode project
# that lives in a sym linked directory is added to an XCode workspace, XCode resolves the actual path of the
# XCode project. This combination results in incorrect relative paths, so just copy the jsi directory locally.
file(COPY ${REACTNATIVE_DIR}/ReactCommon/jsi/jsi DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/jsi)
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/jsi/jsi ${CMAKE_CURRENT_BINARY_DIR}/jsi)
target_include_directories(jsi INTERFACE ${REACTNATIVE_DIR}/ReactCommon/jsi)

add_library(reactnative INTERFACE)
target_include_directories(reactnative INTERFACE ${REACTNATIVE_DIR}/ReactCommon/callinvoker)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/")

set(BABYLON_NATIVE_DIR "${CMAKE_CURRENT_LIST_DIR}/../submodules/BabylonNative")
# Only add the subdirectories we need to avoid consuming the BabylonNative/Apps folder
add_subdirectory("${BABYLON_NATIVE_DIR}/Dependencies" ${BABYLON_NATIVE_DIR}/build/ios/Dependencies EXCLUDE_FROM_ALL)
add_subdirectory("${BABYLON_NATIVE_DIR}/Core" ${BABYLON_NATIVE_DIR}/build/ios/Core EXCLUDE_FROM_ALL)
add_subdirectory("${BABYLON_NATIVE_DIR}/Plugins" ${BABYLON_NATIVE_DIR}/build/ios/Plugins EXCLUDE_FROM_ALL)
add_subdirectory("${BABYLON_NATIVE_DIR}/Polyfills" ${BABYLON_NATIVE_DIR}/build/ios/Polyfills EXCLUDE_FROM_ALL)

set(BABYLON_REACT_NATIVE_SHARED_DIR "${CMAKE_CURRENT_LIST_DIR}/../shared")
add_subdirectory(${BABYLON_REACT_NATIVE_SHARED_DIR} ${CMAKE_CURRENT_BINARY_DIR}/shared)

add_library(BabylonNative
    ${SHARED_SOURCES})

target_compile_definitions(BabylonNative PRIVATE UNICODE)
target_compile_definitions(BabylonNative PRIVATE _UNICODE)

target_include_directories(BabylonNative PRIVATE ${SHARED_INCLUDES})
target_include_directories(BabylonNative PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(BabylonNative
    z
    arcana
    Graphics
    jsi
    reactnative
    JsRuntime
    NativeCapture
    NativeEngine
    NativeInput
    NativeXr
    Window
    XMLHttpRequest
    Canvas)

# TODO: For some reason these don't work, so we specify these in the CMake command line args.
set_target_properties(BabylonNative PROPERTIES
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 12.0
    XCODE_ATTRIBUTE_ENABLE_BITCODE YES
)