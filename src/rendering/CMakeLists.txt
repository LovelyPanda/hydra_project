include_directories ("${PROJECT_SOURCE_DIR}/include")

#add include for tri_stripper
include_directories (${TRI_STRIPPER_INCLUDE_DIR})

#non-optional sources
set(RENDERING_LIB_SOURCES Camera.cpp MeshOptimizer.cpp TerrainOptimizer.cpp TerrainLODManager.cpp TerrainPreprocessor.cpp)

option(BUILD_RENDERING_WITH_OPENGL "If you want to render with OpenGL implenetation of Renderer choose TRUE." TRUE)
if(BUILD_RENDERING_WITH_OPENGL)
    message(STATUS "We do build with OpenGL renderer")
    add_definitions(-DBUILD_RENDERING_WITH_OPENGL)
    find_package(OpenGL REQUIRED)
    list(APPEND RENDERING_LIB_SOURCES OpenGLRenderer.cpp)
    include_directories(${OPENGL_INCLUDE_DIR})
endif()

option(BUILD_RENDERING_WITH_CG "If you want to build Cg wrapper choose TRUE." TRUE)
if(BUILD_RENDERING_WITH_CG)
    message(STATUS "We do build with Cg")
    add_definitions(-DBUILD_RENDERING_WITH_CG)
    find_package(Cg REQUIRED)
    list(APPEND RENDERING_LIB_SOURCES CGProgram.cpp CGContext.cpp CGParameter.cpp)
    include_directories(${CG_INCLUDE_DIRS})
endif()

if(NOT RENDERING_LIB_BUILD_TYPE) #if no specified value
    if(BUILD_LIBS_STATICALLY) #if we build all libs statically
        set(RENDERING_LIB_BUILD_TYPE STATIC)
    else(BUILD_LIBS_STATICALLY) 
        if(CMAKE_COMPILER_IS_GNUCXX) #we can build shared libs with gcc
            set(RENDERING_LIB_BUILD_TYPE SHARED)
        else(CMAKE_COMPILER_IS_GNUCXX)
            set(RENDERING_LIB_BUILD_TYPE STATIC)
        endif(CMAKE_COMPILER_IS_GNUCXX)
    endif(BUILD_LIBS_STATICALLY)
endif()

add_library(hydra_rendering ${RENDERING_LIB_BUILD_TYPE} ${RENDERING_LIB_SOURCES})

#non-optional dependencies
set(LINK_DEPENDENCIES hydra_math hydra_data tri_stripper)

#optional
if(BUILD_RENDERING_WITH_OPENGL)
    list(APPEND LINK_DEPENDENCIES ${OPENGL_LIBRARIES})
endif()

if(BUILD_RENDERING_WITH_CG)
    list(APPEND LINK_DEPENDENCIES ${CG_LIBRARIES})
endif()


target_link_libraries(hydra_rendering ${LINK_DEPENDENCIES})
