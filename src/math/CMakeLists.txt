include_directories ("${PROJECT_SOURCE_DIR}/include")

set(MATH_LIB_SOURCES Vector3D.cpp Quat.cpp Point.cpp Matrix.cpp AABB.cpp)

if(NOT MATH_LIB_BUILD_TYPE) #if no specified value
    if(BUILD_LIBS_STATICALLY) #if we build all libs statically
        set(MATH_LIB_BUILD_TYPE STATIC)
    else(BUILD_LIBS_STATICALLY)  
        if(CMAKE_COMPILER_IS_GNUCXX) #we can build shared libs with gcc
            set(MATH_LIB_BUILD_TYPE SHARED)
        else(CMAKE_COMPILER_IS_GNUCXX)
            set(MATH_LIB_BUILD_TYPE STATIC)
        endif(CMAKE_COMPILER_IS_GNUCXX)
    endif(BUILD_LIBS_STATICALLY)
endif()

add_library(hydra_math ${MATH_LIB_BUILD_TYPE} ${MATH_LIB_SOURCES})
