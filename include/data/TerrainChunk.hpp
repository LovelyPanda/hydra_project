//TerrainChunk.hpp

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

#ifndef TERRAIN_CHUNK_HPP__
#define TERRAIN_CHUNK_HPP__

/**
 * \struct hydra::data::TerrainChunk
 * \brief Represents a terrain's chunk.
 *
 * Chunk contains data needed to render square piece of terrain surface.
 * It stores indices, their level of detail and level of detail number for vertices.
 * Chunk represents only 1 level of detail contrary to hydra::data::TerrainFragment.
 *
 * \warning maximum number of indexed primitives is 65K!
 * \see hydra::data::TerrainFragment
 *
 * \author A.V.Medvedev
 * \date 31.08.2010
 */


#include <vector>
#include "common/SharedPtr.hpp"
#include "math/AABB.hpp"

namespace hydra{

namespace data{

struct TerrainChunk{
  
    ///type of index 
    typedef unsigned short index_t;

    ///type of container for indices
    typedef std::vector<index_t> IndexCont;

    ///vertices' level of detail
    //unsigned char vertices;

    ///level of details level (lies in interval[0, LODsNum - 1])
    ///0 - is level of thumbnail
    ///LODsNum - 1 - is heighest resolution level
    //unsigned char level;

    ///maximum error (absolute value) within this chunk
    //float maxError;

    ///indices
    IndexCont indices;
};

///smart pointer to TerrainChunk
typedef hydra::common::SharedPtr<TerrainChunk>::Type TerrainChunkPtr;

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
