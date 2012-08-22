include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(PUGI_XML_SOURCES pugixml.cpp pugixpath.cpp)

#we always build pugixml statically
add_library(pugixml STATIC ${PUGI_XML_SOURCES})
