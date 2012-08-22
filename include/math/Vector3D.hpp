//Vector3D.hpp

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


#ifndef VECTOR_3D_HPP__
#define VECTOR_3D_HPP__

/**
 * \class hydra::math::Vector3D
 * \brief Class for representing 3-dimensional vector.
 * 
 * Contains different vector operations.
 * See course of linear algebra for more information 
 * about vectors.
 *
 * \todo 
 *       -# what about changing the name to more shorter one
 *       -# what about 2D vectors? what about ND vectors?
 *
 * \author A.V.Medvedev
 * \date 26.04.2010
 *
 */

namespace hydra{

namespace math{

struct Point;

class Vector3D{

public:
    ///Empty constructor. Creates (0, 0, 0) vector.
    Vector3D();

    ///Constructor with specified values for vector.
    Vector3D(float inX, float inY, float inZ);

    ///Constructor builds vector from Point. Vector points from zero of axises to specified point.
    Vector3D(const hydra::math::Point& inPoint);

    ///Constructor with data given at array
    Vector3D(float inData[3]);

    ///Vector addition
    Vector3D operator+(const Vector3D& rhs) const;
    
    ///Vector subtraction
    Vector3D operator-(const Vector3D& rhs) const;
    
    ///Vector scalling
    Vector3D operator*(float rhs) const;
    
    ///Dot product of vectors
    float operator*(const Vector3D& rhs) const;
    
    ///Vector scalling
    Vector3D operator/(float rhs) const;
    
    ///Unary minus operator. Returns same vector in opposite direction
    Vector3D operator-() const; 
    
    ///Adds and sets new value
    Vector3D& operator+=(const Vector3D& rhs);
    
    ///Subtracts and sets new value
    Vector3D& operator-=(const Vector3D& rhs);
    
    ///Scales ans sets new value
    Vector3D& operator*=(float rhs);
    
    ///Scales ans sets new value
    Vector3D& operator/=(float rhs);

    ///Compares x, y, z coords. Helpfull for sorting.
    bool operator<(const Vector3D& rhv) const;

    ///Compares x, y, z coords. Helpfull for sorting.
    bool operator>(const Vector3D& rhv) const;

    ///Returns vector's magnitude
    float getMagnitude() const;

    ///Returns square magnitude (sometimes you need it)
    float getSquareMagnitude() const;

    ///Returns values of vector's coordinates in array
    inline void get(float* inoutVec) const{
        inoutVec[0] = mX;
        inoutVec[1] = mY;
        inoutVec[2] = mZ;
    }
    
    ///Returns x coordinate of vector
    inline float x() const{
        return mX;
    }

    ///Returns y coordinate of vector
    inline float y() const{
        return mY;
    }
    
    ///Returns z coordinate of vector
    inline float z() const{
        return mZ;
    }

    ///sets new x coordinate
    inline void setX(float inX){
        mX = inX;
    }

    ///sets new y coord
    inline void setY(float inY){
        mY = inY;
    }

    ///sets new z coord
    inline void setZ(float inZ){
        mZ = inZ;
    }

    ///Cross product of 2 vectors. Returns new vector perpendicular to both.
    Vector3D cross(const Vector3D& inVec) const;

    ///Vector normalization. After that the magnitude of vector is 1.
    void normalize();

    ///Returns new vector with same direction but with magnitude of 1.
    Vector3D getUnit() const;

    ///Inverts vector. Its direction will be opposite but the magnitude same.
    void inverse();

    ///Transforms vector using the given transformation matrix
    void transform(const float* const matrix);

    ///Returns angle in radians between vectors.
    float getRadians(const Vector3D& inVec) const;

private:
    ///x-component of vector
    float mX;
    ///y-component of vector
    float mY;
    ///z-component of vector
    float mZ;

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
