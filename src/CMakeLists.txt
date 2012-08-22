include_directories ("${PROJECT_SOURCE_DIR}/include")
include_directories ("${PROJECT_SOURCE_DIR}/include/3rd_party")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

#we need boost headers for any type of build
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

#we define some variables to contain path to headers of 3rd-party libraries
#which are present in hydra source. (they all are placed in src/3rd_party at the moment)
#tri stripper headers
set(TRI_STRIPPER_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/3rd_party/tri_stripper")
#pugixml headers
set(PUGI_XML_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/3rd_party/pugixml")
#you should add other 3rd-party libraries here...


#user must choose what parts of hydra does he want to build
message(STATUS "===========================================")
message(STATUS "Please choose components you want to build. 
Remember that the component WILL be built if some other components
depends on it and they are to be built. To see linkage dependencies 
<http://hydraproject.org.ua/pix/linkage_dependency_diagram.png>. 
Most of external dependencies are optional, so you are able to 
drop them with loss of some features.")
message(STATUS "===========================================")

option(BUILD_SYSTEM "Hydra_System library is a handy wrapper over some system-dependent routines like window creation and input handling." TRUE)
if(BUILD_SYSTEM)
    message(STATUS "We do build hydra_system.")
    add_subdirectory (system)
endif()

option(BUILD_LOADING "Hydra_Loading library is able to import data from different external sources. For example it can load images and 3D models." TRUE)
if(BUILD_LOADING)
    message(STATUS "We do build hydra_loading.")
    add_subdirectory (loading)
endif()

# math lib is not optional now
message(STATUS "We do build hydra_math")
add_subdirectory (math)

option(BUILD_RENDERING "Rendering library contains some routines that deal with rendering. For example: MeshOptimizer, Renderer, etc." TRUE)
if(BUILD_RENDERING OR BUILD_SYSTEM)
    message(STATUS "We do build hydra_rendering.")
    add_subdirectory (rendering)
    
    #rendering needs tri_stripper. We shuld build it too.
    add_subdirectory (3rd_party/tri_stripper)
endif()

option(BUILD_SOUND "Sound library is like hydra_rendering but for sound. It contains some inplementation of sound rendering routines." TRUE)
if(BUILD_SOUND OR BUILD_SYSTEM)
    message(STATUS "We do build hydra_sound.")
    add_subdirectory (sound)
endif()

option(BUILD_DATA "Hydra_Data library is a set of data structures for storing some data in source-independent way. Very important library." TRUE)
if(BUILD_DATA OR BUILD_RENDERING OR BUILD_SYSTEM OR BUILD_LOADING OR BUILD_SOUND)
    message(STATUS "We do build hydra_data.")
    add_subdirectory (data)
endif()

# we don't build hydra-common lib as there is nothing to build by now
# but it can be changed in future
# add_subdirectory (common)

#build some example applications

option(BUILD_EXAMPLES "Example application are simple programs that use some hydra libraries. You can check your build of hydra using them." TRUE)
if(BUILD_EXAMPLES)
    message(STATUS "We do build examples")
    add_subdirectory (examples)

    #we need pugixml
    add_subdirectory(3rd_party/pugixml)
endif()

option(BUILD_TESTS "Test applications to test Hydra libraries." TRUE)
if(BUILD_TESTS)
    message(STATUS "We do build tests")
    add_subdirectory (tests)
endif()

