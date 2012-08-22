//ModelViewer.cpp

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
 * Just a sample program which uses loading library from Hydra project.
 * Loads models and shows them.
 *
 * Model file should be passed as the only console argument.
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
#include "SDL/SDL.h"

#define NO_SDL_GLEXT
#include "SDL/SDL_opengl.h"

#include <GL/gl.h>
#include <GL/glu.h>

#include "math/Vector3D.hpp"
#include "rendering/Camera.hpp"
#include "rendering/MeshOptimizer.hpp"

#include "data/Image.hpp"
#include "data/Model.hpp"
#include "data/Mesh.hpp"
#include "data/Material.hpp"

#include <boost/foreach.hpp>


using namespace hydra::loading;
using namespace hydra::math;
using namespace hydra::rendering;
using namespace hydra::common;
using namespace hydra::data;

#ifndef M_PI
    #define M_PI (3.14159265f)
#endif

const int DEFAULT_SCREEN_WIDTH = 640;
const int DEFAULT_SCREEN_HEIGHT = 480;

const float CAM_SPEED = 3.0f;

Camera gCam;

int gScreenWidth;
int gScreenHeight;
int gScreenBPP;
int gScreenVFlags;
int gSystemWidth;
int gSystemHeight;

bool gIsClicked = false;

bool gFullscreenFlag = false;
float gXAngle = 0.0f;
float gYAngle = 0.0f;
float gZAngle = 0.0f;
float gSens = 0.3f;

bool gLeftKeyPressed = false;
bool gRightKeyPressed = false;
bool gUpKeyPressed = false;
bool gDownKeyPressed = false;

bool gLinePolygonMode = false;
bool gDrawBothFaces = false;
bool gDrawNormals = false;
bool gLight = true;
bool gRotate = true;

float gLightPosition[]  = {20.0f, 50.0f, 0.0f, 1.0f};

std::map<std::string, GLuint> gTextures;

ModelPtr gModel;

struct VarrayIds{
    GLuint IBO;
    GLuint VBO;
};

std::vector<VarrayIds> gVarrays;
std::vector<MaterialPtr> gMaterials;

GLuint gNormalList;

float gZoom = 10;

bool gWorkFlag = true;
int gDelay = 20;

float* gCamMatrix = 0;

SDL_Surface* gScreenSurface = 0;

static void clean(){
    if(gCamMatrix) delete[] gCamMatrix;
    if(gScreenSurface) SDL_FreeSurface(gScreenSurface);
    SDL_Quit();
}

static void bindMaterial(const Material& inMaterial){
    const Color& ambient = inMaterial.mAmbient;
    const Color& diffuse = inMaterial.mDiffuse;
    const Color& specular = inMaterial.mSpecular;
    const Color& emissive = inMaterial.mEmissive;

    float amb[4] = {ambient.r(), ambient.g(), ambient.b(), 1.0f};
    float dif[4] = {diffuse.r(), diffuse.g(), diffuse.b(), inMaterial.getOpacity()};
    float spec[4] = {specular.r(), specular.g(), specular.b(), 1.0f};
    float emis[4] = {emissive.r(), emissive.g(), emissive.b(), 1.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT, GL_EMISSION, emis);

    float specExp = inMaterial.mSpecularExponent;
    glMaterialf(GL_FRONT, GL_SHININESS, specExp);

    if(inMaterial.mTexture != Material::ImageId()){
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gTextures[inMaterial.mTexture]);
    }
    else{
        glDisable(GL_TEXTURE_2D);
    }
        
    glActiveTextureARB(GL_TEXTURE1_ARB);
    if(inMaterial.mRefl != Material::ImageId()){
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gTextures[inMaterial.mRefl]);
    }
    else{
        glDisable(GL_TEXTURE_2D);
    }
    glActiveTextureARB(GL_TEXTURE0_ARB);

    //set transparency
    if(inMaterial.getOpacity() != 1.0f){ //if we need transparency
        glDisable(GL_DEPTH_TEST);
        if(!gDrawBothFaces)glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
    }
    else{ //disable blending stuff as we don't need it
        glDisable(GL_BLEND);
        if(!gDrawBothFaces && !gLinePolygonMode) glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }
}

static VarrayIds createMeshBufferObject(const MeshPtr inMesh){
    VarrayIds newVarrayIds;
    GLuint ids[2];
    glGenBuffersARB(2, ids);
    newVarrayIds.VBO = ids[0];
    newVarrayIds.IBO = ids[1];
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, newVarrayIds.VBO);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, static_cast<GLsizeiptr>(sizeof(Vertex) * inMesh->getVertexNum()),
                        (const void*)(&inMesh->mVertices[0]), GL_STATIC_DRAW_ARB);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, newVarrayIds.IBO);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, static_cast<GLsizeiptr>(sizeof(unsigned int) * inMesh->getIndexNum()),
                        (const void*)(&inMesh->mIndices[0]), GL_STATIC_DRAW_ARB);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

    return newVarrayIds;
}

static void createModelBufferObjects(const ModelPtr inModel){
    BOOST_FOREACH(const MeshPtr nextMesh, inModel->mMeshes){
        gVarrays.push_back(createMeshBufferObject(nextMesh));
    }
}

//not optimal for now! (same normals may be drawn few times)
static GLuint createNormalList(){
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glBegin(GL_LINES);
    BOOST_FOREACH(const MeshPtr nextMesh, gModel->mMeshes){    
        const Mesh::VertexCont& vertices = nextMesh->mVertices;
        BOOST_FOREACH(const Mesh::index_t nextIndex, nextMesh->mIndices){
            const Point& vert = vertices[nextIndex].mCoord;
            Vector3D norm = vertices[nextIndex].mNormal.getUnit();
            glVertex3f(vert.x, vert.y, vert.z);
            glVertex3f(vert.x + norm.x(), vert.y + norm.y(), vert.z + norm.z());
        }
    }
    glEnd();
    glEndList();

    return list;
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, inImg->getWidth(), inImg->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, data);
   
    return tex;
}

//load textures to OpenGL
static void loadTextures(){
    ImagePtr img;
    BOOST_FOREACH(const MaterialPtr material, gMaterials){
        //texture
        if(img = gModel->getImage(material->mTexture)) gTextures[material->mTexture] = loadTexture(img);
        //bump
        if(img = gModel->getImage(material->mBump)) gTextures[material->mBump] = loadTexture(img);
        //reflection map
        if(img = gModel->getImage(material->mRefl)) gTextures[material->mRefl] = loadTexture(img);
    }
}

// Setup default render states
static void setupOpenGL(){
    //first try to init extensions:
    if (glewInit() != GLEW_OK || !GLEW_ARB_multitexture || !GLEW_ARB_vertex_buffer_object){
        throw std::runtime_error("error loading OpenGL extensions using GLEW");
    }

    glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
    glClearDepth(1.0f);

    if(gLinePolygonMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glEnable(GL_DEPTH_TEST);

    if(gLinePolygonMode || gDrawBothFaces){
        glDisable(GL_CULL_FACE);
    }
    else{
        glEnable(GL_CULL_FACE);
    }

    glViewport(0, 0, gScreenWidth, gScreenHeight);    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)((float)gScreenWidth/(float)gScreenHeight), 1.0f, 10000.0f);

    float ambientColor[]    = {0.3f, 0.3f, 0.3f, 1.0f};
    float diffuseColor[]    = {0.9f, 0.9f, 0.9f, 1.0f};
    float specularColor[]   = {0.0f, 0.0f, 0.0f, 1.0f};


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

    //set up tex. coord. generator for reflection map (1 tex. unit)
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE0_ARB);


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //drop old lists and textures(if any)
    //BOOST_FOREACH(VarrayIds varId, gVarrays){
        //glDeleteBuffersARB(1, &varId.VBO);
        //glDeleteBuffersARB(1, &varId.IBO);
    //}

   // glDeleteLists(gNormalList, 1);
    
    typedef std::map<std::string, GLuint>::value_type value_type;
    //BOOST_FOREACH(value_type nextPair, gTextures)
        //glDeleteTextures(1, &nextPair.second);

    gVarrays.clear();
    gMaterials.clear();
    gTextures.clear();
    gNormalList = 0;

    BOOST_FOREACH(MeshPtr nextMesh, gModel->mMeshes)
        gMaterials.push_back(nextMesh->mMaterial);
    
    createModelBufferObjects(gModel);
    gNormalList = createNormalList();
    loadTextures();
}

//resize the OpenGL context space (called when window is resized)
static void resize(int new_width, int new_height){
    gScreenWidth = new_width;
    gScreenHeight = new_height;

    //we should reconfigure OpenGL context as on some systems (stupid windowz)
    //context breaks after window resize
    setupOpenGL();
}

//main render function
static void render(){
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
    
    glRotatef(gXAngle, 1.0f, 0.0f, 0.0f);
    glRotatef(gYAngle, 0.0f, 1.0f, 0.0f);
    glRotatef(gZAngle, 0.0f, 0.0f, 1.0f);

    glScalef(gZoom, gZoom, gZoom);

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glEnableClientState(GL_VERTEX_ARRAY);
    glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    size_t vertexStride = sizeof(Vertex);
    size_t coordOffset = offsetof(Vertex, mCoord);
    size_t texCoordOffset = offsetof(Vertex, mTexCoord);
    size_t normalOffset = offsetof(Vertex, mNormal);

    for(size_t i = 0; i < gVarrays.size(); ++i){
        bindMaterial(*gMaterials[i]);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, gVarrays[i].VBO);
        glTexCoordPointer(2, GL_FLOAT, vertexStride, (void *)texCoordOffset);
        glNormalPointer(GL_FLOAT, vertexStride, (void *)normalOffset);
        glVertexPointer(3, GL_FLOAT, vertexStride, (void*)0);

        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, gVarrays[i].IBO);
        if(gModel->mMeshes[i]->mMode == Mesh::TRIANGLES) 
            glDrawElements(GL_TRIANGLES, gModel->mMeshes[i]->getIndexNum(), GL_UNSIGNED_INT, (void*)(coordOffset));
        else if(gModel->mMeshes[i]->mMode == Mesh::TRIANGLE_STRIP)
            glDrawElements(GL_TRIANGLE_STRIP, gModel->mMeshes[i]->getIndexNum(), GL_UNSIGNED_INT, (void*)(coordOffset));
        else assert(!"unsupported primitive assemble mode!");
    }
        
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopClientAttrib();

    if(gDrawNormals){
        if(gLight) glDisable(GL_LIGHTING);
        glCallList(gNormalList);
        if(gLight) glEnable(GL_LIGHTING);
    }

    //OpenGL error handling:
    GLenum GLErrorCode;
    const GLubyte* GLErrorString;
    while((GLErrorCode = glGetError()) != GL_NO_ERROR){
        GLErrorString = gluErrorString(GLErrorCode);
        std::cerr << "OpenGL error: " << GLErrorString << std::endl;
    }
}

static void handleEvents(){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch(event.type){
        case SDL_KEYDOWN:
            switch(event.key.keysym.sym){
                case SDLK_ESCAPE:
                    gWorkFlag = false;
                    break;
                case SDLK_UP:
                case SDLK_w:
                    gUpKeyPressed = true;
                    break;
                case SDLK_DOWN:
                case SDLK_s:
                    gDownKeyPressed = true;
                    break;
                case SDLK_LEFT:
                case SDLK_a:
                    gLeftKeyPressed = true;
                    break;
                case SDLK_RIGHT:
                case SDLK_d:
                    gRightKeyPressed = true;
                    break;
                case SDLK_r:
                    gRotate = !gRotate;
                    break;
                case SDLK_PLUS:
                case SDLK_EQUALS:
                    gZoom *= 2.0f;
                    break;
                case SDLK_MINUS:
                case SDLK_UNDERSCORE:
                    gZoom /= 2.0f;
                    break;
                case SDLK_p:
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
                case SDLK_o:
                    gDrawBothFaces = !gDrawBothFaces;
                    
                    if(gDrawBothFaces){
                        glDisable(GL_CULL_FACE);
                    }
                    else{
                        glEnable(GL_CULL_FACE);
                    }
                    break;
                case SDLK_l:
                    gLight = !gLight;
                    if(gLight) glEnable(GL_LIGHTING);
                    else glDisable(GL_LIGHTING);
                    break;
                case SDLK_n:
                    gDrawNormals = !gDrawNormals;
                    break;
                case SDLK_f:
                    //was it fullscreen?
                    if(gScreenVFlags & SDL_FULLSCREEN){
                        gScreenVFlags ^= SDL_FULLSCREEN;
                        gScreenVFlags |= SDL_RESIZABLE;
                        if(!SDL_SetVideoMode(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, gScreenBPP, gScreenVFlags)){
                            throw std::runtime_error("resize error!");
                        }
                        resize(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
                    }
                    else{
                        gScreenVFlags |= SDL_FULLSCREEN;
                        gScreenVFlags ^= SDL_RESIZABLE;
                        if(!SDL_SetVideoMode(gSystemWidth, gSystemHeight, gScreenBPP, gScreenVFlags)){
                            throw std::runtime_error("resize error!");
                        }
                        resize(gSystemWidth, gSystemHeight);

                    }
                    break;
                default:
                    break;
            }
            break;
        case SDL_KEYUP:
            switch(event.key.keysym.sym){
                case SDLK_UP:
                case SDLK_w:
                    gUpKeyPressed = false;
                    break;
                case SDLK_DOWN:
                case SDLK_s:
                    gDownKeyPressed = false;
                    break;
                case SDLK_LEFT:
                case SDLK_a:
                    gLeftKeyPressed = false;
                    break;
                case SDLK_RIGHT:
                case SDLK_d:
                    gRightKeyPressed = false;
                    break;
                default:
                    break;
            }
            break;
        case SDL_MOUSEMOTION:
            if(gIsClicked){
                gCam.rotateX(M_PI * static_cast<float>(gScreenWidth/2 - event.motion.x) * gSens / 180.0f);
                gCam.rotateY(M_PI * static_cast<float>(gScreenHeight/2 - event.motion.y) * gSens / 180.0f);
                SDL_WarpMouse(gScreenWidth/2, gScreenHeight/2);
            }
            break;
        case SDL_MOUSEBUTTONUP:
            gIsClicked = false;
            break;
        case SDL_MOUSEBUTTONDOWN:
            gIsClicked = true;
            SDL_WarpMouse(gScreenWidth/2, gScreenHeight/2);
            break;
        case SDL_ACTIVEEVENT:
            break;
        case SDL_VIDEORESIZE:
            if(!SDL_SetVideoMode(event.resize.w, event.resize.h, gScreenBPP, gScreenVFlags)){
                throw std::runtime_error("resize error!");
            }
            resize(event.resize.w, event.resize.h);
            break;
        case SDL_QUIT:
            gWorkFlag = false;
            break;
        default:
            break;
        }
    }

    if(gUpKeyPressed) gCam.move(CAM_SPEED, 0.0f, 0.0f);
    if(gDownKeyPressed) gCam.move(-CAM_SPEED, 0.0f, 0.0f);
    if(gRightKeyPressed) gCam.move(0.0f, 0.0f, CAM_SPEED);
    if(gLeftKeyPressed) gCam.move(0.0f, 0.0f, -CAM_SPEED);

}

void printHelp(){
    std::cout <<  "\n\t========= HELP MESSAGE =========\n\
        Just a sample program which uses loading library from Hydra project.\n\
        Loads models and shows them.\n\
        \n\
        Model file should be passed as the only console argument.\n\
        \n\
        You can move through the scene using \"W\", \"A\", \"S\", \"D\" or arrows.\n\
        To stop/start rotation press \"R\"\n\
        To increase model's size (scale), press \"+\"\n\
        To decrease press \"-\"\n\
        To show/hide normals press \"N\"\n\
        To enable/disable lighting press \"L\"\n\
        To switch between polygon modes (filled or lines) press \"P\"\n\
        To enable/disable \"two-side\" draw polygon mode press \"O\"\n\n\
        See <http://hydraproject.org.ua> for more info." 
    << std::endl;
    
}

int main (int argv, char** args){
    
    std::cout << "=========== started ===========" << std::endl;

    //init factories for loading stuff
    initFactories();

    try{
        //load model
        if(argv > 1){
           gModel = loadFromFile<Model>(args[1]);
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
    
    gModel->sortMeshesByOpacity();
    MeshOptimizer meshOpt;
    meshOpt.setCacheSize(32);
    BOOST_FOREACH(MeshPtr nextMesh, gModel->mMeshes){
        meshOpt.optimizeMesh(nextMesh);
    }
    std::cout << "meshes optimization done" << std::endl;
    
    //print model info:
    std::cout << "Model name: " << gModel->mName << std::endl;
    std::cout << "Textures: " << gModel->mImages.size() << std::endl;
    std::cout << "Meshes: " << gModel->mMeshes.size() << std::endl;
    for(size_t i = 0; i < gModel->mMeshes.size(); ++i){
            std::cout << i << ")" << std::endl;
            std::cout << "Vertices: " << gModel->mMeshes[i]->getVertexNum() << std::endl;
            std::cout << "Indices: " << gModel->mMeshes[i]->getIndexNum() << std::endl; 
            std::cout << "Material: " << gModel->mMeshes[i]->mMaterial->mName<< std::endl;
    }

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cerr << "Couldn't init SDL" << std::endl;
        return 1;
    }

    const SDL_VideoInfo* info = 0;
    gScreenWidth = DEFAULT_SCREEN_WIDTH;
    gScreenHeight = DEFAULT_SCREEN_HEIGHT;

    info = SDL_GetVideoInfo( );

    if(!info){
        std::cerr << "Video query failed!" << std::endl;
        clean();
        return 1;
    }

    gSystemWidth = info->current_w;
    gSystemHeight = info->current_h;    
    gScreenBPP = info->vfmt->BitsPerPixel; 
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    gScreenVFlags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_RESIZABLE;
    gScreenVFlags |= SDL_HWPALETTE;
    if(info->hw_available) gScreenVFlags |= SDL_HWSURFACE;
    else  gScreenVFlags |= SDL_SWSURFACE;

    if(info->blit_hw)
        gScreenVFlags |= SDL_HWACCEL;
    if((gScreenSurface = SDL_SetVideoMode(gScreenWidth, gScreenHeight, gScreenBPP, gScreenVFlags)) == 0){
        std::cerr << "Couldn't set video mode" << std::endl;
        clean();
        return 1;
    }

    gCam = Camera(Vector3D(-200.0f, 0.0f, 0.0f), Vector3D(1.0f, 0.0f, 0.0f));
    gCamMatrix = new float[16];
    
    try{
        setupOpenGL();
    }
    catch(const std::exception& exc){
        std::cerr << exc.what() << std::endl;
        clean();
        return 1;
    }

    SDL_EnableKeyRepeat(300, 1);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_SetCaption("Hydra Model Viewer", NULL);

    GLint time = 0;
    GLint lasttime = 0;
    GLint frames = 0;
    try{
        while(gWorkFlag){
            time = SDL_GetTicks();
            if((time-lasttime) > 1000){
                lasttime = time;
                std::cout << "FPS: " << frames << std::endl;
                frames = 0;
            }
            else frames++;

            handleEvents();
            render();
            SDL_GL_SwapBuffers();

            if(gRotate) ++gYAngle;
    
            time = SDL_GetTicks() - time;
            if(time < gDelay){
                SDL_Delay(gDelay-time);
            }
        }
    }
    catch(const std::exception& exc){
        std::cerr << exc.what() << std::endl;
        clean();
        return 1;
    }
    clean();
    std::cout << "finished" << std::endl;
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
