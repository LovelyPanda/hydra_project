include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TRI_STRIPPER_SOURCES  connectivity_graph.cpp policy.cpp tri_stripper.cpp)

#we always build tri stripper statically
add_library(tri_stripper STATIC ${TRI_STRIPPER_SOURCES})
