include(FetchContent)

FetchContent_Declare(BabylonNative
    GIT_REPOSITORY https://github.com/BabylonJS/BabylonNative.git
    GIT_TAG 6ddb0cc1e87451edd7b12b35f524e9be319f3cfc)
FetchContent_MakeAvailable(BabylonNative)

set(SHARED_INCLUDES
    "${CMAKE_CURRENT_LIST_DIR}")

set(SHARED_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/DispatchFunction.h"
    "${CMAKE_CURRENT_LIST_DIR}/BabylonNative.h"
    "${CMAKE_CURRENT_LIST_DIR}/BabylonNative.cpp")

if (${BASEKIT_BUILD})
    set(BABYLON_NATIVE_PLUGIN_NATIVEXR OFF CACHE BOOL "")
    set(BABYLON_NATIVE_PLUGIN_NATIVECAMERA OFF CACHE BOOL "")
endif()
