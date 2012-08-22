include_directories ("${PROJECT_SOURCE_DIR}/include")

set(DATA_LIB_SOURCES  Image.cpp Material.cpp Model.cpp Mesh.cpp Vertex.cpp SoundTrack.cpp ChunkedTerrain.cpp)

if(NOT DATA_LIB_BUILD_TYPE) #if no specified value
    if(BUILD_LIBS_STATICALLY) #if we build all libs statically
        set(DATA_LIB_BUILD_TYPE STATIC)
    else(BUILD_LIBS_STATICALLY) 
        if(CMAKE_COMPILER_IS_GNUCXX) #we can build shared libs with gcc
            set(DATA_LIB_BUILD_TYPE SHARED)
        else(CMAKE_COMPILER_IS_GNUCXX)
            set(DATA_LIB_BUILD_TYPE STATIC)
        endif(CMAKE_COMPILER_IS_GNUCXX)
    endif(BUILD_LIBS_STATICALLY)
endif()

add_library(hydra_data ${DATA_LIB_BUILD_TYPE} ${DATA_LIB_SOURCES})

target_link_libraries(hydra_data hydra_math)
