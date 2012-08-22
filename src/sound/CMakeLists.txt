include_directories ("${PROJECT_SOURCE_DIR}/include")

# non-optional sources
set(SOUND_LIB_SOURCES DummySound.cpp)

option(BUILD_SOUND_WITH_OPENAL "OpenAL Sound renderer implementation. If you want to be able to use this sound renderer choose TRUE. I'll need OpenAL installed in your system." TRUE)

if(BUILD_SOUND_WITH_OPENAL)
    find_package(OpenAL REQUIRED)
    message(STATUS "We do build sound with OpenAL implementation.")
    include_directories (${OPENAL_INCLUDE_DIR})
    add_definitions(-DBUILD_SOUND_WITH_OPENAL)
    #add optional sources
    list(APPEND SOUND_LIB_SOURCES OpenALSound.cpp)
endif()


if(NOT SOUND_LIB_BUILD_TYPE) #if no specified value
    if(BUILD_LIBS_STATICALLY) #if we build all libs statically
        set(SOUND_LIB_BUILD_TYPE STATIC)
    else(BUILD_LIBS_STATICALLY) 
        if(CMAKE_COMPILER_IS_GNUCXX) #we can build shared libs with gcc
            set(SOUND_LIB_BUILD_TYPE SHARED)
        else(CMAKE_COMPILER_IS_GNUCXX)
            set(SOUND_LIB_BUILD_TYPE STATIC)
        endif(CMAKE_COMPILER_IS_GNUCXX)
    endif(BUILD_LIBS_STATICALLY)
endif()

add_library(hydra_sound ${SOUND_LIB_BUILD_TYPE} ${SOUND_LIB_SOURCES})

# non-optional dependecies
set(LINK_DEPENDENCIES hydra_data)

#optional
if(BUILD_SOUND_WITH_OPENAL)
    list(APPEND LINK_DEPENDENCIES ${OPENAL_LIBRARY})
endif()

target_link_libraries(hydra_sound ${LINK_DEPENDENCIES})
