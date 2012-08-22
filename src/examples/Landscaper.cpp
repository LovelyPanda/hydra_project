//Landscaper.cpp

/*
 *   Copyright 2010-2011 Alexander Medvedev
 *
 *   This file is part of Hydra project.
 *   See <http://hydraproject.org.ua> for more info.
 *
 *   Hydra is free software: you can redistribute it and/or modify
 *   it under the terms of the MIT License:
 *   <http://www.opensource.org/licenses/mit-license.php>
 *   See notice at the end of this file.
 */


/**
 * Height map image file should be passed as the only console argument.
 *
 * You can move throw the scene using "W", "A", "S", "D" or arrows.
 * To stop/start rotation press "R"
 * To increase model's size (scale) twice, press "+"
 * To decrease press "-"
 * To show/hide normals press "N"
 * To enable/disable lighting press "L"
 * To switch between polygon modes (filled or lines) press "P"
 * To enable/disable "two-side" draw polygon mode press "O"
 */

#include "loading/LoadingMain.hpp"

#include <cmath>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <map>

#include <GL/glew.h>

//we don't need to include SDL.h as we don't need to use SDL functions directly
#include "system/SDLWindow.hpp"
#include "system/Input.hpp"
#include "system/Event.hpp"

#include <GL/gl.h>
#include <GL/glu.h>

#include "math/Vector3D.hpp"
#include "rendering/Camera.hpp"
#include "common/Timer.hpp"

#include "rendering/Renderer.hpp"

#include "data/Image.hpp"
#include "data/Mesh.hpp"

#include <boost/foreach.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include "rendering/TerrainLODManager.hpp"
#include "MyTerrainRAMLoadStrategy.hpp"
#include "MyTerrainVRAMLoadStrategy.hpp"

using namespace hydra::loading;
using namespace hydra::math;
using namespace hydra::rendering;
using namespace hydra::common;
using namespace hydra::data;
using namespace hydra::system;

#ifndef M_PI
    #define M_PI (3.14159265f)
#endif

//camera speed (points per second)
const float CAM_SPEED = 100.0f;

Camera gCam;

bool gIsClicked = false;
float gSens = 0.3f;

bool gLeftKeyPressed = false;
bool gRightKeyPressed = false;
bool gUpKeyPressed = false;
bool gDownKeyPressed = false;

bool gLinePolygonMode = false;
bool gDrawBothFaces = false;
bool gDrawNormals = false;
bool gLight = true;

float gLightPosition[]  = {200.0f, 500.0f, 0.0f, 1.0f};

ImagePtr gTexImage;
GLuint gTexture;

//ImagePtr gDetailImage;
//GLuint gDetailTexture;

float gZoom = 10;
bool gWorkFlag = true;
float* gCamMatrix = 0;

WindowPtr gWindow;
InputPtr gInput;

unsigned int gNumChunksRendered = 0;
unsigned int gNumTrianglesRendered = 0;

bool gResized = false;

const float kHorizontalFOV = 60.0f;

static void clean(){
    if(gCamMatrix) delete[] gCamMatrix;
}

//loads texture from specified Image
GLuint loadTexture(const ImagePtr inImg){
    GLuint tex;

    const unsigned char* data = inImg->getData();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &tex); 
    glBindTexture(GL_TEXTURE_2D, tex);
   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//--------------------------------------------------  

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, inImg->getWidth(), inImg->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, data);
   
    return tex;
}

// Setup default render states
static void setupOpenGL(){
    //first try to init extensions:
    if (glewInit() != GLEW_OK || !GLEW_ARB_multitexture || !GLEW_ARB_vertex_buffer_object){
        throw std::runtime_error("error loading OpenGL extensions using GLEW");
    }

    GLfloat clearColor[4] = {0.3f, 0.4f, 0.5f, 1.0f};
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glClearDepth(1.0f);

    if(gLinePolygonMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glEnable(GL_DEPTH_TEST);

    glFrontFace(GL_CCW);

    if(gLinePolygonMode || gDrawBothFaces){
        glDisable(GL_CULL_FACE);
    }
    else{
        glEnable(GL_CULL_FACE);
    }

    glViewport(0, 0, gWindow->getWidth(), gWindow->getHeight());    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(kHorizontalFOV, ((float)gWindow->getWidth()/(float)gWindow->getHeight()), 1.0f, 2000.0f);

    float ambientColor[]    = {0.3f, 0.3f, 0.3f, 1.0f};
    float diffuseColor[]    = {0.9f, 0.9f, 0.9f, 1.0f};
    float specularColor[]   = {0.9f, 0.9f, 0.9f, 1.0f};


    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
    glLightfv(GL_LIGHT0, GL_POSITION, gLightPosition);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glEnable(GL_LIGHT0);
    if(gLight)glEnable(GL_LIGHTING);


    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_NORMALIZE);
   
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
/*
    glActiveTextureARB(GL_TEXTURE1_ARB);
    //glEnable(GL_TEXTURE_2D);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
*/
    glActiveTextureARB(GL_TEXTURE0_ARB);

    //enable fog
    glEnable(GL_FOG);
   	glFogi(GL_FOG_MODE, GL_LINEAR);
   	glFogfv(GL_FOG_COLOR, clearColor);
   	glHint(GL_FOG_HINT, GL_NICEST);
   	glFogf(GL_FOG_START, 500.0);
   	glFogf(GL_FOG_END, 1400.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //drop old lists and textures(if any)
    //glDeleteBuffersARB(1, &gLandscape.VBO);
    //glDeleteBuffersARB(1, &gLandscape.IBO);
    glDeleteTextures(1, &gTexture);
    gTexture = loadTexture(gTexImage);
//    gDetailTexture = loadTexture(gDetailImage);

    //terrain's material
    float amb[4] = {0.4f, 0.4f, 0.4f, 1.0f};
    float dif[4] = {0.7f, 0.7f, 0.7f, 1.0f};
    float spec[4] = {0.9f, 0.9f, 0.9f, 1.0f};
    float emis[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT, GL_EMISSION, emis);

    float specExp = 50.0f;
    glMaterialf(GL_FRONT, GL_SHININESS, specExp);
}

//main render function
static void render(const TerrainLODManager::RenderChunkCont& chunkIds, MyTerrainVRAMLoadStrategy& VRAMLoader, const ChunkedTerrain& terrain, unsigned int terrainWidth, unsigned int terrainHeight, boost::mutex& inOpenGLMutex){

    unsigned int fragmentWidth = terrain.getFragmentWidth();
    unsigned int numFragmentsX = terrainWidth / (fragmentWidth - 1);
    unsigned int numFragmentsY = terrainHeight / (fragmentWidth - 1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    assert(gCamMatrix);
    gCam.getMatrix(gCamMatrix);
    glMultMatrixf(gCamMatrix);


/*  gluLookAt(gCam.getPos().x(), gCam.getPos().y(), gCam.getPos().z(), 
          gCam.getPos().x() + gCam.getDir().x(), 
          gCam.getPos().y() + gCam.getDir().y(),
          gCam.getPos().z() + gCam.getDir().z(), 
          gCam.getUp().x(), gCam.getUp().y(), gCam.getUp().z());
*/  

    gLightPosition[0] = gCam.getPos().x();
    gLightPosition[1] = gCam.getPos().y();
    gLightPosition[2] = gCam.getPos().z();

    glLightfv(GL_LIGHT0, GL_POSITION, gLightPosition);
    
    //glScalef(gZoom, gZoom, gZoom);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, gTexture);
/*
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, gDetailTexture);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(1.0f / (fragmentWidth - 1.0f), 1.0f / (fragmentWidth - 1.0f), 1.0f);
*/
    glActiveTextureARB(GL_TEXTURE0_ARB);

    glMatrixMode(GL_MODELVIEW);

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glEnableClientState(GL_VERTEX_ARRAY);
    glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glClientActiveTextureARB(GL_TEXTURE1_ARB);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glClientActiveTextureARB(GL_TEXTURE0_ARB);

    glEnableClientState(GL_NORMAL_ARRAY);

    size_t vertexStride = sizeof(Vertex);
    size_t coordOffset = offsetof(Vertex, mCoord);
    size_t texCoordOffset = offsetof(Vertex, mTexCoord);
    size_t normalOffset = offsetof(Vertex, mNormal);

    //render 
    #define STO_PITSOT 100500
    TerrainFragmentId lastFragment(STO_PITSOT, STO_PITSOT);
    #undef STO_PITSOT
    
    GLuint vhandle = 0;
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    for(TerrainLODManager::RenderChunkCont::const_iterator iter = chunkIds.begin(); iter != chunkIds.end(); ++iter){
        const TerrainFragment::ChunkData& chunkData = terrain.getFragment(iter->getFragmentId()).tree->getNode(iter->getQuadTreePos()).data;

        //TODO: frustum culling using chunkData.aabb (!)

        boost::lock_guard<boost::mutex> lock(inOpenGLMutex);

        GLuint oldVhandle = vhandle;

        try{
            //we should get vlod handle even in case when
            //we do not change fragment
            vhandle = VRAMLoader.getVertexBuffer(iter->getFragmentId(), chunkData.vertices);
        }
        catch(const std::runtime_error& err){
            std::cerr << "Exception got: " << err.what() << std::endl;
            continue;
        }

        //we may not change buffer if it has been activated before
        if(vhandle != oldVhandle){

            //move fragment to its place
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();

            glTranslatef(iter->getFragmentId().getX() / (float)numFragmentsX, iter->getFragmentId().getY() / (float)numFragmentsY, 0.0f);
            glScalef(1.0f / (float)numFragmentsX, 1.0f / (float)numFragmentsY, 1.0f);
            
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(iter->getFragmentId().getY() * (fragmentWidth - 1.0f), 0.0f, iter->getFragmentId().getX() * (fragmentWidth - 1.0f));

            glBindBufferARB(GL_ARRAY_BUFFER_ARB, vhandle);
        }

        if(vhandle == 0) continue;

        GLuint ihandle = VRAMLoader.getIndexBuffer(*iter);

        assert(glIsBufferARB(ihandle));
        assert(glIsBufferARB(vhandle));
        
        glClientActiveTextureARB(GL_TEXTURE0_ARB);
        glTexCoordPointer(2, GL_FLOAT, vertexStride, (void *)texCoordOffset);

        glClientActiveTextureARB(GL_TEXTURE1_ARB);
        glTexCoordPointer(2, GL_FLOAT, vertexStride, (void *)texCoordOffset);

        glClientActiveTextureARB(GL_TEXTURE0_ARB);

        glNormalPointer(GL_FLOAT, vertexStride, (void *)normalOffset);
        glVertexPointer(3, GL_FLOAT, vertexStride, (void*)0);

        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ihandle);
        glDrawElements(GL_TRIANGLES, chunkData.ptr->indices.size(), GL_UNSIGNED_SHORT, (void*)(coordOffset));

        ++gNumChunksRendered;
        gNumTrianglesRendered += (chunkData.ptr->indices.size() / 3);
    }
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
   
    {
        boost::lock_guard<boost::mutex> lock(inOpenGLMutex);

        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }

    glDisableClientState(GL_NORMAL_ARRAY);

    glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glClientActiveTextureARB(GL_TEXTURE1_ARB);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisableClientState(GL_VERTEX_ARRAY);
    glPopClientAttrib();
    
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, 0);
  
//    glActiveTextureARB(GL_TEXTURE1_ARB);
//    glBindTexture(GL_TEXTURE_2D, 0);

    //OpenGL error handling:
    GLenum GLErrorCode;
    const GLubyte* GLErrorString;
    while((GLErrorCode = glGetError()) != GL_NO_ERROR){
        GLErrorString = gluErrorString(GLErrorCode);
        std::cerr << "OpenGL error: " << GLErrorString << std::endl;
    }
}

static void handleEvents(){
    Event event;
    while((event = gInput->getNextEvent()).type != Event::EMPTY){
        switch(event.type){
            case Event::KEY_PRESS:
            switch(event.key){
                case Event::ESCAPE:
                    gWorkFlag = false;
                    break;
                case Event::UP:
                case Event::KEY_w:
                    gUpKeyPressed = true;
                    break;
                case Event::DOWN:
                case Event::KEY_s:
                    gDownKeyPressed = true;
                    break;
                case Event::LEFT:
                case Event::KEY_a:
                    gLeftKeyPressed = true;
                    break;
                case Event::RIGHT:
                case Event::KEY_d:
                    gRightKeyPressed = true;
                    break;
                case Event::PLUS:
                case Event::EQUALS:
                    gZoom *= 2.0f;
                    break;
                case Event::MINUS:
                case Event::UNDERSCORE:
                    gZoom /= 2.0f;
                    break;
                case Event::KEY_p:
                    gLinePolygonMode = !gLinePolygonMode;

                    if(gLinePolygonMode){
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        glDisable(GL_CULL_FACE);
                    }
                    else{
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                        if(!gDrawBothFaces) glEnable(GL_CULL_FACE);
                    }
                    break;
                case Event::KEY_o:
                    gDrawBothFaces = !gDrawBothFaces;
                    
                    if(gDrawBothFaces){
                        glDisable(GL_CULL_FACE);
                    }
                    else{
                        glEnable(GL_CULL_FACE);
                    }
                    break;
                case Event::KEY_l:
                    gLight = !gLight;
                    if(gLight) glEnable(GL_LIGHTING);
                    else glDisable(GL_LIGHTING);
                    break;
                case Event::KEY_n:
                    gDrawNormals = !gDrawNormals;
                    break;
                case Event::KEY_f:
                    if(gWindow->isFullscreen()){
                        gWindow->toWindowMode(640, 480);
                    }
                    else gWindow->toFullscreenMode(false);
                    gResized = true;
                    break;
                case Event::MOUSE_LEFT:
                case Event::MOUSE_RIGHT:
                    gIsClicked = true;
                    gInput->warpMouse(gWindow->getWidth()/2, gWindow->getHeight()/2);
                    break;
                default:
                    break;
            }
            break;
            case Event::KEY_RELEASE:
            switch(event.key){
                case Event::UP:
                case Event::KEY_w:
                    gUpKeyPressed = false;
                    break;
                case Event::DOWN:
                case Event::KEY_s:
                    gDownKeyPressed = false;
                    break;
                case Event::LEFT:
                case Event::KEY_a:
                    gLeftKeyPressed = false;
                    break;
                case Event::RIGHT:
                case Event::KEY_d:
                    gRightKeyPressed = false;
                    break;
                case Event::MOUSE_LEFT:
                case Event::MOUSE_RIGHT:
                    gIsClicked = false;
                    break;
                default:
                    break;
            }
            break;
        case Event::MOUSE_MOTION:
            if(gIsClicked){
                gCam.rotateX(M_PI * static_cast<float>(gWindow->getWidth() / 2.0f - event.x) * gSens / 180.0f);
                gCam.rotateY(M_PI * static_cast<float>(gWindow->getHeight() / 2.0f - event.y) * gSens / 180.0f);
                gInput->warpMouse(gWindow->getWidth()/2, gWindow->getHeight()/2);
            }
            break;
        case Event::ACTIVATE:
        case Event::DEACTIVATE:
            break;
        case Event::RESIZE:
            gWindow->resize(event.x, event.y);
            gResized = true; //this should be handled elsewhere
            break;
        case Event::QUIT:
            gWorkFlag = false;
            break;
        default:
            break;
        }
    }


}

void printHelp(){
}

int main (int argv, char** args){
    std::cout << "=========== started ===========" << std::endl;

    //init factories for loading stuff
    initFactories();

    try{
        if(argv > 2){
           gTexImage = loadFromFile<Image>(args[2]);
           //gDetailImage = loadFromFile<Image>(args[3]);
        }
        else{
            printHelp();
            std::cout << "Press ENTER to exit..." << std::endl;
            std::cin.get();
            return 1;
        }
    }
    catch(const std::runtime_error& err){
        std::cerr << err.what() << std::endl;
        printHelp();
        std::cout << "Press ENTER to exit..." << std::endl;
        std::cin.get(); 
        return 1;
    }
    std::cout << "loaded" << std::endl;
    //we don't need loaders any more
    dropFactories();

    boost::asio::io_service ioService;
    boost::asio::io_service::work ioServiceWork(ioService);
    boost::thread_group threads;

    const unsigned int NUM_THREADS = 3;
    //create pool of handlers
    for(unsigned int i = 0; i < NUM_THREADS; ++i)
        threads.create_thread(boost::bind(&boost::asio::io_service::run, &ioService));
    assert(NUM_THREADS == threads.size());

    //set video properties
    Window::Properties props;
    props.width = 640;
    props.height = 480;
    props.bpp = 0; //use default
    props.resizable = true;
    props.fullscreen = false;
    props.doublebuffered = true;
    props.cursorInvisibleFlag = true; //invisible
    props.caption = "Landscaper";

    //init SDL window
    gWindow = WindowPtr(new SDLWindow(props));
    gWindow->createRenderer("OpenGL");

    gInput = InputPtr(gWindow->getInput());

    gCam = Camera(Vector3D(200.0f, 400.0f, 0.0f), Vector3D(1.0f, 0.0f, 1.0f), Vector3D(0.0f, 1.0f, 0.0f));
    gCamMatrix = new float[16];
    
    try{
        setupOpenGL();
    }
    catch(const std::exception& exc){
        std::cerr << exc.what() << std::endl;
        clean();
        return 1;
    }


    ChunkedTerrainPtr chunkedTerrain(new ChunkedTerrain());
    //create load strategies
    MyTerrainRAMLoadStrategyPtr RAMStrategy(new MyTerrainRAMLoadStrategy(chunkedTerrain, ioService, args[1]));
    MyTerrainVRAMLoadStrategyPtr VRAMStrategy(new MyTerrainVRAMLoadStrategy(chunkedTerrain, ioService));

    boost::mutex& OpenGLMutex = VRAMStrategy->getOpenGLMutex();

    //fill parameters
    TerrainLODManager::Parameters parameters;
    parameters.perspectiveScallingFactor = calculatePerspectiveScallingFactor(gWindow->getWidth(), kHorizontalFOV / 180.0f * 3.14f);
    parameters.maxTolerableError = 5.0f;
    parameters.unloadFactor = 0.90f;
    parameters.VRAMLoadFactor = 0.90f;
    parameters.RAMLoadFactor = 0.75f;
    parameters.maxVisibleDistance = 2000.0f;

    //create manager
    TerrainLODManager terrainLODManager(parameters, chunkedTerrain, RAMStrategy, RAMStrategy, VRAMStrategy);


    Timer fpsTimer;
    Timer terrainTimer;
    Timer cameraTimer;

    cameraTimer.start();
    fpsTimer.start();
    terrainTimer.start();
    int frames = 0;

    //set position to the center of the terrain
    gCam.place(RAMStrategy->getTerrainWidth() / 2.0f, 50.0f, RAMStrategy->getTerrainHeight() / 2.0f);


    terrainLODManager.initiateUpdate(gCam);

    try{
        while(gWorkFlag){
            handleEvents();

            float timeElapsedSinceLastRedraw = cameraTimer.getSeconds();
            cameraTimer.start();

            if(gUpKeyPressed) gCam.move(CAM_SPEED * timeElapsedSinceLastRedraw, 0.0f, 0.0f);
            if(gDownKeyPressed) gCam.move(-CAM_SPEED * timeElapsedSinceLastRedraw, 0.0f, 0.0f);
            if(gRightKeyPressed) gCam.move(0.0f, 0.0f, CAM_SPEED * timeElapsedSinceLastRedraw);
            if(gLeftKeyPressed) gCam.move(0.0f, 0.0f, -CAM_SPEED * timeElapsedSinceLastRedraw);

            //device lost
            if(gResized){
                gResized = false;

                //clear all VRAM data
                terrainLODManager.dropVRAM();

                assert(terrainLODManager.getChunksToRender(gCam).empty());

                parameters.perspectiveScallingFactor = calculatePerspectiveScallingFactor(gWindow->getWidth(), kHorizontalFOV / 180.0f * 3.14f);

                terrainLODManager.updateParameters(parameters);
                
                terrainLODManager.initiateUpdate(gCam);
                
                //setup OpenGL
                setupOpenGL();
            }

            if(fpsTimer.getMilliseconds() > 1000){
                fpsTimer.start();
                std::cout << "FPS: " << frames << std::endl;

                std::cout << " Chunks per frame (approx.):  \t" << (float) gNumChunksRendered / frames << 
                          "\n Triangles per frame (approx.):\t" << (float) gNumTrianglesRendered / frames << std::endl;
                
                frames = 0;
                gNumChunksRendered = 0;
                gNumTrianglesRendered = 0;

            }
            else{
                frames++;
            }

            if(terrainTimer.getMilliseconds() > 1000){
                terrainLODManager.initiateUpdate(gCam);
                
                terrainTimer.start();
            }
            const TerrainLODManager::RenderChunkCont& chunkIds = (terrainLODManager.getChunksToRender(gCam));
            render(chunkIds, *VRAMStrategy, *chunkedTerrain, RAMStrategy->getTerrainWidth(), RAMStrategy->getTerrainHeight(), OpenGLMutex);

            if(gWindow->isDoubleBuffered()) gWindow->swapBuffers();
            else glFinish();

        }
    }
    catch(const std::exception& exc){
        std::cerr << exc.what() << std::endl;
        ioService.stop();
        threads.join_all();
        clean();
        return 1;
    }
    ioService.stop();
    threads.join_all();
    clean();
    std::cout << "Successfully finished." << std::endl;
    return 0;
}

/*
 *   Copyright 2010-2011 Alexander Medvedev
 *
 *   Permission is hereby granted, free of charge, to any person 
 *   obtaining a copy of this software and associated documentation
 *   files (the "Software"), to deal in the Software without
 *   restriction, including without limitation the rightsto use, 
 *   copy, modify, merge, publish, distribute, sublicense, and/or
 *   sell copies of the Software, and to permit persons to whom
 *   the Software is furnished to do so, subject to the following 
 *   conditions:
 *
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 */
