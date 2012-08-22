//Material.cpp

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


#include "data/Material.hpp"
#include "data/Color.hpp"
#include <string>

using hydra::data::Material;
using hydra::data::Color;

Material::Material(): mName("default"), mAmbient(1.0f, 1.0f, 1.0f),
        mDiffuse(1.0f, 1.0f, 1.0f), mSpecular(1.0f, 1.0f, 1.0f), 
        mEmissive(0.0f, 0.0f, 0.0f),
        mTransparency(0.0f), mTexture(), mBump(), mRefl(){}

Material::Material(const std::string& inName, const Color& inAmbient, 
                            const Color& inSpecular, const Color& inDiffuse, 
                            const Color& inEmissive, ImageId inTexture, 
                            float inTransparency): 
          //initializers
          mName(inName), mAmbient(inAmbient), mDiffuse(inDiffuse),
          mSpecular(inSpecular), mEmissive(inEmissive), 
          mTransparency(inTransparency), mTexture(inTexture), 
          mBump(), mRefl(){

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
