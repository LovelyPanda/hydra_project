include_directories ("${PROJECT_SOURCE_DIR}/include")

#non-optional sources
set(SYSTEM_LIB_SOURCES)

option(BUILD_SYSTEM_WITH_SDL "If you want to build a wrapper inplementation using SDL choose TRUE. You'll need SDL installed in your system." TRUE)
if(BUILD_SYSTEM_WITH_SDL)
    message(STATUS "We do build hydra_system with SDL implementation.")
    add_definitions(-DBUILD_SYSTEM_WITH_SDL)
    find_package (SDL REQUIRED)
    list(APPEND SYSTEM_LIB_SOURCES SDLWindow.cpp SDLInput.cpp)
    include_directories(${SDL_INCLUDE_DIR})
endif()

option(BUILD_SYSTEM_WITH_WIN32 "If you need to build a wrapper implenetation using Win32 choose TRUE. Target system must be Windows in this case." FALSE)
if(BUILD_SYSTEM_WITH_WIN32)
    message(STATUS "We do build hydra_system with Win32 implementation.")
endif()

if(NOT SYSTEM_LIB_BUILD_TYPE) #if no specified value
    if(BUILD_LIBS_STATICALLY) #if we build all libs statically
        set(SYSTEM_LIB_BUILD_TYPE STATIC)
    else(BUILD_LIBS_STATICALLY) 
        if(CMAKE_COMPILER_IS_GNUCXX) #we can build shared libs with gcc
            set(SYSTEM_LIB_BUILD_TYPE SHARED)
        else(CMAKE_COMPILER_IS_GNUCXX)
            set(SYSTEM_LIB_BUILD_TYPE STATIC)
        endif(CMAKE_COMPILER_IS_GNUCXX)
    endif(BUILD_LIBS_STATICALLY)
endif()

add_library(hydra_system ${SYSTEM_LIB_BUILD_TYPE} ${SYSTEM_LIB_SOURCES})

#non-optional dependencies
set(LINK_DEPENDENCIES hydra_rendering hydra_sound)

if(BUILD_SYSTEM_WITH_SDL)
    list(APPEND LINK_DEPENDENCIES ${SDL_LIBRARY})
endif()

target_link_libraries(hydra_system ${LINK_DEPENDENCIES})
