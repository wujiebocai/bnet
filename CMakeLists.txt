cmake_minimum_required(VERSION 3.21)
project(net)

# version
execute_process(
    OUTPUT_VARIABLE GIT_BRANCH
    COMMAND git symbolic-ref --short -q HEAD
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
#string(TIMESTAMP TODAY "%Y%m%d%H%I")
#set(BUILD_VERSION "${TODAY}")
#configure_file(
#	"${CMAKE_CURRENT_SOURCE_DIR}/version/version_config.h.in"
#	"${CMAKE_CURRENT_SOURCE_DIR}/version/version_config.h"
#)

set(CMAKE_CXX_STANDARD 20)

# build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
    #set(CMAKE_BUILD_TYPE "Debug")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# def
add_definitions(-DASIO_STANDALONE)
add_definitions(-DASIO_NO_DEPRECATED)
add_definitions(-DUSE_OPENSSL)
add_definitions(-DBNET_ENABLE_SSL)
add_definitions(-DBNET_ENABLE_HTTP)
#add_definitions(-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING)

# binary path
set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/__GEN)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/__GEN)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY})

# platform
if(UNIX)
    add_definitions(-DASIO_HAS_EPOLL)
    # option
    set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
    # cxx flag
    set(CMAKE_CXX_FLAGS "-std=c++20 -Wall -fPIC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -g")
    
    # openssl
    set(OPENSSL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/dep/openssl")
    set(OPENSSL_LIBRARY 
        "${PROJECT_SOURCE_DIR}/dep/lib/ubuntu/libssl.a"
        "${PROJECT_SOURCE_DIR}/dep/lib/ubuntu/libcrypto.a")
    # thread
    set(THREAD_LIBRARY pthread)
    # filesystem
    set(FILE_SYSTEM_LIBRARY stdc++fs)
	# 打开ASAN内存检测
    #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    #    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")
    #    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libasan")
	#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra")
    #    MESSAGE(STATUS "Open memory check")
else()
    # cxx flag
    if (MSVC_VERSION GREATER_EQUAL "1900")
        add_compile_options("/std:c++latest")
    else()
        message(FATAL_ERROR "Current msvc not support")
    endif()
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    # mt
    set(CompilerFlags
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL
    )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
    # complie options
    add_compile_options(/MP)
    add_compile_options(-bigobj)
    # def
    add_definitions(-D_WIN32_WINDOWS)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
    add_definitions(-DASIO_HAS_IOCP)
    # openssl
    set(OPENSSL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/3rd/openssl")
    set(OPENSSL_LIBRARY 
	    "Crypt32.lib"
        "${PROJECT_SOURCE_DIR}/dep/lib/win/x64/libcrypto.lib"
        "${PROJECT_SOURCE_DIR}/dep/lib/win/x64/libssl.lib")
    # other
    set(THREAD_LIBRARY)
    set(FILE_SYSTEM_LIBRARY)
endif()

# include
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dep")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dep/asio/asio/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dep/openssl/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dep/misc")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/bnet")               

# sub
add_subdirectory(test)
