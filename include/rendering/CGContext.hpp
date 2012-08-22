//CGContext.hpp

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


#ifndef CG_CONTEXT_WRAPPER_HPP__
#define CG_CONTEXT_WRAPPER_HPP__

/**
 * \class hydra::rendering::CGContext
 * \brief Wrapper over CG context
 *
 * Wraps CGcontext and provides some functions to deal with
 * Cg shaders and other stuff.
 *
 * \author A.V.Medvedev
 * \date 01.05.2011
 */

#include <boost/noncopyable.hpp>

#include "common/SharedPtr.hpp"
#include "rendering/CGError.hpp"

#include <Cg/cg.h>

namespace hydra{
namespace rendering{

class CGContext: private boost::noncopyable{
public:
    ///Constructs the Cg context
    CGContext();

    ///destroys the context
    ~CGContext();

    ///get last Cg error
    inline CGError getLastError(){
        CGerror error;
        const char* errorString = cgGetLastErrorString(&error);

        return CGError(error, (errorString) ? errorString : std::string());
    }

    ///check for errors and throw it if any
    inline void checkForErrors(){
        CGError error = getLastError();
        if(error) throw error;
    }

    ///checks if the context is a valid Cg context object
    ///(it could be invalidated by someone)
    bool isValid() const;

    ///enabled/disables deffered mode as parameter setting mode(it is not enabled by default)
    ///\see cgSetParameterSettingMode
    void setDefferedParameterSettingMode(bool enable);

    ///returns native context representation
    ///warning: use with caution
    CGcontext getRepresentation() const{
        return context;
    }

private:
    CGcontext context;
};

///smart pointer to Cg context
typedef hydra::common::SharedPtr<CGContext>::Type CGContextPtr;

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
