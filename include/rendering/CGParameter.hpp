//CGParameter.hpp

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


#ifndef CG_PROGRAM_PARAMETER_WRAPPER_HPP__
#define CG_PROGRAM_PARAMETER_WRAPPER_HPP__

/**
 * \class hydra::rendering::CGParameter
 * \brief Wrapper over CG program's parameter
 *
 * Wraps CGparameter.
 *
 * \author A.V.Medvedev
 * \date 01.05.2011
 */

#include <Cg/cg.h>
#include "common/SharedPtr.hpp"
#include <boost/noncopyable.hpp>

#include "math/Matrix.hpp"
#include "math/Vector3D.hpp"
#include "data/Color.hpp"

namespace hydra{
namespace rendering{

template <typename T>
class CGParameter{

public:
    ///empty ctor
    CGParameter(): parameter(0){

    }

    ///just save parameter
    CGParameter(CGparameter inParameter): parameter(inParameter){

    }

    ///we don't call cgDestroyParameter as documentation say we don't need to call
    ///if we are not creating parameters with Create* functions
    ~CGParameter(){

    }

    ///Sets parameter's value. This function has some implementations
    ///for different types (T). If you are getting 'undefined reference'
    ///error, there is no implementation for requested type.
    void setValue(const T& newValue);

    ///get parameter's native representation
    inline CGparameter getRepresentation() const{
        return parameter;
    }

private:
    CGparameter parameter;
};

//specializations

///sets 3 component vector as parameter value
template <>
void CGParameter<hydra::math::Vector3D>::setValue(const hydra::math::Vector3D& newValue);

///sets color as parameter's value
template <>
void CGParameter<hydra::data::Color>::setValue(const hydra::data::Color& newValue);

///sets float value
template <>
void CGParameter<float>::setValue(const float& newValue);
    
///sets matrix as parameter's value. Matrix should be laid in row-major order.
template <>
void CGParameter<hydra::math::Matrix>::setValue(const hydra::math::Matrix& newValue);

} //rendering
} //hydra namespace

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
