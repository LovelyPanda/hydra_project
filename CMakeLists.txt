#CMAKE file for build

project (hydra)

cmake_minimum_required(VERSION 2.6)


if(CMAKE_BUILD_TYPE STREQUAL "Profile")
    if(CMAKE_COMPILER_IS_GNUCXX) #profiling under gcc
        set(PROFILE_FLAGS_GCC " -pg")
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}${PROFILE_FLAGS_GCC}) #add flag
    endif()
endif()

if(CMAKE_COMPILER_IS_GNUCXX) #under gcc
    set(WARNING_FLAGS " -Wall -pedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}${WARNING_FLAGS}") #all warnings
endif()

if(BUGGED_CG_GLSL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUGGED_CG_GLSL")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release) #default type is release
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
add_subdirectory(src)
