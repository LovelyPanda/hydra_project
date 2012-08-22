//Vertex.cpp

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

#include "data/Vertex.hpp"
#include "math/Point.hpp"
#include "math/Vector3D.hpp"
#include <cmath>

using hydra::math::Point;
using hydra::math::Vector3D;
using hydra::data::Vertex;

bool Vertex::equals(const Vertex& rhv, float inThreshold) const{
    //check coordinate equality first
    float coordEquality = (fabsf(mCoord.x - rhv.mCoord.x) + 
                           fabsf(mCoord.y - rhv.mCoord.y) +
                           fabsf(mCoord.z - rhv.mCoord.z)) / 3.0f;
    if(coordEquality > inThreshold) return false;
    
    //check texture coordinate equality
    float texCoordEquality = (fabsf(mTexCoord.x - rhv.mTexCoord.x) + 
                              fabsf(mTexCoord.y - rhv.mTexCoord.y)) / 2.0f;
    if(texCoordEquality > inThreshold) return false;

    //check normal vector equality
    float normalEquality = (fabsf(mNormal.x() - rhv.mNormal.x()) + 
                            fabsf(mNormal.y() - rhv.mNormal.y()) +
                            fabsf(mNormal.z() - rhv.mNormal.z())) / 3.0f;
    return (normalEquality < inThreshold);
}

bool hydra::data::operator<(const Vertex& lhv, const Vertex& rhv){
    if(lhv.mCoord < rhv.mCoord) return true;
    else if(lhv.mCoord > rhv.mCoord) return false;
    //coords are equal here, so compare texture coordinates
    else if(lhv.mTexCoord < rhv.mTexCoord) return true;
    else if(lhv.mTexCoord > rhv.mTexCoord) return false;
    //texture coords are also equal here, compare normals
    else if(lhv.mNormal < rhv.mNormal) return true;
    else return false;
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
