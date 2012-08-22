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
 * \author A.V.Medvedev
 * \date 01.05.2011
 */

#include <Cg/cg.h>
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
    }

    ///constructs best profile (may throw exception, if no appropriate profiles found)
    CGProfile(CGProfile::Class inProfileClass = UNKNOWN): profileClass(inProfileClass){
        if(inProfileClass == VERTEX){
            if(cgIsProfileSupported(CG_PROFILE_GLSLV)){
                profile = CG_PROFILE_GLSLV;
            }
            else if(cgIsProfileSupported(CG_PROFILE_ARBVP1)){
                profile = CG_PROFILE_ARBVP1;
            }
            else{
                throw std::runtime_error("No appropriate Cg profiles supported.");
            }
        }
        else if(inProfileClass == FRAGMENT){
            if(cgIsProfileSupported(CG_PROFILE_GLSLF)){
                profile = CG_PROFILE_GLSLF;
            }
            else if(cgIsProfileSupported(CG_PROFILE_ARBFP1)){
                profile = CG_PROFILE_ARBFP1;
            }
            else{
                throw std::runtime_error("No appropriate Cg profiles supported.");
            }
        }
        else if(inProfileClass == UNKNOWN){
            profile = CG_PROFILE_UNKNOWN;
        }
        else{
            //not implemented yet
            std::runtime_error("This Cg profile is not implemented yet.");
        }

        setOptimalOptions();
    }

    ///sets optimal options for shader compiler according to this profile
    ///(you should check for errors after this call)
    void setOptimalOptions(){
        cgGLSetOptimalOptions(profile);
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
