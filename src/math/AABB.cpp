//AABB.cpp

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

#include "math/AABB.hpp"
#include <cassert>

using hydra::math::AABB;

float AABB::getShortestDistance(const hydra::math::Vector3D& inPoint) const{
    //if inside return 0.0f
    if(isInside(inPoint)) return 0.0f;        
    
    //relative point's position
    Vector3D relativePos = inPoint - mCorner;

    bool x_positive = (relativePos.x() > (mVector.x() / 2.0f)), 
         y_positive = (relativePos.y() > (mVector.y() / 2.0f)), 
         z_positive = (relativePos.z() > (mVector.z() / 2.0f));

    float closestCornerX = 0.0f, closestCornerY = 0.0f, closestCornerZ = 0.0f;
    if(x_positive) closestCornerX = mVector.x();
    if(y_positive) closestCornerY = mVector.y();
    if(z_positive) closestCornerZ = mVector.z();

    Vector3D closestCorner(closestCornerX, closestCornerY, closestCornerZ);
    float distanceToClosestCorner = (relativePos - closestCorner).getMagnitude();

    //if in the interval
    bool xIsInInterval = relativePos.x() > 0.0f && relativePos.x() < mVector.x(), 
         yIsInInterval = relativePos.y() > 0.0f && relativePos.y() < mVector.y(), 
         zIsInInterval = relativePos.z() > 0.0f && relativePos.z() < mVector.z();

    assert(!(xIsInInterval && yIsInInterval && zIsInInterval));
    
    float distanceToClosestPlane = -1.0f;

    if(xIsInInterval && yIsInInterval){
        if(z_positive){
            distanceToClosestPlane = fabsf(relativePos.z() - mVector.z());
        }
        else{
            distanceToClosestPlane = fabsf(relativePos.z());
        }
    }

    if(xIsInInterval && zIsInInterval){
        if(y_positive){
            distanceToClosestPlane = fabsf(relativePos.y() - mVector.y());
        }
        else{
            distanceToClosestPlane = fabsf(relativePos.y());
        }
    }
    if(zIsInInterval && yIsInInterval){
        if(x_positive){
            distanceToClosestPlane = fabsf(relativePos.x() - mVector.x());
        }
        else{
            distanceToClosestPlane = fabsf(relativePos.x());
        }
    }   

    if(distanceToClosestPlane < 0.0f) return distanceToClosestCorner;
    else return std::min(distanceToClosestCorner, distanceToClosestPlane);
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
