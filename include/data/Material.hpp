//Material.hpp

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


#ifndef MATERIAL_HPP__
#define MATERIAL_HPP__

/**
 * \class hydra::data::Material
 * \brief Class for representing a material.
 * 
 * Simple container with getters and setters.
 * Contains id of images (not images itself).
 * This is made to give possibility of shared textures usage
 * between different materials or maybe even models.
 *
 * \author A.V. Medvedev
 * \date 26.04.2010
 */

#include <string>
#include "data/Color.hpp"
#include "common/SharedPtr.hpp"

namespace hydra{

namespace data{

class Material{

public:
    ///image id type
    typedef std::string ImageId;

    ///empty constructor
    Material();

    ///another constructor
    explicit Material(const std::string& inName, const hydra::data::Color& inAmbient = hydra::data::Color(), const hydra::data::Color& inSpecular = hydra::data::Color(), 
        const hydra::data::Color& inDiffuse = hydra::data::Color(), const hydra::data::Color& inEmissive = hydra::data::Color(), ImageId inTexture = ImageId(), float inTransp = 0.0);

    ///set transparency of material (same as setting 1.0 - opacity)
    inline void setTransparency(float inTransp){
        mTransparency = inTransp;
        if(mTransparency < 0.0f) mTransparency = 0.0f;
        else if(mTransparency > 1.0f) mTransparency = 1.0f;
    }

    ///set opacity of material (same as setting 1.0 - transparency)
    inline void setOpacity(float inOpacity){
        mTransparency = (1.0f - inOpacity);
        if(mTransparency < 0.0f) mTransparency = 0.0f;
        else if(mTransparency > 1.0f) mTransparency = 1.0f;
    }

    ///returns opacity
    inline float getOpacity() const{
        return (1.0f - mTransparency);
    }

    ///name
    std::string mName;
    ///ambient comp.
    hydra::data::Color mAmbient;
    ///diffusual comp.
    hydra::data::Color mDiffuse;
    ///specular comp.
    hydra::data::Color mSpecular;
    ///emissive comp
    hydra::data::Color mEmissive;
    ///transparency
    float mTransparency;
    ///spec. exp. factor
    float mSpecularExponent;
    ///optical density
    float mOpticalDensity;
    ///transmission filter
    hydra::data::Color mTransmissionFilter;
    ///texture id
    ImageId mTexture;
    ///bump id 
    ImageId mBump;
    ///reflection map id 
    ImageId mRefl;
};

///pointer (smart) to Material
typedef common::SharedPtr<hydra::data::Material>::Type MaterialPtr;

///compare function for sorting by opacity
inline bool compareByOpacity(const hydra::data::Material& inMat1, const hydra::data::Material& inMat2){
    return (inMat1.mTransparency > inMat2.mTransparency);
}

} //data namespace

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
