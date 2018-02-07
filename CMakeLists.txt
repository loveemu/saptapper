cmake_minimum_required(VERSION 2.8)

project(saptapper C CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VERSION "1.2.0")

if(MSVC)
    if(CMAKE_CL_64)
        set(MSVC_ARCHITECTURE_NAME x64)
    else()
        set(MSVC_ARCHITECTURE_NAME win32)
    endif()

    # Use bundled dependencies
    set(ZLIB_ROOT ${CMAKE_SOURCE_DIR}/dependencies/zlib)
    set(ZLIB_LIBRARY ${CMAKE_SOURCE_DIR}/dependencies/zlib/lib/${MSVC_ARCHITECTURE_NAME}/zlibstat.lib)
    add_definitions(-DZLIB_WINAPI)
endif()

find_package(ZLIB REQUIRED)

if(MSVC)
    option(STATIC_CRT "Use static CRT libraries" ON)

    # Rewrite command line flags to use /MT if necessary
    if(STATIC_CRT)
        foreach(flag_var
                CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            if(${flag_var} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            endif(${flag_var} MATCHES "/MD")
        endforeach(flag_var)
    endif()

    # Allow for wildcards in command-line path arguments
    # http://msdn.microsoft.com/en-us/library/8bch7bkk.aspx
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} setargv.obj")

    # Disable MSVC specific secure error
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

#============================================================================
# saptapper
#============================================================================

set(SRCS
    src/saptapper.cpp
    src/BytePattern.cpp
    src/Mp2kDriver.cpp
    src/NatsumeDriver.cpp
    src/VgmDriverParam.cpp
)

add_executable(saptapper ${SRCS})

if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(saptapper ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)
