cmake_minimum_required(VERSION 2.8)

project(util C CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    option(STATIC_CRT "Use static CRT libraries" OFF)

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
endif()


#============================================================================
# bin2c
#============================================================================

set(BIN2C_SRCS
    bin2c/bin2c.cpp
)

add_executable(bin2c ${BIN2C_SRCS})

#============================================================================
# bincrop
#============================================================================

set(BINCROP_SRCS
    bincrop/bincrop.cpp
)

add_executable(bincrop ${BINCROP_SRCS})
