//Quat.cpp

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

#include "math/Quat.hpp"
#include "math/Vector3D.hpp"
#include <cmath>

using hydra::math::Quat;
using hydra::math::Vector3D;

Quat::Quat(): mVec(), mScalar(0.0f){

}

Quat::Quat(const Quat& inQuat): mVec(inQuat.getVec()), mScalar(inQuat.getScalar()){

}

Quat::Quat(float inX, float inY, float inZ, float inW): mVec(inX, inY, inZ), mScalar(inW){

}

Quat::Quat(const Vector3D& inVec, float inScalar): mVec(inVec), mScalar(inScalar){

}

Quat::Quat(float inAngleRadians, const Vector3D& inAxis){
	float the_sin = static_cast<float>(sin(inAngleRadians / 2.0f));
	mVec = Vector3D(the_sin * inAxis.x(), the_sin * inAxis.y(), the_sin * inAxis.z());
	mScalar =  static_cast<float>(cos(inAngleRadians / 2.0f));
}

Quat Quat::operator*(const Quat& rhs) const{
	return Quat((rhs.getVec()*getScalar() + getVec()*rhs.getScalar() +
	getVec().cross(rhs.getVec())), (getScalar() * rhs.getScalar() -
	(getVec() * rhs.getVec()))); //YAY!
}

Quat Quat::operator+(const Quat& rhs) const{
	return Quat((getVec() + rhs.getVec()),(getScalar() + rhs.getScalar()));
}

Quat Quat::operator-(const Quat& rhs) const{
	return Quat((getVec() - rhs.getVec()),(getScalar() - rhs.getScalar()));
}

Quat& Quat::operator=(const Quat& rhs){
	setScalar(rhs.getScalar());
	setVec(rhs.getVec());
	return (*this);
}

Quat& Quat::operator-=(const Quat& rhs){
	setScalar(getScalar() - rhs.getScalar());
	setVec(getVec() - rhs.getVec());
	return (*this);
}

Quat& Quat::operator*=(const Quat& rhs){
	Quat res = (*this) * rhs;
	setScalar(res.getScalar());
	setVec(res.getVec());
	return (*this);
}

Quat& Quat::operator+=(const Quat& rhs){
	setScalar(getScalar() + rhs.getScalar());
	setVec(getVec() - rhs.getVec());
	return (*this);
}

void Quat::normalize(){
    float magn = getMagnitude();
    if(magn > 0.0001f){
        mScalar /= magn;
        mVec /= magn;
    }
}

void Quat::rotate(Vector3D& inoutVec) const{
	Quat inQuat(inoutVec);
	inQuat = (*this) * inQuat * getInverse();
	inoutVec = inQuat.getVec();
}

void Quat::inverse(){
	float squareMagnitude = getSquareMagnitude();
	mScalar /= squareMagnitude;
	mVec /= (-squareMagnitude);
}

Quat Quat::getInverse() const{
	Quat res(*this);
	res.inverse();
	return res;
}

float Quat::getMagnitude() const{
	float sm = getSquareMagnitude();
	return sqrtf(sm);
}

float Quat::getSquareMagnitude() const{
	return (getScalar()*getScalar() + getVec().getSquareMagnitude());
}

void Quat::getRotationMatrix(float* inoutMatrix) const{
	inoutMatrix[0] = 1 - 2*(mVec.x()*mVec.y() + mVec.z()*mVec.z());
	inoutMatrix[1] = 2*(mVec.x()*mVec.y() - mScalar*mVec.z());
	inoutMatrix[2] = 2*(mVec.x()*mVec.z() + mVec.y()*mScalar);
	inoutMatrix[3] = 2*(mVec.x()*mVec.y() + mScalar*mVec.z());
	inoutMatrix[4] = 1 - 2*(mVec.x()*mVec.x() + mVec.z()*mVec.z());
	inoutMatrix[5] = 2*(mVec.y()*mVec.z() - mScalar*mVec.x());
	inoutMatrix[6] = 2*(mVec.x()*mVec.z() - mScalar*mVec.y());
	inoutMatrix[7] = 2*(mVec.y()*mVec.z() + mScalar*mVec.x());
	inoutMatrix[8] = 1 - 2*(mVec.x()*mVec.x() + mVec.y()*mVec.y());
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
