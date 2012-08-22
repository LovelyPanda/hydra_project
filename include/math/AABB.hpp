//AABB.hpp

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


#ifndef AXIS_ALIGNED_BOUNDING_BOX_HPP__
#define AXIS_ALIGNED_BOUNDING_BOX_HPP__

/**
 * \class hydra::math::AABB
 * \brief Axis aligned bounding box.
 *
 * Simple class which stores AABB characteristics.
 * It is the simplest approximation for some 3D object.
 * May be used for some rendering technics (like occlusion query and
 * frustum culling) or for collision detection.
 * 
 * \todo Check it. It may contain errors!
 *
 * \author A.V.Medvedev
 * \date 30.08.2010
 */

#include "math/Vector3D.hpp"

#include <algorithm>
#include <cmath>

namespace hydra{

namespace math{

class AABB{

public:
    ///builds empty AABB
    inline AABB(){
    
    }

    ///builds AABB from corner coord and vector to opposite corner
    inline AABB(const hydra::math::Vector3D& inCorner, const hydra::math::Vector3D& inVectorToOppositeCorner){

        //we should make some checks
        float corner_x = std::min(inCorner.x(), inCorner.x() + inVectorToOppositeCorner.x());
        float corner_y = std::min(inCorner.y(), inCorner.y() + inVectorToOppositeCorner.y());
        float corner_z = std::min(inCorner.z(), inCorner.z() + inVectorToOppositeCorner.z());

        mCorner = hydra::math::Vector3D(corner_x, corner_y, corner_z);

        float vec_x, vec_y, vec_z;
        vec_x = fabsf(inVectorToOppositeCorner.x());
        vec_y = fabsf(inVectorToOppositeCorner.y());
        vec_z = fabsf(inVectorToOppositeCorner.z());

        mVector = hydra::math::Vector3D(vec_x, vec_y, vec_z);
    }
    
    ///whether specified point is inside AABB
    inline bool isInside(const hydra::math::Vector3D& inPoint) const{
        hydra::math::Vector3D diff = inPoint - mCorner;
        return (diff.x() >= 0.0f && diff.x() <= mVector.x()) &&
            (diff.y() >= 0.0f && diff.y() <= mVector.y()) &&
            (diff.z() >= 0.0f && diff.z() <= mVector.z());
    }

    ///calculates center point of AABB
    inline hydra::math::Vector3D getCenter() const{
        return mCorner + mVector / 2.0f;
    }

    ///calculates square distance between specified point and center of AABB
    inline float getSquareDistance(const hydra::math::Vector3D& inPoint) const{
        hydra::math::Vector3D diff = inPoint - getCenter();
        
        return (diff.x()) * (diff.x()) +
               (diff.y()) * (diff.y()) +
               (diff.z()) * (diff.z());
    }

    ///calculates distance between specified point and center of AABB
    inline float getDistance(const hydra::math::Vector3D& inPoint) const{
        return sqrtf(getSquareDistance(inPoint));        
    }

    ///calculates the shortest distance from point to box's surface
    float getShortestDistance(const hydra::math::Vector3D& inPoint) const;

    ///returns corner of AABB (closest to (0,0,0) )
    inline const hydra::math::Vector3D& getCorner() const{
        return mCorner;
    }

    ///returns vector of AABB from corner to opposite corner
    inline const hydra::math::Vector3D& getVector() const{
        return mVector;
    }

private:
    
    ///contains corner which is closer to (0, 0, 0)
    hydra::math::Vector3D mCorner;
    ///contains vector to opposite corner
    hydra::math::Vector3D mVector;
};

} //math namespace

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
