//Vertex.hpp

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


#ifndef VERTEX_HPP__
#define VERTEX_HPP__

/**
 * \struct hydra::data::Vertex
 * \brief Structure represents simple vertex for use in graphics
 *
 * Contains coordinate, texture coordinate and vector normal.
 */

#include "math/Point.hpp"
#include "math/Vector3D.hpp"

namespace hydra{

namespace data{

struct Vertex{

    ///vertex's coordinate
    hydra::math::Point mCoord;
    
    ///vertex's texture coordinate [may be 3D]
    hydra::math::Point mTexCoord;

    ///vertex's normal vector
    hydra::math::Vector3D mNormal;

    ///checks equality using threshold value
    bool equals(const Vertex& rhv, float inThreshold = 0.0f) const;
};

///operator < compares coordinates, then texture coordinates, then normals
bool operator<(const Vertex& lhv, const Vertex& rhv);

} //data namespace

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
