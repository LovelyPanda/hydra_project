//TerrainFragment.hpp

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

#ifndef TERRAIN_FRAGMENT_HPP__
#define TERRAIN_FRAGMENT_HPP__

/**
 * \struct hydra::data::TerrainFragment
 * \brief Struct represents square fragment of terrain surface.
 *
 * Terrain heightmap is splitted into square fragments upon load.
 * Each fragment may contain several levels of detail and even
 * some hierarchical structure of LODs.
 * This struct represents such square. It contains different LODs
 * of vertices (with flags of whether they are loaded),
 * hierarchy of chunks (with same flags).
 *
 * \see hydra::data::TerrainChunk
 *
 * \author A.V.Medvedev
 * \date 05.09.2010
 */

#include <vector>
#include "common/QuadTree.hpp"
#include "common/SharedPtr.hpp"
#include "math/AABB.hpp"
#include "data/Vertex.hpp"
#include <boost/cstdint.hpp>

namespace hydra{

namespace data{

struct TerrainChunk;
typedef hydra::common::SharedPtr<TerrainChunk>::Type TerrainChunkPtr;

struct TerrainFragment{

    /**
     * \brief Contains status of some loadable object.
     *
     * Shows whether object is in memory (RAM, VRAM) or
     * is it unloaded.
     */
    enum Status{
        ///unloaded from RAM and VRAM
        UNLOADED = 0,

        ///loaded to RAM but not in VRAM
        RAM,

        ///loaded to VRAM and present in RAM
        VRAM,

        ///currently scheduled to load
        LOADING
    };

    /**
     * \brief Contains pointer to chunk data and chunk's status.
     *
     * Contains smart pointer to TerrainChunk object which may be empty.
     * Also contains status of data.
     *
     * \see hydra::data::TerrainFragment::ChunkStatus
     */
    struct ChunkData{
        ///chunk's status
        volatile TerrainFragment::Status status;

        ///maximum error (in world space) of chunk
        float maxError;

        ///vertices' level of detail
        unsigned char vertices;

        ///level of details level (lies in interval[0, LODsNum - 1])
        ///0 - is level of thumbnail
        ///LODsNum - 1 - is heighest resolution level
        unsigned char level;
       
        ///axis aligned bounding box of chunk
        hydra::math::AABB aabb;

        ///pointer to data
        hydra::data::TerrainChunkPtr ptr;
    };

    ///type of quadtree of chunks
    typedef hydra::common::QuadTree<ChunkData> QuadTreeOfChunks;

    ///pointer (smart) to quadtree of chunks
    typedef hydra::common::SharedPtr<QuadTreeOfChunks>::Type QuadTreeOfChunksPtr;
    
    ///terrain vertex (compressed)
    struct CompressedVertex{
        ///x coordinate
        boost::uint16_t x;
        
        ///z coordinate
        boost::uint16_t z;
        
        ///y coordinate
        float y;

        ///x component of normal vector
        float normalX;

        ///z component of normal vector
        float normalZ;
    };

    ///type of container for vertices
    typedef std::vector<CompressedVertex> VertexCont;

    ///quad tree which contains chunks for different levels of details
    QuadTreeOfChunksPtr tree;

    ///status of fragment's tree itself (not the chunks)
    volatile Status status;

    ///vertex levels of details
    std::vector<VertexCont> vertexLODs;

    ///vertex LODs' statuses
    std::vector<Status> vertexLODsStatuses;
};

///smart pointer to terrain fragment
typedef hydra::common::SharedPtr<TerrainFragment>::Type TerrainFragmentPtr;

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
