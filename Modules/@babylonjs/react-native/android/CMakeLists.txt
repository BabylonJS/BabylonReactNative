cmake_minimum_required(VERSION 3.13.2)
# set targetPlatform, will be passed in from gradle when this sample is completed
# openGL Supportability
# platform         status
#   (0 12)           ES2/ES3 not supported
#   [12, 18)         ES2 only; for ES3, app do dynamic load/detection
#                    this applies to the situations that:
#                        - minimum API is set to less than 18. In this case
#                          there is no ES3 header/lib support inside NDK
#                        - the built APK might be running on newer API phones
#                    with dynamic loading of ES3, the same APK would still be able
#                    to use ES3. Otherwise, app would stuck with ES2 even phone is
#                    is newer than the minimum API level (for example, Android-27 etc).
#
#   [18, 24)         ES2 & ES3
#                    If app is built to only support API-18 or later,
#                    set minimum api level to 18 is good enough, NDK supprts ES3
#                    with the right header and lib files. No need to use ES3 dynamic
#                    detection.
#   [24, infinite)   ES2 & ES3 & Vulkan
project(ReactNativeBabylon)

set(THIS_LIBRARY BabylonNative)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_STANDARD 17)
file(TO_CMAKE_PATH "${REACTNATIVE_DIR}" REACTNATIVE_DIR_CMAKE)

# Configure Babylon Native to use JSI
set(NAPI_JAVASCRIPT_ENGINE "JSI" CACHE STRING "The JavaScript engine to power N-API")
add_subdirectory(${REACTNATIVE_DIR_CMAKE}/ReactCommon/jsi/jsi ${CMAKE_CURRENT_BINARY_DIR}/jsi)
target_include_directories(jsi INTERFACE ${REACTNATIVE_DIR_CMAKE}/ReactCommon/jsi)
target_include_directories(jsi INTERFACE ${REACTNATIVE_DIR_CMAKE}/ReactAndroid/src/main/jni/react/turbomodule)
target_include_directories(jsi INTERFACE ${REACTNATIVE_DIR_CMAKE}/ReactCommon/callinvoker)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/src/")

set(BABYLON_NATIVE_BUILD_APPS OFF CACHE BOOL "")
set(BABYLON_REACT_NATIVE_SHARED_DIR "${CMAKE_CURRENT_LIST_DIR}/../shared")
add_subdirectory(${BABYLON_REACT_NATIVE_SHARED_DIR} ${CMAKE_CURRENT_BINARY_DIR}/shared)

if (TARGET NativeXr)
    disable_warnings(NativeXr)
endif()
if (TARGET NativeCamera)
    disable_warnings(NativeCamera)
endif()
disable_warnings(XMLHttpRequest)
disable_warnings(Graphics)
disable_warnings(NativeEngine)
disable_warnings(NativeInput)
disable_warnings(NativeCapture)
disable_warnings(NativeOptimizations) # deprecated stl
disable_warnings(NativeTracing) # deprecated stl
disable_warnings(JsRuntime) # deprecated stl
disable_warnings(Scheduling) # deprecated stl
disable_warnings(Canvas)
disable_warnings(Window)

add_library(fbjni SHARED IMPORTED)
set_target_properties(fbjni PROPERTIES
    IMPORTED_LOCATION ${FBJNI_LIBPATH}/${ANDROID_ABI}/libfbjni.so
    INTERFACE_INCLUDE_DIRECTORIES ${FBJNI_INCPATH})

add_library(BabylonNative SHARED
    src/main/cpp/BabylonNativeInterop.cpp
    ${SHARED_SOURCES})

disable_warnings(BabylonNative) # deprecated stl

# >= React Native 0.71 with prefabs
include(${REACTNATIVE_DIR_CMAKE}/ReactAndroid/cmake-utils/folly-flags.cmake)

# We configured the REACT_COMMON_DIR variable as it's commonly used to reference
# shared C++ code in other targets.
set(REACT_COMMON_DIR ${REACT_ANDROID_DIR}/../ReactCommon)

# If you have ccache installed, we're going to honor it.
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

set(BUILD_DIR ${PROJECT_BUILD_DIR})
# When building on Windows, PROJECT_BUILD_DIR might be empty. If it is, then the build breaks because of string replace
if(CMAKE_HOST_WIN32 AND NOT "${PROJECT_BUILD_DIR}" STREQUAL "")
    string(REPLACE "\\" "/" BUILD_DIR ${BUILD_DIR})
endif()

target_include_directories(BabylonNative
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_BUILD_DIR}/generated/rncli/src/main/jni)

target_compile_options(BabylonNative PRIVATE -Wall -Werror -fexceptions -frtti -std=c++17 -DWITH_INSPECTOR=1 -DLOG_TAG=\"ReactNative\")

find_package(fbjni REQUIRED CONFIG)
add_library(fbjni ALIAS fbjni::fbjni)

target_link_libraries(BabylonNative
    GLESv3
    android
    EGL
    log
    -lz
    arcana
    fbjni                               # via 3rd party prefab
    AndroidExtensions
    GraphicsDevice
    JsRuntime
    NativeCapture
    NativeEngine
    NativeInput
    NativeOptimizations
    NativeTracing
    Window
    XMLHttpRequest
    Canvas
    ${ADDITIONAL_LIBRARIES})

# We use an interface target to propagate flags to all the generated targets
# such as the folly flags or others.
#add_library(common_flags INTERFACE)
#target_compile_options(common_flags INTERFACE ${folly_FLAGS})
#target_link_libraries(ReactAndroid::react_codegen_rncore INTERFACE common_flags)

# If project is on RN CLI v9, then we can use the following lines to link against the autolinked 3rd party libraries.
if(EXISTS ${PROJECT_BUILD_DIR}/generated/rncli/src/main/jni/Android-rncli.cmake)
        include(${PROJECT_BUILD_DIR}/generated/rncli/src/main/jni/Android-rncli.cmake)
        target_link_libraries(${CMAKE_PROJECT_NAME} ${AUTOLINKED_LIBRARIES})
        foreach(autolinked_library ${AUTOLINKED_LIBRARIES})
            target_link_libraries(${autolinked_library} common_flags)
        endforeach()
endif()

# If project is running codegen at the app level, we want to link and build the generated library.
if(EXISTS ${PROJECT_BUILD_DIR}/generated/source/codegen/jni/CMakeLists.txt)
        add_subdirectory(${PROJECT_BUILD_DIR}/generated/source/codegen/jni/ codegen_app_build)
        get_property(APP_CODEGEN_TARGET DIRECTORY ${PROJECT_BUILD_DIR}/generated/source/codegen/jni/ PROPERTY BUILDSYSTEM_TARGETS)
        target_link_libraries(${CMAKE_PROJECT_NAME} ${APP_CODEGEN_TARGET})
        target_link_libraries(${APP_CODEGEN_TARGET} common_flags)
endif()

target_include_directories(BabylonNative PRIVATE ${SHARED_INCLUDES})

target_compile_definitions(BabylonNative PRIVATE BABYLON_NATIVE_PLUGIN_NATIVEXR=${BABYLON_NATIVE_PLUGIN_NATIVEXR})
target_compile_definitions(BabylonNative PRIVATE BABYLON_NATIVE_PLUGIN_NATIVECAMERA=${BABYLON_NATIVE_PLUGIN_NATIVECAMERA})
