//CGProgram.hpp

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


#ifndef CG_PROGRAM_WRAPPER_HPP__
#define CG_PROGRAM_WRAPPER_HPP__

/**
 * \class hydra::rendering::CGProgram
 * \brief Wrapper over CG program (shader)
 *
 * Wraps CGprogram and provides some functions to deal with
 * Cg programs and their contents.
 *
 * \author A.V.Medvedev
 * \date 01.05.2011
 */

#include <Cg/cg.h>
#include "common/SharedPtr.hpp"
#include "common/PimplPtr.hpp"
#include <boost/noncopyable.hpp>
#include "rendering/CGParameter.hpp"
#include "rendering/CGContext.hpp"
#include "rendering/CGProfile.hpp"
#include <istream>
#include <stdexcept>
#include <cassert>

namespace hydra{
namespace rendering{

class CGProgram: private boost::noncopyable{
public:
    ///creates program
    CGProgram(CGContext& context, CGProfile& profile, const std::string& sourceCode);

    ///creates program using data from input stream (utility function)
    CGProgram(CGContext& context, CGProfile& profile, std::istream& stream);

    ///destroys the program (all references become invalid)
    ~CGProgram();

    ///load program (all the programs should be loaded before binding)
    void load();

    ///bind program to current state
    ///\warning profile should be enabled to finish binding
    void bind();

    ///get named parameter from program (may throw exception)
    template<typename T>
    CGParameter<T> getNamedParameter(const std::string& name){
        assert(program && cgIsProgram(program));

        CGparameter parameter = cgGetNamedParameter(program, name.c_str());
        if(!parameter) throw std::runtime_error("Parameter with specified name not found.");

        return CGParameter<T>(parameter);
    }

    ///update all parameters and send data to GPU
    ///\warning program must be bound before using this
    void updateParameters();

    ///returns native program representation
    ///\warning use with caution
    CGprogram getRepresentation() const;

private:
    CGprogram program;
};

///smart pointer to program
typedef hydra::common::SharedPtr<CGProgram>::Type CGProgramPtr;

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
