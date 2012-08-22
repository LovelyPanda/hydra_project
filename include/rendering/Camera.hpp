//Camera.hpp

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

#ifndef CAMERA_HPP__
#define CAMERA_HPP__

/**
 * \class hydra::rendering::Camera
 * \brief Class for representing a camera object.
 * 
 * It has some functions to move camera around.
 * Class simplifies rendering and manipulating of camera.
 *
 * \todo add new functionality
 *
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

#include "math/Vector3D.hpp"
#include "common/SharedPtr.hpp"

namespace hydra{

namespace rendering{

class Camera{

public:
	///Creates camera at (0,0,0) directed to (1,0,0).
	Camera();

	///Creates camera with specified position.
	explicit Camera(const hydra::math::Vector3D& inPos, const hydra::math::Vector3D& inDir, const hydra::math::Vector3D& inUp = hydra::math::Vector3D(0.0f, 1.0f, 0.0f));
	
	///Moves camera relatively to its direction
	void move(float inForward, float inUp, float inLeft);
	
	///Sets coordinates of the camera to specified (global coords)
	void place(float inX, float inY, float inZ);

	///Returns position vector of camera
	inline const hydra::math::Vector3D& getPos() const{
		return mPos;
	}

	///Returns direction vector of camera
	inline const hydra::math::Vector3D& getDir() const{
		return mDir;
	}

	///Returns UP-vector of camera
	inline const hydra::math::Vector3D& getUp() const{
		return mUp;
	}

	///Sets matrix of transformations according to camera's data
	void getMatrix(float* inoutMatrix) const;
	
	///Rotates camera around specified axis on specified angle
	void rotate(float inAngleRadians, const hydra::math::Vector3D& inAxis);
	
	///Rotates camera around X axis
	void rotateX(float inAngleRadians);
	
	///Rotates camera around Y axis
	void rotateY(float inAngleRadians);
	

protected:
    ///vector of position
    hydra::math::Vector3D mPos;
    ///vector of direction
    hydra::math::Vector3D mDir;
    ///up-vector
    hydra::math::Vector3D mUp;	
};

///pointer (smart) to camera
typedef hydra::common::SharedPtr<Camera>::Type CameraPtr;

} //rendering namespace

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
