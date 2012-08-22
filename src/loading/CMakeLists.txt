include_directories ("${PROJECT_SOURCE_DIR}/include")
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

# non-optional sources
set(LOADING_LIB_SOURCES  FileManager.cpp LoadingMain.cpp OBJModelLoader.cpp MD5ModelLoader.cpp MD5AnimationLoader.cpp)

#we ask user what parts does he want to be built
option(BUILD_LOADING_WITH_IL "Building with DevIL library will allow loding images from lots of formats. This option must be ON if you are planning to load models or images using hydra_loading library. You'll need DevIL library installed to build with this option." TRUE)

if(BUILD_LOADING_WITH_IL)
    find_package(DevIL REQUIRED)
    message(STATUS "We do build hydra_loading with DevIL.")
    add_definitions(-DBUILD_LOADING_WITH_IL)
    # add optional sources
    list(APPEND LOADING_LIB_SOURCES ILImageLoader.cpp)
    include_directories(${IL_INCLUDE_DIR})
endif(BUILD_LOADING_WITH_IL)

option(BUILD_LOADING_WITH_OGG "Building with OggVorbis will allow loading sound tracks from ogg format. You'll need libvorbis, libogg and libvorbisfile libraries installed to build with this option." TRUE)

if(BUILD_LOADING_WITH_OGG)
    find_package(OggVorbis REQUIRED)
    message(STATUS "We do build hydra_loading with OggVorbis.")
    add_definitions(-DBUILD_LOADING_WITH_OGG)
    # add optional sources
    list(APPEND LOADING_LIB_SOURCES OGGSoundTrackLoader.cpp)
    include_directories(${VORBIS_INCLUDE_DIR})
endif(BUILD_LOADING_WITH_OGG)

option(BUILD_LOADING_WITH_ZIP "Building with Zip will allow loading data from zip archives. You'll need libzip installed in your build system." TRUE)

if(BUILD_LOADING_WITH_ZIP)
    find_package(LibZip REQUIRED)
    find_package(ZLIB REQUIRED)
    message(STATUS "We do build hydra_loading with Zip.")
    add_definitions(-DBUILD_LOADING_WITH_ZIP)
    # add optional sources
    list(APPEND LOADING_LIB_SOURCES ZipFileManager.cpp)
    include_directories(${LIBZIP_INCLUDE_DIR})
endif(BUILD_LOADING_WITH_ZIP)

#we can add option of disabling FileManager here

#set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS filesystem system)


if(NOT LOADING_LIB_BUILD_TYPE) #if no specified value
    if(BUILD_LIBS_STATICALLY) #if we build all libs statically
        set(LOADING_LIB_BUILD_TYPE STATIC)
    else(BUILD_LIBS_STATICALLY) 
        if(CMAKE_COMPILER_IS_GNUCXX) #we can build shared libs with gcc
            set(LOADING_LIB_BUILD_TYPE SHARED)
        else(CMAKE_COMPILER_IS_GNUCXX)
            set(LOADING_LIB_BUILD_TYPE STATIC)
        endif(CMAKE_COMPILER_IS_GNUCXX)
    endif(BUILD_LIBS_STATICALLY)
endif()

add_library(hydra_loading ${LOADING_LIB_BUILD_TYPE} ${LOADING_LIB_SOURCES})

# non-optional dependencies
set(LINK_DEPENDENCIES ${Boost_LIBRARIES} hydra_math hydra_data)

#add optional linkage dependencies
if(BUILD_LOADING_WITH_IL)
    list(APPEND LINK_DEPENDENCIES ${IL_LIBRARIES})
endif()

if(BUILD_LOADING_WITH_OGG)
    list(APPEND LINK_DEPENDENCIES ${OGGVORBIS_LIBRARIES})
endif()

if(BUILD_LOADING_WITH_ZIP)
    list(APPEND LINK_DEPENDENCIES ${LIBZIP_LIBRARY})
    list(APPEND LINK_DEPENDENCIES ${ZLIB_LIBRARY})
endif()

target_link_libraries(hydra_loading ${LINK_DEPENDENCIES})
