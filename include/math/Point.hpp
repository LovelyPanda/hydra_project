//Point.hpp

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


#ifndef POINT_HPP__
#define POINT_HPP__

/**
 * \struct hydra::math::Point
 * \brief Simple struct for representing a point in 3D
 *
 */

namespace hydra{

namespace math{
    
struct Point{

    ///builds zero point (0, 0, 0)
    inline Point(): x(0.0f), y(0.0f), z(0.0f){
    
    }

    ///builds point with specified coordinates
    inline Point(float inX, float inY, float inZ): x(inX), y(inY), z(inZ){

    }

    ///x-coord
    float x;
    ///y-coord
    float y;
    ///z-coord
    float z;
};

///operator < compares coordinates x, y, z. May be usefull for sorting
bool operator<(const Point& lhv, const Point& rhv);

///operator > compares coordinates x, y, z. May be usefull for sorting
bool operator>(const Point& lhv, const Point& rhv);

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
