cmake_minimum_required(VERSION 3.15.3)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(ReactNativeBabylon)

set(CMAKE_CXX_STANDARD 17)

set(BABYLON_REACT_NATIVE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../react-native")
set(BABYLON_REACT_NATIVE_IOSANDROID "${CMAKE_CURRENT_LIST_DIR}/../../react-native-iosandroid")

include(${BABYLON_REACT_NATIVE_DIR}/shared/CMakeLists.txt)

# Configure Babylon Native to use JSI
# Note: We should avoid installing node_modules in the Module\@babylonjs\react-native folder.
# Installing react-native dependencies for both the Playground app and the @babylonjs\react-native package will generate a bad bundle/runtime errors.
set(NAPI_JAVASCRIPT_ENGINE "JSI" CACHE STRING "")
get_filename_component(REACTNATIVE_DIR_CMAKE "${CMAKE_CURRENT_LIST_DIR}/../../../../Apps/BRNPlayground/node_modules/react-native" ABSOLUTE)
get_filename_component(REACTNATIVE_WINDOWS_DIR_CMAKE "${CMAKE_CURRENT_LIST_DIR}/../../../../Apps/BRNPlayground/node_modules/react-native-windows/PropertySheets/External" ABSOLUTE)
add_subdirectory("${REACTNATIVE_DIR_CMAKE}/ReactCommon/jsi/jsi" ${CMAKE_CURRENT_BINARY_DIR}/jsi)
target_include_directories(jsi INTERFACE ${REACTNATIVE_DIR_CMAKE}/ReactCommon/jsi)

set(BABYLON_NATIVE_BUILD_APPS OFF CACHE BOOL "")
set(BABYLON_NATIVE_USE_SWAPCHAINPANEL ON CACHE BOOL "")
add_subdirectory(${babylonnative_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/BabylonNative EXCLUDE_FROM_ALL)
# Disable Unity build for UrlLib because of conflict in header between windows.h and winrt
set_property(TARGET UrlLib PROPERTY UNITY_BUILD false)

add_library(BabylonNative
    ${SHARED_SOURCES})

if (${BASEKIT_BUILD})
    target_compile_definitions(BabylonNative PRIVATE BASEKIT_BUILD=1)
else()
    set(ADDITIONAL_LIBRARIES NativeXr NativeCamera)
endif()

target_include_directories(BabylonNative PRIVATE ${SHARED_INCLUDES})

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
    XMLHttpRequest
    Canvas
    ${ADDITIONAL_LIBRARIES})

set_target_properties(BabylonNative PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(arcana PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(GraphicsDevice PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(jsi PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(JsRuntime PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(NativeCapture PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(NativeEngine PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(NativeInput PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(NativeOptimizations PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(Window PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(XMLHttpRequest PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(Canvas PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(NativeXr PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
set_target_properties(NativeCamera PROPERTIES VS_PROJECT_IMPORT "${REACTNATIVE_WINDOWS_DIR_CMAKE}/Microsoft.ReactNative.Cpp.Dependencies.props")
