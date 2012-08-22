//Camera.cpp

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

#include "rendering/Camera.hpp"
#include "math/Vector3D.hpp"
#include "math/Quat.hpp"

using hydra::math::Quat;
using hydra::math::Vector3D;
using hydra::rendering::Camera;

Camera::Camera(): mPos(0.0f, 0.0f, 0.0f), mDir(1.0f, 0.0f, 0.0f), 
		                          mUp(0.0f, 1.0f, 0.0f){

}

Camera::Camera(const Vector3D& inPos, const Vector3D& inDir,
	       const Vector3D& inUp):
		mPos(inPos), mDir(inDir), mUp(inUp){
	mDir.normalize();
	mUp.normalize();

}

//moves camera relatively to its direction
void Camera::move(float inForward, float inUp, float inLeft){
	mDir.normalize();
	mPos += (mDir * inForward);
	Vector3D leftRightAxis = mDir.cross(mUp);
	leftRightAxis.normalize();
	mPos += leftRightAxis * inLeft;
	mPos += mUp * inUp;
}

void Camera::place(float inX, float inY, float inZ){
	mPos = Vector3D(inX, inY, inZ);
}

void Camera::getMatrix(float* inoutMatrix) const{
	Vector3D leftAxis = mDir.cross(mUp);
	leftAxis.normalize();
	Vector3D down = leftAxis.cross(mDir);
	down.normalize();
	inoutMatrix[0] = leftAxis.x();
	inoutMatrix[4] = leftAxis.y();
	inoutMatrix[8] = leftAxis.z();

	inoutMatrix[1] = down.x();
	inoutMatrix[5] = down.y();
	inoutMatrix[9] = down.z();

	inoutMatrix[2] = -mDir.x();
	inoutMatrix[6] = -mDir.y();
	inoutMatrix[10] = -mDir.z();

	inoutMatrix[12] = - (mPos.x()*inoutMatrix[0] + mPos.y()*inoutMatrix[4] + mPos.z()*inoutMatrix[8]);
	inoutMatrix[13] = - (mPos.x()*inoutMatrix[1] + mPos.y()*inoutMatrix[5] + mPos.z()*inoutMatrix[9]);
	inoutMatrix[14] = - (mPos.x()*inoutMatrix[2] + mPos.y()*inoutMatrix[6] + mPos.z()*inoutMatrix[10]);
	
	inoutMatrix[3] = 0.0f;
	inoutMatrix[7] = 0.0f;
	inoutMatrix[11] = 0.0f;
	inoutMatrix[15] = 1.0f;
}

void Camera::rotate(float inAngleRadians, const Vector3D& inAxis){
	Quat rotationQuat(inAngleRadians, inAxis);
	rotationQuat.rotate(mDir);
}

void Camera::rotateX(float inAngleRadians){
	rotate(inAngleRadians, mUp);
}

void Camera::rotateY(float inAngleRadians){
	rotate(inAngleRadians, mDir.cross(mUp));
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
