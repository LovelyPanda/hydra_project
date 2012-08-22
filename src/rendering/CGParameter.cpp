//CGParameter.cpp

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

#include "rendering/CGParameter.hpp"
#include <cassert>

using hydra::rendering::CGParameter;

///sets 3 component vector as parameter value
template <>
void CGParameter<hydra::math::Vector3D>::setValue(const hydra::math::Vector3D& newValue){
    assert(parameter != 0);

    cgSetParameter3f(parameter, newValue.x(), newValue.y(), newValue.z());
}
    
///sets color as parameter's value
template <>
void CGParameter<hydra::data::Color>::setValue(const hydra::data::Color& newValue){
    assert(parameter != 0);
    
    cgSetParameter4f(parameter, newValue.r(), newValue.g(), newValue.b(), newValue.a());
}

///sets float value
template <>
void CGParameter<float>::setValue(const float& newValue){
    assert(parameter != 0);
    
    cgSetParameter1f(parameter, newValue);
}

///sets matrix as parameter's value. Matrix should be laid in row-major order.
template <>
void CGParameter<hydra::math::Matrix>::setValue(const hydra::math::Matrix& newValue){
    assert(parameter != 0);
    
    cgSetMatrixParameterfr(parameter, newValue.get());
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
