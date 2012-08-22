//Vector3D.cpp

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

#include "math/Vector3D.hpp"
#include "math/Point.hpp"
#include <cmath>
#include <cassert>

using hydra::math::Vector3D;
using hydra::math::Point;

Vector3D::Vector3D(): mX(0.0f), mY(0.0f), mZ(0.0f){

}

Vector3D::Vector3D(float inX, float inY, float inZ): mX(inX), mY(inY), mZ(inZ){

}

Vector3D::Vector3D(const Point& inPoint): mX(inPoint.x), mY(inPoint.y), mZ(inPoint.z){

}

Vector3D::Vector3D(float inData[3]): mX(inData[0]), mY(inData[1]), mZ(inData[2]){

}

Vector3D Vector3D::operator+(const Vector3D& rhs) const{
	return Vector3D(mX + rhs.x(), mY + rhs.y(), mZ + rhs.z());
}

Vector3D Vector3D::operator-(const Vector3D& rhs) const{
	return Vector3D(mX - rhs.x(), mY - rhs.y(), mZ - rhs.z());
}

Vector3D Vector3D::operator*(float rhs) const{
	return Vector3D(mX * rhs, mY * rhs, mZ * rhs);
}

/// Dot product of vectors.
float Vector3D::operator*(const Vector3D& rhs) const{
	return (mX * rhs.x() + mY * rhs.y() + mZ * rhs.z());
}

Vector3D Vector3D::operator/(float rhs) const{
	assert(rhs != 0);
	return Vector3D(mX / rhs, mY / rhs, mZ / rhs);
}

Vector3D Vector3D::operator-() const{
	return Vector3D(-mX, -mY, -mZ);
}

Vector3D& Vector3D::operator+=(const Vector3D& rhs){
	mX += rhs.x();
	mY += rhs.y();
	mZ += rhs.z();
	return (*this);
}

Vector3D& Vector3D::operator-=(const Vector3D& rhs){
	mX -= rhs.x();
	mY -= rhs.y();
	mZ -= rhs.z();
	return (*this);
}

Vector3D& Vector3D::operator*=(float rhs){
	mX *= rhs;
	mY *= rhs;
	mZ *= rhs;
	return (*this);
}

Vector3D& Vector3D::operator/=(float rhs){
	assert((rhs != 0) && "are you trying to devide by 0?");
    if(rhs != 0.0f){
	    mX /= rhs;
	    mY /= rhs;
	    mZ /= rhs;
    }
	return (*this);
}

bool Vector3D::operator<(const Vector3D& rhv) const{
    if(mX < rhv.mX) return true;
    else if(mX > rhv.mX) return false;
    else if(mY < rhv.mY) return true;
    else if(mY > rhv.mY) return false;
    else if(mZ < rhv.mZ) return true;
    else return false;
}

bool Vector3D::operator>(const Vector3D& rhv) const{
    if(mX > rhv.mX) return true;
    else if(mX < rhv.mX) return false;
    else if(mY > rhv.mY) return true;
    else if(mY < rhv.mY) return false;
    else if(mZ > rhv.mZ) return true;
    else return false;
}

float Vector3D::getMagnitude() const{
	return sqrtf(getSquareMagnitude());
}

float Vector3D::getSquareMagnitude() const{
	return (mX*mX + mY*mY + mZ*mZ);
}

Vector3D Vector3D::cross(const Vector3D& inVec) const{
	return Vector3D(mY*inVec.z() - mZ*inVec.y(), -(mX*inVec.z() - mZ*inVec.x()), mX*inVec.y() - mY*inVec.x());
}

void Vector3D::normalize(){
	float magn = getMagnitude();
	//assert((magn != 0) && "this is a zero vector. It can't be normalized");
	if(!magn) return;
    mX /= magn;
	mY /= magn;
	mZ /= magn;
}

//returns unit vector with the same direction
Vector3D Vector3D::getUnit() const{
	Vector3D res(*this);
	res.normalize();
	return res;
}

void Vector3D::inverse(){
	mX = -mX;
	mY = -mY;
	mZ = -mZ;
}

void Vector3D::transform(const float* const matrix){
	float newX = matrix[0]*mX + matrix[1]*mY + matrix[2] * mZ;
	float newY = matrix[3]*mX + matrix[4]*mY + matrix[5] * mZ;
	float newZ = matrix[6]*mX + matrix[7]*mY + matrix[8] * mZ;
	mX = newX;
	mY = newY;
	mZ = newZ;
}

//returns angle between 2 vectors in radians
float Vector3D::getRadians(const Vector3D& inVec) const{
	float dot = (*this) * inVec;
	dot*=dot;
	dot /= (getSquareMagnitude()*inVec.getSquareMagnitude());
	dot = sqrtf(dot); // 1 sqrtf-call except 2 calls in getMagnitudes
	return acosf(dot); 
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
