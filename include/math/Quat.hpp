//Quat.hpp

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


#ifndef QUAT_HPP__
#define QUAT_HPP__

/**
 * \class hydra::math::Quat
 * \brief Class for representing a quaternion.
 * 
 * Contains different operations with quaternions.
 * See course of mathematics for more information
 * about how to use quaternions.
 * 
 * \todo think about pimpl
 *
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

#include "Vector3D.hpp"

namespace hydra{

namespace math{
    
class Quat{

public:
    ///Empty constructor creates quaternions with all zeros
    Quat();

    ///Copy-constructor
    Quat(const Quat& inQuat);

    ///Constructor from vector and scalar.
    ///Values are copied directly to object with no transformations
    Quat(const hydra::math::Vector3D& inVec, float inScalar = 0.0f);

    ///Constructor from 4 floats (scalar value [W] is last one)
    Quat(float inX, float inY, float inZ, float inW);
    
    ///Constructor wich creates quaternion from vector representing axis of rotation
    ///and angle of rotation.
    Quat(float inAngleRadians, const hydra::math::Vector3D& inAxis);
    
    ///quaternion multiplication    
    Quat operator*(const Quat& rhs) const;

    ///quaternion addition
    Quat operator+(const Quat& rhs) const;

    ///quaternion subtraction
    Quat operator-(const Quat& rhs) const;

    ///sets values
    Quat& operator=(const Quat& rhs);

    ///subtracts and sets new values
    Quat& operator-=(const Quat& rhs);

    ///multiplies and sets new value
    Quat& operator*=(const Quat& rhs);

    ///adds ans sets new values
    Quat& operator+=(const Quat& rhs);

    ///directly sets scalar
    inline void setScalar(float inScalar){
        mScalar = inScalar;
    }

    ///directly sets vector's value
    inline void setVec(const hydra::math::Vector3D& inVec){
        mVec = inVec;
    }

    ///returns scalar value
    inline float getScalar() const{
        return mScalar;
    }

    ///returns vector's value
    inline const hydra::math::Vector3D& getVec() const{
        return mVec;
    }

    ///normalization
    void normalize();

    ///rotates given vector using quaternion's conversion
    void rotate(hydra::math::Vector3D& inoutVec) const;

    ///inverts quaternion
    void inverse();

    ///returns new quaternion which is inverted
    Quat getInverse() const;

    ///returns magnitude of quaternion
    float getMagnitude() const;

    ///returns square magnitude (sometimes you don't need magnitude)
    float getSquareMagnitude() const;

    ///sets values to 4x4 matrix
    void getRotationMatrix(float* inoutMatrix) const;

private:
    hydra::math::Vector3D mVec;
    float mScalar;
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
