//Matrix.hpp

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


#ifndef MATRIX_HPP__
#define MATRIX_HPP__

/**
 * \class hydra::math::Matrix
 * \brief Class for representing a matrix.
 *
 * So far it is a floating point 4x4 row-major matrix.
 *
 * \todo Add more functionality!
 *
 * \author A.V.Medvedev
 * \date 25.06.2010
 */

namespace hydra{

namespace math{

class Vector3D;

class Matrix{
public:
    ///default ctor. Builds identity matrix.
    Matrix();

    ///Constructor for building matrix from 4x4 array
    Matrix(const float* inMatrixData);

    ///Copy-constructor
    Matrix(const Matrix& inMatrix);

    ///destructor
    ~Matrix();

    ///function set pointer to 16 float elements of matrix
    ///User must allocate enough space before calling this function!
    inline void get(float* outData) const{
        for(int i = 0; i < 16; ++i) outData[i] = mData[i];
    }

    ///function returns pointer to 16 float elements of matrix
    inline const float* get() const{
        return mData;
    }

    ///function changes matrix object to create inverted matrix
    void invert();

    ///function transposes matrix (rows -> columns, columns -> rows)
    void transpose();

    ///matrix multiplication
    Matrix operator*(const Matrix& inMatrix);

    friend hydra::math::Vector3D operator* (const hydra::math::Vector3D& inVec, const hydra::math::Matrix& inMatrix);

private:
    float mData[16];
};

///vector to matrix multiplication
hydra::math::Vector3D operator*(const hydra::math::Vector3D& inVec, const hydra::math::Matrix& inMatrix);

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
