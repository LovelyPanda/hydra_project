//Window.hpp

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



#ifndef WINDOW_HPP__
#define WINDOW_HPP__

/**
 * \class hydra::system::Window
 * \brief Class for respresenting a window.
 *
 * Abstract interface of such platform-specific entity as window.
 *
 * \author A.V.Medvedev
 * \date 30.07.2010
 */

#include "common/SharedPtr.hpp"
#include <boost/noncopyable.hpp>
#include <string>

namespace hydra{

namespace rendering{

class Renderer;
typedef common::SharedPtr<Renderer>::Type RendererPtr;

}

namespace system{

class Input;
typedef common::SharedPtr<Input>::Type InputPtr;

class Window: private boost::noncopyable{

public:

    ///struct with window properties
    struct Properties{
        ///window's width
        unsigned int width;

        ///window's height
        unsigned int height;

        ///bits per pixel
        unsigned int bpp;
        
        ///window's caption
        std::string caption;

        ///whether cursor is invisible (true = invisible)
        bool cursorInvisibleFlag;

        ///whether window is resizable (true = resizable)
        bool resizable;

        ///whether window is doublebuffered
        bool doublebuffered;

        ///whether window is fullscreen
        bool fullscreen;
    };


    ///destructor
    virtual ~Window(){}

    ///resize window
    virtual void resize(unsigned int inWidth, unsigned int inHeight) = 0;

    ///changes mode to fullscreen one (if you do not want to keep current resolution
    ///but want library to choose best [default system resolution] pass 'false' as argument)
    virtual void toFullscreenMode(bool inPreserveResolution = true) = 0;

    ///changes mode to window one. If no new resolution specified (or zero values)
    ///the default window size is used Window::DEFAULT_* (640 X 480)
    virtual void toWindowMode(unsigned int inWidth = 0, unsigned int inHeight = 0) = 0;

    ///sets specified video mode
    virtual void setVideoMode(const Window::Properties& inProperties) = 0;

    ///creates renderer's context (if it is possible or if no any)
    virtual hydra::rendering::RendererPtr createRenderer(const std::string& inRendererName) = 0;

    ///returns current renderer's context (if any)
    virtual hydra::rendering::RendererPtr getRenderer() = 0;

    ///returns Input context
    virtual InputPtr getInput() = 0;

    ///swap buffers (if doublebuffered)
    virtual void swapBuffers() = 0;

    ///whether window is doublebuffered
    virtual bool isDoubleBuffered() const = 0;

    ///get window width
    virtual unsigned int getWidth() const = 0;

    ///get window height
    virtual unsigned int getHeight() const = 0;

    ///is it in fullscreen mode
    virtual bool isFullscreen() const = 0;

    ///returns current properties structure (creates copy)
    virtual Window::Properties getProperties() const = 0;

    ///sets window's caption
    virtual void setCaption(const std::string& inCaption) = 0;

    ///default window width
    static const unsigned int DEFAULT_WIDTH = 640;

    ///default window height
    static const unsigned int DEFAULT_HEIGHT = 480;
};

///pointer to window
typedef common::SharedPtr<hydra::system::Window>::Type WindowPtr;

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
