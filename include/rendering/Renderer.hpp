//Renderer.hpp

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


#ifndef RENDERER_HPP__
#define RENDERER_HPP__

/**
 * \class hydra::rendering::Renderer
 * \brief Abstract class for representing renderer's context.
 *
 * Interface of renderer or entity which is responsible
 * for rendering an image from some data.
 * Renderer may be implemented using some hardware accelerated API
 * such as OpenGL or Direct3D but may be some software renderer.
 *
 * \author A.V.Medvedev
 * \date 30.07.2010
 */

#include <boost/noncopyable.hpp>
#include "common/SharedPtr.hpp"

namespace hydra{
namespace rendering{

class Renderer: boost::noncopyable{

public:
    virtual ~Renderer(){}
    virtual void someFunction() = 0;
};

///pointer to renderer object
typedef common::SharedPtr<hydra::rendering::Renderer>::Type RendererPtr;

} //rendering namespace

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
