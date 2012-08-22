//SDLWindow.hpp

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



#ifndef SDL_WINDOW_HPP__
#define SDL_WINDOW_HPP__

/**
 * \class hydra::system::SDLWindow
 * \brief SDL implementation of Window.
 *
 * Class for creating a window using SDL library.
 *
 * \author A.V.Medvedev
 * \date 30.07.2010
 */

#include "system/Window.hpp"
#include "common/PimplPtr.hpp"

namespace hydra{
namespace system{

class SDLWindow: public hydra::system::Window{

public:
    ///constructor
    SDLWindow(const Window::Properties& inProperties);

    ///destructor
    virtual ~SDLWindow();

    ///resize window
    virtual void resize(unsigned int inWidth, unsigned int inHeight);

    ///changes mode to fullscreen one
    virtual void toFullscreenMode(bool inPreserveResolution = true);

    ///changes window mode to window one (not fullscreen)
    virtual void toWindowMode(unsigned int inWidth = 0, unsigned int inHeight = 0);

    ///sets video mode
    virtual void setVideoMode(const Window::Properties& inProperties);

    ///creates renderer's context (only 'opengl' is possible for renderer name)
    virtual rendering::RendererPtr createRenderer(const std::string& inRendererName);

    ///returns OpenGL context
    virtual rendering::RendererPtr getRenderer();

    ///returns input
    virtual InputPtr getInput();

    ///swaps buffers
    virtual void swapBuffers();

    ///whether window is doublebuffered
    virtual bool isDoubleBuffered() const;

    ///returns window's width
    virtual unsigned int getWidth() const;

    ///returns window's height
    virtual unsigned int getHeight() const;

    ///returns window's properties
    virtual Window::Properties getProperties() const;

    ///is it fullscreen
    virtual bool isFullscreen() const;

    virtual void setCaption(const std::string& inCaption);

private:

    ///pimpl idiom
    struct Impl;
    common::PimplPtr<Impl>::Type mImpl;

};

} //system namespace

} //hydra

#endif

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
