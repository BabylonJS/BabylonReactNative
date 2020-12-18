# cmake 3.15.3+ because of a c++ regression when swift is enable
cmake_minimum_required(VERSION 3.15.3)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(BabylonNative)
set(BABYLON_NATIVE_PLATFORM "UWP")
set(CMAKE_SYSTEM_NAME "WindowsStore")
set(CMAKE_SYSTEM_VERSION "10.0")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BABYLON_NATIVE_DIR "${CMAKE_CURRENT_LIST_DIR}/../submodules/BabylonNative")

# Configure Babylon Native to use JSI
set(NAPI_JAVASCRIPT_ENGINE "JSI" CACHE STRING "The JavaScript engine to power N-API")
set(REACTNATIVE_DIR_CMAKE "${CMAKE_CURRENT_LIST_DIR}/../node_modules/react-native")
add_subdirectory("${REACTNATIVE_DIR_CMAKE}/ReactCommon/jsi/jsi" ${CMAKE_CURRENT_BINARY_DIR}/jsi)
target_include_directories(jsi INTERFACE ${REACTNATIVE_DIR_CMAKE}/ReactCommon/jsi)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../submodules/BabylonNative/Dependencies" ${CMAKE_CURRENT_BINARY_DIR}/Dependencies EXCLUDE_FROM_ALL)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../submodules/BabylonNative/Core" ${CMAKE_CURRENT_BINARY_DIR}/Core EXCLUDE_FROM_ALL)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../submodules/BabylonNative/Plugins" ${CMAKE_CURRENT_BINARY_DIR}/Plugins EXCLUDE_FROM_ALL)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../submodules/BabylonNative/Polyfills" ${CMAKE_CURRENT_BINARY_DIR}/Polyfills EXCLUDE_FROM_ALL)

if(DEFINED EXTENSIONS_DIRS)
    foreach(extensionDir IN LISTS EXTENSIONS_DIRS)
        add_subdirectory(${extensionDir} "Extensions" EXCLUDE_FROM_ALL)
    endforeach()
endif()
