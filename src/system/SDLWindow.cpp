//SDLWindow.cpp

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

#include "system/SDLWindow.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/OpenGLRenderer.hpp"

#include "system/SDLInput.hpp"

#include "SDL/SDL.h"

#define NO_SDL_GLEXT
#include "SDL/SDL_opengl.h"


#include <stdexcept>
#include <cassert>

#include <algorithm>
#include <cstring>

using hydra::system::Window;
using hydra::system::SDLWindow;
using hydra::rendering::Renderer;
using hydra::rendering::RendererPtr;
using hydra::rendering::OpenGLRenderer;
using hydra::system::SDLInput;
using hydra::system::InputPtr;

struct SDLWindow::Impl{
    int screenVideoFlags;
    Window::Properties properties;
    unsigned int bestVideoBPP;
    unsigned int bestVideoWidth;
    unsigned int bestVideoHeight;
    SDL_Surface* surface;
    RendererPtr renderer;
    InputPtr input;
};


SDLWindow::SDLWindow(const Window::Properties& inProperties): mImpl(new Impl()){
    assert(mImpl);

    //init SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        throw std::runtime_error("SDLWindow: Couldn't init SDL.");
    }

    //get best video mode info

    const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();
    if(!videoInfo) throw std::runtime_error("SDLWindow: Couldn't get video info.");
    //save video info
    mImpl->bestVideoBPP = videoInfo->vfmt->BitsPerPixel;
    mImpl->bestVideoWidth = videoInfo->current_w;
    mImpl->bestVideoHeight = videoInfo->current_h;

    //save properties
    mImpl->properties = inProperties;
    Window::Properties& newProperties = mImpl->properties;

    //find out window size
    if(!inProperties.fullscreen){
        //for window mode use defaut ones if no specified
        if(inProperties.width <= 0) newProperties.width = Window::DEFAULT_WIDTH;
        if(inProperties.height <= 0) newProperties.height = Window::DEFAULT_HEIGHT;
    }
    else{
        //use system width/height value for fullscreen mode
        if(inProperties.width <= 0) newProperties.width = mImpl->bestVideoWidth;
        if(inProperties.height <= 0) newProperties.height = mImpl->bestVideoHeight;
    }

    //find out proper 'bits per pixel' value
    if(inProperties.bpp < 8){
        newProperties.bpp = mImpl->bestVideoBPP;
    }

    //now fill the SDL video flags
    int flags = 0;
    if(inProperties.fullscreen){
        flags |= SDL_FULLSCREEN;
    }
    else{
        if(inProperties.resizable) flags |= SDL_RESIZABLE;
    }
    
    flags |= SDL_HWPALETTE;

    if(videoInfo->hw_available) flags |= SDL_HWSURFACE;
    else flags |= SDL_SWSURFACE;

    if(videoInfo->blit_hw) flags |= SDL_HWACCEL;
    
    mImpl->screenVideoFlags = flags;

    //create OpenGL context
    createRenderer("OpenGL");

    mImpl->input = InputPtr(new SDLInput());

    //create surface and set video mode (may throw std::runtime_error)
    setVideoMode(newProperties);
}

SDLWindow::~SDLWindow(){
    //if(mImpl && mImpl->surface) SDL_FreeSurface(mImpl->surface);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void SDLWindow::resize(unsigned int inWidth, unsigned int inHeight){
    assert(mImpl);

    mImpl->properties.width = inWidth;
    mImpl->properties.height = inHeight;

    setVideoMode(mImpl->properties);
}

void SDLWindow::toFullscreenMode(bool inPreserveResolution){
    assert(mImpl);

    //was it fullscreen?
    if(isFullscreen()){
        return;
    }

    if(!inPreserveResolution){
        mImpl->properties.width = mImpl->bestVideoWidth;
        mImpl->properties.height = mImpl->bestVideoHeight;
    }

    mImpl->properties.fullscreen = true;

    setVideoMode(mImpl->properties);
}

void SDLWindow::toWindowMode(unsigned int inWidth, unsigned int inHeight){
    assert(mImpl);

    if(!isFullscreen()){
        return;
    }

    if(inWidth <= 0) mImpl->properties.width = Window::DEFAULT_WIDTH;
    else mImpl->properties.width = inWidth;
    if(inHeight <= 0) mImpl->properties.height = Window::DEFAULT_HEIGHT;
    else mImpl->properties.height = inHeight;
    
    mImpl->properties.fullscreen = false;
    
    setVideoMode(mImpl->properties);
}

void SDLWindow::setVideoMode(const Window::Properties& inProperties){
    assert(mImpl);

    mImpl->properties = inProperties;
    int flags = mImpl->screenVideoFlags;

    //fullscreen flag has higher priority
    if(inProperties.fullscreen) {
        flags &= !SDL_RESIZABLE;
        flags |= SDL_FULLSCREEN;

        if(inProperties.width <= 0) mImpl->properties.width = mImpl->bestVideoWidth;
        if(inProperties.height <= 0) mImpl->properties.height = mImpl->bestVideoHeight;
    }
    else{
        flags &= !SDL_FULLSCREEN;
        if(inProperties.resizable) flags |= SDL_RESIZABLE;
        else flags &= !SDL_RESIZABLE;

        if(inProperties.width <= 0) mImpl->properties.width = Window::DEFAULT_WIDTH;
        if(inProperties.height <= 0) mImpl->properties.height = Window::DEFAULT_HEIGHT;
    }

    //OpenGL specific flags
    if(mImpl->renderer){
        flags |= SDL_OPENGL;
        if(inProperties.doublebuffered) SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        else SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
    }
    else{
        flags &= (!SDL_OPENGL);
    }

    if(inProperties.bpp < 8) mImpl->properties.bpp = mImpl->bestVideoBPP;

    mImpl->screenVideoFlags = flags;

    mImpl->surface = SDL_SetVideoMode(mImpl->properties.width, mImpl->properties.height, mImpl->properties.bpp, mImpl->screenVideoFlags);

    if(!mImpl->surface) throw std::runtime_error("SDLWindow: Couldn't change video mode.");

    SDL_ShowCursor((mImpl->properties.cursorInvisibleFlag)? SDL_DISABLE : SDL_ENABLE);
    SDL_WM_SetCaption(mImpl->properties.caption.c_str(), 0);
}


RendererPtr SDLWindow::createRenderer(const std::string& inRendererName){
    assert(mImpl);
    if(mImpl->renderer) return mImpl->renderer;

    std::string rendererName = inRendererName;
    //to lower case
    std::transform(rendererName.begin(), rendererName.end(), rendererName.begin(), tolower);

    if(rendererName != "opengl" && rendererName != "default"){
        throw std::runtime_error("SDLWindow: Could not create rendering context of specified type. Only 'OpenGL' is supported.");
    }

    //create OpenGL context
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    if(mImpl->properties.doublebuffered) SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    else SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0); 
    
    int flags = mImpl->screenVideoFlags;
    flags |= SDL_OPENGL;

    if(mImpl->properties.doublebuffered) flags |= SDL_GL_DOUBLEBUFFER;
    else flags ^= SDL_GL_DOUBLEBUFFER;

    mImpl->renderer = RendererPtr(new OpenGLRenderer());

    return mImpl->renderer;
}

RendererPtr SDLWindow::getRenderer(){
    assert(mImpl);
    
    return mImpl->renderer;
}

InputPtr SDLWindow::getInput(){
    assert(mImpl);

    return mImpl->input;
}

void SDLWindow::swapBuffers(){
    assert(mImpl);
    if(!mImpl->renderer) return;

    if(mImpl->properties.doublebuffered) SDL_GL_SwapBuffers();
}

bool SDLWindow::isDoubleBuffered() const{
    assert(mImpl);

    return mImpl->properties.doublebuffered;
}

unsigned int SDLWindow::getWidth() const{
    assert(mImpl);
    return mImpl->properties.width;
}

unsigned int SDLWindow::getHeight() const{
    assert(mImpl);
    return mImpl->properties.height;
}

Window::Properties SDLWindow::getProperties() const{
    assert(mImpl);
    return mImpl->properties;
}

bool SDLWindow::isFullscreen() const{
    assert(mImpl);
    return mImpl->properties.fullscreen;
}

void SDLWindow::setCaption(const std::string& inCaption){
    assert(mImpl);
    mImpl->properties.caption = inCaption;
    SDL_WM_SetCaption(inCaption.c_str(), 0);
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
