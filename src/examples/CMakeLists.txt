set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#build simple loader (console app)
add_executable(SimpleLoader SimpleLoader.cpp)

target_link_libraries(SimpleLoader hydra_loading hydra_data hydra_math hydra_rendering) 

#build sound player
if(BUILD_SOUND_WITH_OPENAL)
    add_executable(SoundPlayer SoundPlayer.cpp)
    target_link_libraries(SoundPlayer hydra_loading hydra_data hydra_sound)
endif()


#build zip loader
if(BUILD_LOADING_WITH_ZIP)
    add_executable(ZipModelLoader ZipModelLoader.cpp)
    target_link_libraries(ZipModelLoader hydra_loading hydra_data hydra_math hydra_rendering)
endif()


#we need some additional libraries for other examples

# we need SDL
find_package (SDL)
# Workaround for the non-working REQUIRED flag
if (NOT SDL_FOUND)
   message (STATUS "SDL not found!")
else ()
   include_directories(${SDL_INCLUDE_DIR})
endif()

find_package (OpenGL)
find_package (GLEW)
if (NOT GLEW_FOUND)
    message (STATUS "GLEW not found!")
else()
    include_directories(${GLEW_INCLUDE_DIR})
endif()

if(SDL_FOUND AND OPENGL_FOUND AND GLEW_FOUND)
    add_executable (ModelViewer ModelViewer.cpp)
    target_link_libraries (ModelViewer
        ${GLEW_LIBRARIES}
        ${OpenGL_LIBRARY}
        ${SDL_LIBRARY}
        hydra_loading
        hydra_rendering
        hydra_math
        hydra_data
    )

    add_executable (NewModelViewer NewModelViewer.cpp)
    target_link_libraries (NewModelViewer
        ${GLEW_LIBRARIES}
        ${OpenGL_LIBRARY}
        hydra_loading
        hydra_rendering
        hydra_math
        hydra_data
        hydra_system
    )

#CGModelView uses CG wrapper headers, so he needs CG_INCLUDE_DIRS
if(BUILD_RENDERING_WITH_CG)
    find_package(Cg REQUIRED)
    include_directories(${CG_INCLUDE_DIRS})

    add_executable (CGModelViewer CGModelViewer.cpp)
    target_link_libraries (CGModelViewer
        ${GLEW_LIBRARIES}
        ${OpenGL_LIBRARY}
        ${CG_LIBRARIES}
        hydra_loading
        hydra_rendering
        hydra_math
        hydra_data
        hydra_system
    )
endif()

#set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS thread system)

if(WIN32)
    set(ADDITIONAL_WINSOCK_DEPENDENCY wsock32)
endif(WIN32)

add_executable (Landscaper Landscaper.cpp MyTerrainRAMLoadStrategy.cpp MyTerrainVRAMLoadStrategy.cpp)
    target_link_libraries (Landscaper
        ${GLEW_LIBRARIES}
        ${OpenGL_LIBRARY}
        hydra_loading
        hydra_rendering
        hydra_math
        hydra_data
        hydra_system
        ${Boost_LIBRARIES}
        #pthread
        ${ADDITIONAL_WINSOCK_DEPENDENCY}
        pugixml
    )

    #set(Boost_USE_STATIC_LIBS   ON)
    set(Boost_USE_MULTITHREADED ON)
    find_package(Boost COMPONENTS program_options thread)

    include_directories(${PUGI_XML_INCLUDE_DIR})
    add_executable (LandscapePreprocessor LandscapePreprocessor.cpp)
    target_link_libraries (LandscapePreprocessor
        hydra_loading
        hydra_rendering
        hydra_math
        hydra_data
        ${Boost_LIBRARIES}
        #pthread
        pugixml
    )

    add_executable (AnimationViewer AnimationViewer.cpp)
    target_link_libraries (AnimationViewer
        ${GLEW_LIBRARIES}
        ${OpenGL_LIBRARY}
        ${SDL_LIBRARY}
        hydra_loading
        hydra_rendering
        hydra_math
        hydra_data
    )

else ()

    message(STATUS "Some of needed for example applications libraries havn't been found. So we don't build some of example applications.")

endif ()
