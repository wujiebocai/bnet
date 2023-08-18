# build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Set build type to Debug")
else()
	set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Set build type to ${CMAKE_BUILD_TYPE}")
endif()

# cxx flag
if (MSVC_VERSION GREATER_EQUAL "1900")
    add_compile_options("/std:c++latest")
else()
    message(FATAL_ERROR "Current msvc not support")
endif()

# binary path
set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/__GEN/windows/${CMAKE_BUILD_TYPE})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/__GEN/windows/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${LIBRARY_OUTPUT_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${LIBRARY_OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${LIBRARY_OUTPUT_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_OUTPUT_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${LIBRARY_OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${LIBRARY_OUTPUT_PATH})
set(GENERATE_PATH ${RUNTIME_OUTPUT_DIRECTORY})
set(BIN_PATH ${PROJECT_SOURCE_DIR}/bin/windows)
set(BIN_SUFFIX ".exe")
set(DLL_SUFFIX ".dll")

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
    "${PROJECT_SOURCE_DIR}/3rd/lib/win/x64/libcrypto.lib"
    "${PROJECT_SOURCE_DIR}/3rd/lib/win/x64/libssl.lib")
#configure_file(${PROJECT_SOURCE_DIR}/thirds/openssl/windows/lib/libeay32.dll ${BIN_PATH}/libeay32.dll COPYONLY)
#configure_file(${PROJECT_SOURCE_DIR}/thirds/openssl/windows/lib/ssleay32.dll ${BIN_PATH}/ssleay32.dll COPYONLY)

# zlib
#set(ZLIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirds/zlib/windows/include")
#set(ZLIB_LIBRARIES "${PROJECT_SOURCE_DIR}/thirds/zlib/windows/lib/zlib.lib")
#configure_file(${PROJECT_SOURCE_DIR}/thirds/zlib/windows/lib/zlib1.dll ${BIN_PATH}/zlib1.dll COPYONLY)

# mysql
set(MYSQL_INCLUDE_DIR "D:/vm_sharefile/mysql-5.7.25-winx64/include")
#set(MYSQL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirds/mysql/windows/include")
#set(MYSQL_LIBRARY "${PROJECT_SOURCE_DIR}/thirds/mysql/windows/lib/libmysql.lib")
#configure_file(${PROJECT_SOURCE_DIR}/thirds/mysql/windows/lib/libmysql.dll ${BIN_PATH}/libmysql.dll COPYONLY)

# other
set(THREAD_LIBRARY)
set(FILE_SYSTEM_LIBRARY)

# warning
add_definitions(
    /wd4018 # 'expression' : signed/unsigned mismatch
    /wd4065 # switch statement contains 'default' but no 'case' labels
    /wd4146 # unary minus operator applied to unsigned type, result still unsigned
    /wd4244 # 'conversion' conversion from 'type1' to 'type2', possible loss of data
    /wd4251 # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
    /wd4267 # 'var' : conversion from 'size_t' to 'type', possible loss of data
    /wd4305 # 'identifier' : truncation from 'type1' to 'type2'
    /wd4307 # 'operator' : integral constant overflow
    /wd4309 # 'conversion' : truncation of constant value
    /wd4334 # 'operator' : result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
    /wd4355 # 'this' : used in base member initializer list
    /wd4506 # no definition for inline function 'function'
    /wd4800 # 'type' : forcing value to bool 'true' or 'false' (performance warning)
    /wd4996 # The compiler encountered a deprecated declaration.
)