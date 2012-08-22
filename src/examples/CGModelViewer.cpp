//CGModelViewer.cpp

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
 * Loads models and shows them. Uses Cg wrapper to render.
 *
 * Model file should be passed as first console argument.
 * Vertex shader file is second argument while fragment shader is third argument.
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
#include <fstream>
#include <boost/foreach.hpp>

#include <GL/glew.h>

//we don't need to include SDL.h as we don't need to use SDL functions directly
#include "system/SDLWindow.hpp"
#include "system/Input.hpp"
#include "system/Event.hpp"

#include <GL/gl.h>
#include <GL/glu.h>

#include "math/Vector3D.hpp"
#include "math/Matrix.hpp"
#include "rendering/Camera.hpp"
#include "common/Timer.hpp"

#include "rendering/Renderer.hpp"

#include "data/Image.hpp"
#include "data/Model.hpp"
#include "data/Mesh.hpp"
#include "data/Material.hpp"

//glm
#include "glm/glm.hpp"
//#include "glm/gtc/matrix_projection.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

//Cg
#include <Cg/cg.h>
#include <Cg/cgGL.h>

//Cg C++ wrapper headers
#include "rendering/CGContext.hpp"
#include "rendering/CGProgram.hpp"
#include "rendering/CGError.hpp"
#include "rendering/CGParameter.hpp"
#include "rendering/CGProfile.hpp"

using namespace hydra::rendering;
using namespace hydra::math;
using namespace hydra::data;
using namespace hydra::loading;
using namespace hydra::system;
using namespace hydra::common;

CGContextPtr gCgContext;

CGProfile gCgVProfile,
          gCgFProfile;

CGProgramPtr gCgVProgram,
             gCgFProgram;

CGParameter<Matrix> gCgParam_ModelViewProj,
                    gCgParam_ModelView,
                    gCgParam_ModelViewIT;

CGParameter<Color>  gCgParam_GlobalAmbient,
                    gCgParam_LightColor,
                    gCgParam_Ka,
                    gCgParam_Ke,
                    gCgParam_Kd,
                    gCgParam_Ks;

CGParameter<Vector3D> gCgParam_LightPosition,
                      gCgParam_EyePosition;

CGParameter<float> gCgParam_Shininess,
                   gCgParam_Time,
                   gCgParam_Frequency,
                   gCgParam_ScaleFactor;

std::string gVertexShaderFileName;
std::string gFragmentShaderFileName;

#ifndef M_PI
    #define M_PI (3.14159265f)
#endif

const int DEFAULT_SCREEN_WIDTH = 640;
const int DEFAULT_SCREEN_HEIGHT = 480;

const float CAM_SPEED = 3.0f;

Camera gCam;

glm::mat4 gProjMatrix;
glm::mat4 gModelViewMatrix;
glm::mat4 gIdentityMatrix(1.0f, 0.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 1.0f, 0.0f,
                          1.0f, 1.0f, 1.0f, 1.0f);

bool gIsClicked = false;

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


//for vertex shader
float gScaleFactor = 2.0f;
float gFrequency = 0.5f;

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
Timer gTimer;

WindowPtr gWindow;
InputPtr gInput;

static void clean(){
    if(gCamMatrix) delete[] gCamMatrix;
}

static inline void checkOpenGLErrors(const std::string& inMessage){
    //OpenGL error handling:
    GLenum GLErrorCode;
    const GLubyte* GLErrorString;
    while((GLErrorCode = glGetError()) != GL_NO_ERROR){
        GLErrorString = gluErrorString(GLErrorCode);
        std::cerr << "OpenGL error: " << GLErrorString << ". Acquired " << inMessage << std::endl;
    }
}

static void bindMaterial(const Material& inMaterial){
    const Color& ambient = inMaterial.mAmbient;
    const Color& diffuse = inMaterial.mDiffuse;
    const Color& specular = inMaterial.mSpecular;
    const Color& emissive = inMaterial.mEmissive;

    Color diffuseWithOpacity = Color(diffuse.r(), diffuse.g(), diffuse.b(), inMaterial.getOpacity());

    /*glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT, GL_EMISSION, emis);
    */

    gCgParam_Ka.setValue(ambient);
    gCgParam_Kd.setValue(diffuseWithOpacity);
    gCgParam_Ks.setValue(specular);
    gCgParam_Ke.setValue(emissive);

    float specExp = inMaterial.mSpecularExponent;
    //glMaterialf(GL_FRONT, GL_SHININESS, specExp);
    gCgParam_Shininess.setValue(specExp);

    Color lightColor(0.7, 0.7, 0.7);
    gCgParam_LightColor.setValue(lightColor);

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

//Setup CG
static void initCg(){
    gCgContext = CGContextPtr(new CGContext());
    cgGLSetDebugMode(CG_FALSE);
    gCgContext->setDefferedParameterSettingMode(true);

    //initialize vertex shader profile
    gCgVProfile = CGProfile(CGProfile::VERTEX);
    gCgContext->checkForErrors();

    std::cout << "Vertex profile: " << gCgVProfile.getString() << std::endl;

    //load vertex shader
    std::ifstream vertexFileStream(gVertexShaderFileName.c_str(), std::ifstream::in | std::ifstream::binary);
    gCgVProgram = CGProgramPtr(new CGProgram(*gCgContext, gCgVProfile, vertexFileStream));
    gCgVProgram->load();
    gCgContext->checkForErrors();

    gCgParam_ModelViewProj = gCgVProgram->getNamedParameter<Matrix>("modelViewProj");

    //gCgParam_ModelView = cgGetNamedParameter(gCgVProgram, "modelView");
    //checkForCgError(gCgContext, "could not gey modelView parameter");
    //gCgParam_ModelViewIT = cgGetNamedParameter(gCgVProgram, "modelViewIT");
    //checkForCgError(gCgContext, "could not gey modelViewIT parameter");

    gCgParam_ScaleFactor = gCgVProgram->getNamedParameter<float>("scaleFactor");
    gCgParam_Time = gCgVProgram->getNamedParameter<float>("time");
    gCgParam_Frequency = gCgVProgram->getNamedParameter<float>("frequency");


    //initialize fragment shader
    gCgFProfile = CGProfile(CGProfile::FRAGMENT);
    gCgContext->checkForErrors();
    
    std::cout << "Fragment profile: " << gCgFProfile.getString() << std::endl;
    
    std::ifstream fragmentFileStream(gFragmentShaderFileName.c_str(), std::ifstream::in | std::ifstream::binary);
    gCgFProgram = CGProgramPtr(new CGProgram(*gCgContext, gCgFProfile, fragmentFileStream));
    gCgFProgram->load();
    gCgContext->checkForErrors();

    gCgParam_GlobalAmbient = gCgFProgram->getNamedParameter<Color>("globalAmbient");
    gCgParam_LightColor = gCgFProgram->getNamedParameter<Color>("lightColor");
    gCgParam_LightPosition = gCgFProgram->getNamedParameter<Vector3D>("lightPosition");
    gCgParam_EyePosition = gCgFProgram->getNamedParameter<Vector3D>("eyePosition");
    
    gCgParam_Ke = gCgFProgram->getNamedParameter<Color>("Ke");
    gCgParam_Ka = gCgFProgram->getNamedParameter<Color>("Ka");
    gCgParam_Kd = gCgFProgram->getNamedParameter<Color>("Kd");
    gCgParam_Ks = gCgFProgram->getNamedParameter<Color>("Ks");

    gCgParam_Shininess = gCgFProgram->getNamedParameter<float>("shininess");
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

    glViewport(0, 0, gWindow->getWidth(), gWindow->getHeight());    
    /*
    glMatrixMode(GL_PROJECTION); //don't need it
    glLoadIdentity(); //--
    gluPerspective(60.0f, ((float)gWindow->getWidth()/(float)gWindow->getHeight()), 1.0f, 10000.0f); //--
    */
    gProjMatrix = glm::perspective(60.0f, ((float)gWindow->getWidth()/(float)gWindow->getHeight()), 1.0f, 10000.f);

//    float ambientColor[]    = {0.3f, 0.3f, 0.3f, 1.0f};
//    float diffuseColor[]    = {0.9f, 0.9f, 0.9f, 1.0f};
//    float specularColor[]   = {0.0f, 0.0f, 0.0f, 1.0f};


//    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
//    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor);
//    glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
//    glLightfv(GL_LIGHT0, GL_POSITION, gLightPosition);
//    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
//    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
//    glEnable(GL_LIGHT0);
//    if(gLight)glEnable(GL_LIGHTING);


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


    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();

    //drop old lists and textures(if any)
    BOOST_FOREACH(VarrayIds varId, gVarrays){
        glDeleteBuffersARB(1, &varId.VBO);
        glDeleteBuffersARB(1, &varId.IBO);
    }

    glDeleteLists(gNormalList, 1);
    
    typedef std::map<std::string, GLuint>::value_type value_type;
    BOOST_FOREACH(value_type nextPair, gTextures)
        glDeleteTextures(1, &nextPair.second);

    gVarrays.clear();
    gMaterials.clear();
    gTextures.clear();
    gNormalList = 0;

    BOOST_FOREACH(MeshPtr nextMesh, gModel->mMeshes)
        gMaterials.push_back(nextMesh->mMaterial);
    
    createModelBufferObjects(gModel);
    gNormalList = createNormalList();
    loadTextures();

    try{
        //CG stuff
        gCgContext.reset();
        initCg();
    }
    catch(const std::runtime_error& exc){
        throw std::runtime_error(std::string("CGError: ") + exc.what());
    }
}

//main render function
static void render(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glLoadIdentity();

    //set identity matrix to model view matrix
    gModelViewMatrix = gIdentityMatrix;

    gCgVProfile.enable();
    gCgFProfile.enable();


    gCgVProgram->bind();
    gCgContext->checkForErrors();

    checkOpenGLErrors("after binding vertex program");

    assert(gCamMatrix);
    //get camera matrix
    gCam.getMatrix(gCamMatrix);

    glm::mat4 cameraMatrix(gCamMatrix[0], gCamMatrix[1], gCamMatrix[2], gCamMatrix[3],
                           gCamMatrix[4], gCamMatrix[5], gCamMatrix[6], gCamMatrix[7],
                           gCamMatrix[8], gCamMatrix[9], gCamMatrix[10], gCamMatrix[11],
                           gCamMatrix[12], gCamMatrix[13], gCamMatrix[14], gCamMatrix[15]);


    //multiply current matrix with camera matrix
    gModelViewMatrix *= cameraMatrix;
    
    //glMultMatrixf(gCamMatrix);


/*  gluLookAt(gCam.getPos().x(), gCam.getPos().y(), gCam.getPos().z(), 
          gCam.getPos().x() + gCam.getDir().x(), 
          gCam.getPos().y() + gCam.getDir().y(),
          gCam.getPos().z() + gCam.getDir().z(), 
          gCam.getUp().x(), gCam.getUp().y(), gCam.getUp().z());
*/  

    
    /*
    glRotatef(gXAngle, 1.0f, 0.0f, 0.0f);
    glRotatef(gYAngle, 0.0f, 1.0f, 0.0f);
    glRotatef(gZAngle, 0.0f, 0.0f, 1.0f);

    glScalef(gZoom, gZoom, gZoom);
    */

    gModelViewMatrix = glm::rotate(gModelViewMatrix, gXAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    gModelViewMatrix = glm::rotate(gModelViewMatrix, gYAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    gModelViewMatrix = glm::rotate(gModelViewMatrix, gZAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    gModelViewMatrix = glm::scale(gModelViewMatrix, glm::vec3(gZoom));

    /*
    float projMatrData[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projMatrData);
    Matrix proj(projMatrData);
    proj.transpose();
    */

    //transpose?
    

    /*
    float modelViewMatrixData[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrixData);
    Matrix modelView(modelViewMatrixData);
    modelView.transpose();
    */
    
    //transpose?
    


    Matrix invModelView(glm::value_ptr(glm::transpose(gModelViewMatrix)));//modelView);
    invModelView.invert();

    Vector3D cam_pos(0.0f, 0.0f, 0.0f);
    Vector3D objectSpaceEyePos = cam_pos * invModelView;//gCam.getPos();
    Vector3D objectSpaceLightPos = cam_pos * invModelView; //light is at the camera position

    glm::mat4 modelViewProj = gProjMatrix * gModelViewMatrix;
    Matrix modelViewProjMatrix(glm::value_ptr(glm::transpose(modelViewProj)));//proj * modelView;
    
    //push matrices to vertex shader
    gCgParam_ModelViewProj.setValue(modelViewProjMatrix);//modelViewProjectionMatrixData);
    //cgSetMatrixParameterfr(gCgParam_ModelView, modelMatrix);
    //cgSetMatrixParameterfr(gCgParam_ModelViewIT, modelMatrixIT);

    gCgParam_Time.setValue(gTimer.getSeconds());
    gCgParam_Frequency.setValue(gFrequency);
    gCgParam_ScaleFactor.setValue(gScaleFactor);
  
    checkOpenGLErrors("after setting CG parameters");

    gCgVProgram->updateParameters();
    checkOpenGLErrors("after updating CG vertex parameters");

    gCgFProgram->bind();
    //gCgFProfile.enable();
    gCgContext->checkForErrors();
    
    checkOpenGLErrors("after binding fragment program");

    //push light position and eye position (both in object-space)
    gCgParam_LightPosition.setValue(objectSpaceLightPos);
    gCgParam_EyePosition.setValue(objectSpaceEyePos);

    gCgParam_GlobalAmbient.setValue(Color(0.3f, 0.3f, 0.3f));
    
    gCgFProgram->updateParameters();
    checkOpenGLErrors("after updating CG fragment parameters");


    //============THE RENDERING ITSELF==========================

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
        gCgVProgram->updateParameters();
        gCgFProgram->updateParameters();
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, gVarrays[i].VBO);
        glTexCoordPointer(2, GL_FLOAT, vertexStride, (void *)texCoordOffset);
        glNormalPointer(GL_FLOAT, vertexStride, (void *)normalOffset);
        glVertexPointer(3, GL_FLOAT, vertexStride, (void*)0);

        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, gVarrays[i].IBO);
        if(gModel->mMeshes[i]->mMode == Mesh::TRIANGLES) 
            glDrawElements(GL_TRIANGLES, gModel->mMeshes[i]->getIndexNum(), GL_UNSIGNED_INT, (void*)(coordOffset));
        else if(gModel->mMeshes[i]->mMode == Mesh::TRIANGLE_STRIP)
            glDrawElements(GL_TRIANGLE_STRIP, gModel->mMeshes[i]->getIndexNum(), GL_UNSIGNED_INT, (void*)(coordOffset));
        else assert(!"unknown primitive assemble mode!");
        checkOpenGLErrors("after drawing the VBO");
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
    //============THE RENDERING ITSELF ENDED==========================
    
    
    
    checkOpenGLErrors("after rendering");

    gCgFProfile.disable();
    gCgVProfile.disable();
    gCgContext->checkForErrors();
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
                case Event::KEY_r:
                    gRotate = !gRotate;
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
                    setupOpenGL();
                    break;
                case Event::MOUSE_LEFT:
                case Event::MOUSE_RIGHT:
                    gIsClicked = true;
                    break;
                default:
                    break;
            }
            if(event.mod & Event::CTRL_MASK) std::cout << "ctrl is pressed" << std::endl;
            if(event.mod & Event::ALT_MASK) std::cout << "alt is pressed" << std::endl;
            if(event.mod & Event::LCTRL_MASK) std::cout << "left ctrl is pressed" << std::endl;
            if(event.mod & Event::LALT_MASK) std::cout << "left alt is pressed" << std::endl;
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
            setupOpenGL();
            break;
        case Event::QUIT:
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

static void printHelp(){
    std::cout <<  "\n\t========= HELP MESSAGE =========\n\
        Just a sample program which uses loading library from Hydra project.\n\
        Loads models and shows them. Uses Cg wrapper to render.\n\
        \n\
        Model file should be passed as the only console argument.\n\
        Vertex shader file is second argument while fragment shader is third argument.\n\
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

    if(argv != 4){
        printHelp();
        std::cout << "Press ENTER to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    gVertexShaderFileName = args[2];
    gFragmentShaderFileName = args[3];

    try{
        //load model
       gModel = loadFromFile<Model>(args[1]);
        
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

    //set video properties
    Window::Properties props;
    props.width = 640;
    props.height = 480;
    props.bpp = 0; //use default
    props.resizable = true;
    props.fullscreen = false;
    props.doublebuffered = true;
    props.cursorInvisibleFlag = true; //invisible
    props.caption = "Hydra Shader Viewer";

    //init SDL window
    gWindow = WindowPtr(new SDLWindow(props));
    gWindow->createRenderer("OpenGL");

    gInput = InputPtr(gWindow->getInput());

    gCam = Camera(Vector3D(-200.0f, 0.0f, 0.0f), Vector3D(1.0f, 0.0f, 0.0f));
    gCamMatrix = new float[16];
    
    try{
        setupOpenGL();
    }
    catch(const std::exception& exc){
        std::cerr << "Error while seting opengl up: " << exc.what() << std::endl;
        clean();
        return 1;
    }


    gTimer.start();
    int frames = 0;
    try{
        while(gWorkFlag){
            if(gTimer.getMilliseconds() > 1000){
                gTimer.start();
                std::cout << "FPS: " << frames << std::endl;
                frames = 0;
            }
            else frames++;

            handleEvents();
            render();

            if(gWindow->isDoubleBuffered()) gWindow->swapBuffers();
            else glFinish();

            if(gRotate) ++gYAngle;
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
