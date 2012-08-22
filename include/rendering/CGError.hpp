//CGError.hpp

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


#ifndef CG_ERROR_WRAPPER_HPP__
#define CG_ERROR_WRAPPER_HPP__

/**
 * \class hydra::rendering::CGError
 * \brief Wrapper over CG error
 *
 * Wraps CGerror and provides some functions to deal with
 * Cg errors.
 * CGError may be used as exception (it is derived from std::runtime_error).
 *
 * \author A.V.Medvedev
 * \date 01.05.2011
 */

#include <Cg/cg.h>
#include "common/SharedPtr.hpp"
#include <stdexcept>

namespace hydra{
namespace rendering{

class CGError: public std::runtime_error{
public:
    ///constructor
    CGError(CGerror inError, const std::string& inDescription) throw(): std::runtime_error(inDescription), error(inError){

    }

    ///copy-constructor
    CGError(const CGError& inCGError) throw(): std::runtime_error(inCGError.what()), error(inCGError.error){
        
    }

    ///assignment
    CGError& operator=(const CGError& inCGError) throw(){
        if(this != &inCGError){
            //static_cast<std::runtime_error>(*this) = inCGError;
            std::runtime_error::operator=(inCGError);
            error = inCGError.error;
        }

        return *this;
    }

    ///destructor
    virtual ~CGError() throw(){

    }
/*
    ///shows description
    virtual const char* what() const throw(){
        return errorString.c_str();
    }
*/
    ///returns native representation
    CGerror getRepresentation() const{
        return error;
    }

    ///returns true in case of error
    inline operator bool() const{
        return (error != CG_NO_ERROR);
    }

private:
    CGerror error;
};

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
