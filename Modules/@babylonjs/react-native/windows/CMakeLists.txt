cmake_minimum_required(VERSION 3.15.3)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(ReactNativeBabylon)

set(CMAKE_CXX_STANDARD 17)

# BabylonNative CMake entry
set(BABYLON_NATIVE_BUILD_APPS OFF CACHE BOOL "")
set(BABYLON_NATIVE_USE_SWAPCHAINPANEL ON CACHE BOOL "")

# Force all cmake-built targets to use the same C++/WinRT headers as the vcxproj.
# The vcxproj uses the CppWinRT NuGet package version pinned by react-native-windows
# (CppWinRTVersion, typically 2.0.230706.1).
#
# Problem: cmake's include_directories(BEFORE) puts our path in AdditionalIncludeDirectories,
# but the cmake VS generator for WindowsStore auto-adds a NuGet packages.config referencing
# the latest Microsoft.Windows.CppWinRT. That NuGet package's .props runs AFTER cmake's
# include dirs and prepends its (newer) path, winning the search order.
#
# Fix: generate a Directory.Build.targets in the cmake binary directory. MSBuild imports
# .targets files AFTER all .props and project content, including NuGet package .props.
# An ItemDefinitionGroup there prepends our path LAST, making it first in the final
# compiler include order and ensuring the correct CppWinRT version is found.
#
# CPPWINRT_INCLUDE_PATH is passed from BabylonNativeBuild MSBuild target in the vcxproj:
#   -DCPPWINRT_INCLUDE_PATH="$(CppWinRTToolkitDir)build\native\include"
# $(CppWinRTToolkitDir) is set by the packages.config-restored Microsoft.Windows.CppWinRT.props
# (nuget restore runs before MSBuild, so the path is guaranteed to exist).
if(DEFINED CPPWINRT_INCLUDE_PATH AND NOT "${CPPWINRT_INCLUDE_PATH}" STREQUAL "")
    file(WRITE "${CMAKE_BINARY_DIR}/Directory.Build.targets"
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Project xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
  <!-- Prepend the CppWinRT include path that matches BabylonReactNative.vcxproj.
       This ItemDefinitionGroup runs after all NuGet .props (including any auto-restored
       Microsoft.Windows.CppWinRT), so it wins the include search order. -->
  <ItemDefinitionGroup>
    <ClCompile>
      <AdditionalIncludeDirectories>${CPPWINRT_INCLUDE_PATH};%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
    </ClCompile>
  </ItemDefinitionGroup>
</Project>
")
    message("-- CppWinRT: Generated Directory.Build.targets in ${CMAKE_BINARY_DIR} with path: ${CPPWINRT_INCLUDE_PATH}")
else()
    message(WARNING "CppWinRT: CPPWINRT_INCLUDE_PATH not set or empty ('${CPPWINRT_INCLUDE_PATH}'). "
        "cmake-built targets may use a different CppWinRT version than the vcxproj, "
        "causing a C++/WinRT version detect_mismatch link error.")
endif()

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

