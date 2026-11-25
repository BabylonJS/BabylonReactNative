include(FetchContent)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/BabylonNative)

# Babylon Native builds with -fno-rtti, which must be undone for embedding in React Native:
# - React Native's JSI headers use dynamic_cast, including from templates instantiated
#   by Babylon Native targets.
# - Without RTTI, clang emits hidden local copies of the standard exception type_info
#   (std::exception, std::invalid_argument, ...). When the static libraries are linked
#   into the host app (iOS), catch clauses in app code can bind to those copies and fail
#   to match exceptions thrown by libc++, terminating the process.
function(enable_babylon_native_rtti directory)
    get_property(targets DIRECTORY "${directory}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS targets)
        get_target_property(target_type ${target} TYPE)
        get_target_property(imported ${target} IMPORTED)
        if(NOT imported AND target_type MATCHES "^(STATIC_LIBRARY|SHARED_LIBRARY|MODULE_LIBRARY|OBJECT_LIBRARY|EXECUTABLE)$")
            target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-frtti>)
        endif()
    endforeach()

    get_property(subdirectories DIRECTORY "${directory}" PROPERTY SUBDIRECTORIES)
    foreach(subdirectory IN LISTS subdirectories)
        enable_babylon_native_rtti("${subdirectory}")
    endforeach()
endfunction()

set(SHARED_INCLUDES
    "${CMAKE_CURRENT_LIST_DIR}" PARENT_SCOPE)

set(SHARED_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/DispatchFunction.h"
    "${CMAKE_CURRENT_LIST_DIR}/BabylonNative.h"
    "${CMAKE_CURRENT_LIST_DIR}/BabylonNative.cpp" PARENT_SCOPE)

if (${BABYLON_NATIVE_PLUGIN_NATIVECAMERA})
    list(APPEND ADDITIONAL_LIBRARIES NativeCamera)
endif()
if (${BABYLON_NATIVE_PLUGIN_NATIVEXR})
    list(APPEND ADDITIONAL_LIBRARIES NativeXr)
endif()

set(ADDITIONAL_LIBRARIES "${ADDITIONAL_LIBRARIES}" PARENT_SCOPE)