cmake_minimum_required(VERSION 3.15.3)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(ReactNativeBabylon)

set(CMAKE_CXX_STANDARD 17)

# BabylonNative CMake entry
set(BABYLON_NATIVE_BUILD_APPS OFF CACHE BOOL "")
set(BABYLON_NATIVE_USE_SWAPCHAINPANEL ON CACHE BOOL "")

# Directory for CPPWINRT generated files (from BabylonReactNative.vcxproj / CppWinRT NuGet)
set(WINRT_GENERATED_FILES_DIR "${CMAKE_CURRENT_LIST_DIR}/BabylonReactNative/Generated Files")

# Add WINRT generated files directory globally
include_directories("${WINRT_GENERATED_FILES_DIR}" "${WINRT_GENERATED_FILES_DIR}/winrt")

# Ensure consistent CppWinRT compilation settings across all targets to match vcxproj
# This prevents linker errors due to CppWinRT version/setting mismatches
add_compile_definitions(
    WINRT_SOURCE_LOCATION=false
)

# Configure Babylon Native to use JSI
# Note: We should avoid installing node_modules in the Module\@babylonjs\react-native folder.
# Installing react-native dependencies for both the Playground app and the @babylonjs/react-native package will generate a bad bundle/runtime errors.
set(NAPI_JAVASCRIPT_ENGINE "JSI" CACHE STRING "")

set(PLAYGROUND_DIR "${CMAKE_CURRENT_LIST_DIR}/../../..")

if(EXISTS "${PLAYGROUND_DIR}/react-native/package.json")
    get_filename_component(REACTNATIVE_DIR_CMAKE "${PLAYGROUND_DIR}/react-native" ABSOLUTE)
else()
    message("Playground directory ${PLAYGROUND_DIR}")
    message(FATAL_ERROR "No Playground available")
endif()

add_subdirectory("${REACTNATIVE_DIR_CMAKE}/ReactCommon/jsi/jsi" ${CMAKE_CURRENT_BINARY_DIR}/jsi)
target_include_directories(jsi INTERFACE ${REACTNATIVE_DIR_CMAKE}/ReactCommon/jsi)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../shared ${CMAKE_CURRENT_BINARY_DIR}/shared)

# Disable Unity build for UrlLib because of conflict in header between windows.h and winrt
set_property(TARGET UrlLib PROPERTY UNITY_BUILD false)

add_library(BabylonNative ${SHARED_SOURCES})

target_compile_definitions(BabylonNative PRIVATE BABYLON_NATIVE_PLUGIN_NATIVEXR=${BABYLON_NATIVE_PLUGIN_NATIVEXR})
target_compile_definitions(BabylonNative PRIVATE BABYLON_NATIVE_PLUGIN_NATIVECAMERA=${BABYLON_NATIVE_PLUGIN_NATIVECAMERA})
target_include_directories(BabylonNative PRIVATE ${SHARED_INCLUDES})

# Ensure all dependency targets use consistent CppWinRT settings to match BabylonModule.obj
# Apply WINRT_SOURCE_LOCATION=false to all BabylonNative and its dependencies
set(BABYLON_TARGETS arcana GraphicsDevice jsi JsRuntime NativeCapture NativeEngine NativeInput NativeOptimizations NativeTracing Window Scheduling XMLHttpRequest ShaderCache Canvas)
foreach(TARGET_NAME ${BABYLON_TARGETS})
    if(TARGET ${TARGET_NAME})
        get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
        if(TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
            target_compile_definitions(${TARGET_NAME} INTERFACE WINRT_SOURCE_LOCATION=false)
        else()
            target_compile_definitions(${TARGET_NAME} PRIVATE WINRT_SOURCE_LOCATION=false)
        endif()
    endif()
endforeach()

# because of warning C5030: attribute 'msvc::intrinsic' is not recognized
# solution is to update VS version but this might bring more issues
disable_warnings(Graphics)
disable_warnings(NativeEngine)
disable_warnings(NativeCamera)
disable_warnings(NativeInput)
disable_warnings(NativeCapture)
disable_warnings(Canvas)
disable_warnings(Window)
disable_warnings(ExternalTexture)

target_link_libraries(BabylonNative
    arcana
    GraphicsDevice
    jsi
    JsRuntime
    NativeCapture
    NativeEngine
    NativeInput
    NativeOptimizations
    NativeTracing
    Window
    Scheduling
    XMLHttpRequest
    ShaderCache
    Canvas
    ${ADDITIONAL_LIBRARIES})

