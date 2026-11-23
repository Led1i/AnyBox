cmake_minimum_required(VERSION 3.25)

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

project(AnyBox VERSION 0.1.0 LANGUAGES C CXX)

if(NOT WIN32)
    message(FATAL_ERROR "AnyBox requires Windows")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "AnyBox requires an x64 target")
endif()

add_library(anybox_options INTERFACE)
target_compile_features(anybox_options INTERFACE cxx_std_20)
if(MSVC)
    if(NOT CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "x64")
        message(FATAL_ERROR "AnyBox requires an x64 target")
    endif()
    target_compile_options(anybox_options INTERFACE /W4 /WX /permissive- /EHsc- /Zc:__cplusplus /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    execute_process(COMMAND "${CMAKE_CXX_COMPILER}" -dumpmachine OUTPUT_VARIABLE compilerTarget OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
    if(NOT compilerTarget MATCHES "^x86_64-")
        message(FATAL_ERROR "AnyBox requires an x64 target")
    endif()
    target_compile_options(anybox_options INTERFACE -Wall -Wextra -Wpedantic -Werror -fexceptions)
endif()
set(CMAKE_CXX_EXTENSIONS OFF)

set(ANYBOX_RUNTIME_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

include(CTest)
include(cmake/AnyBoxLibrary.cmake)

add_subdirectory(3rdparty)

add_subdirectory(core/common)
add_subdirectory(core/libs)
