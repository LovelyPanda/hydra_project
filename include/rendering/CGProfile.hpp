//CGProfile.hpp

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


#ifndef CG_PROFILE_WRAPPER_HPP__
#define CG_PROFILE_WRAPPER_HPP__

/**
 * \class hydra::rendering::CGProfile
 * \brief Wrapper over CG profile
 *
 * Wraps CGprofile and provides some functions to deal with
 * Cg profiles.
 *
 * \todo move implementation to source file
 *
 * \author A.V.Medvedev
 * \date 01.05.2011
 */

#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include "common/SharedPtr.hpp"
#include <stdexcept>

namespace hydra{
namespace rendering{

class CGProfile{
public:
    ///class of profile
    enum Class{
        UNKNOWN = 0,
        VERTEX,
        GEOMETRY,
        FRAGMENT,
        TESSELATION_CONTROL,
        TESSELATION_EVALUATION
    };

    ///constructs best profile (may throw exception, if no appropriate profiles found)
    CGProfile(CGProfile::Class inProfileClass = UNKNOWN): profileClass(inProfileClass){
        if(inProfileClass == VERTEX){
#ifdef BUGGED_CG_GLSL
            profile = cgGLGetLatestProfile(CG_GL_VERTEX);
#else
            if(cgIsProfileSupported(CG_PROFILE_GLSLV)){
                profile = CG_PROFILE_GLSLV;
            }
            else if(cgIsProfileSupported(CG_PROFILE_ARBVP1)){
                profile = CG_PROFILE_ARBVP1;
            }
            else{
                throw std::runtime_error("No appropriate Cg profiles supported.");
            }
#endif
        }
        else if(inProfileClass == FRAGMENT){
#ifdef BUGGED_CG_GLSL
            profile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
#else
            if(cgIsProfileSupported(CG_PROFILE_GLSLF)){
                profile = CG_PROFILE_GLSLF;
            }
            else if(cgIsProfileSupported(CG_PROFILE_ARBFP1)){
                profile = CG_PROFILE_ARBFP1;
            }
            else{
                throw std::runtime_error("No appropriate Cg profiles supported.");
            }
#endif
        }
        else if(inProfileClass == UNKNOWN){
            profile = CG_PROFILE_UNKNOWN;
        }
        else{
            //not implemented yet
            std::runtime_error("This Cg profile is not implemented yet.");
        }

        if(inProfileClass != UNKNOWN){
            setOptimalOptions();
        }
    }

    ///sets optimal options for shader compiler according to this profile
    ///(you should check for errors after this call)
    void setOptimalOptions(){
        cgGLSetOptimalOptions(profile);
    }

    ///enable profile (make some OpenGL calls)
    void enable(){
        cgGLEnableProfile(profile);
    }

    ///disable profile
    void disable(){
        cgGLDisableProfile(profile);
    }

    ///get profile string
    const char* getString() const{
        return cgGetProfileString(profile);
    }

    ///get native profile representation
    inline CGprofile getRepresentation() const{
        return profile;
    }

private:
    CGprofile profile;
    Class profileClass;
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
