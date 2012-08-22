//TerrainChunkId.hpp

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

#ifndef TERRAIN_CHUNK_ID_HPP__
#define TERRAIN_CHUNK_ID_HPP__

/**
 * \class hydra::data::TerrainChunkId
 * \brief Class for representing Chunk identificator.
 *
 * Objects of this class are used to find out which Chunk object
 * is needed. They are used for example at the loading process, when you
 * should specify which chunk do you want to load.
 * It contains TerrainFragmentId instance as TerrainChunk is a part of TerrainFragment.
 *
 * \see hydra::data::TerrainChunk
 * \see hydra::data::TerrainFragment
 * \see hydra::data::TerrainFragmentId
 *
 * \author A.V.Medvedev
 * \date 31.08.2010
 */

#include "data/TerrainFragmentId.hpp"

namespace hydra{

namespace data{

class TerrainChunkId{

public:
    ///builds empty chunk id
    inline TerrainChunkId(){

    }

    ///builds terrain chunk id from specified parameters
    explicit inline TerrainChunkId(hydra::data::TerrainFragmentId inFragId, 
                                   unsigned short inQuadTreePos):
                                                    mFragId(inFragId),
                                                    mQuadTreePos(inQuadTreePos){
        
    }
    
    ///returns fragment id which contains this chunk
    const hydra::data::TerrainFragmentId& getFragmentId() const{
        return mFragId;
    }

    ///returns chunk's position in quad tree
    unsigned short getQuadTreePos() const{
        return mQuadTreePos;
    }

    ///operator < for sorting and other such things
    inline bool operator<(const TerrainChunkId& inId) const{
        if(mFragId != inId.mFragId){
            return (mFragId < inId.mFragId);
        }
        else return (mQuadTreePos < inId.mQuadTreePos);
    }

    ///operator ==
    inline bool operator==(const TerrainChunkId& inId) const{
        return (mFragId == inId.mFragId && mQuadTreePos == inId.mQuadTreePos);       
    }

    ///operator !=
    inline bool operator!=(const TerrainChunkId& inId) const{
        return !operator==(inId);
    }

private:
    hydra::data::TerrainFragmentId mFragId;
    unsigned short mQuadTreePos;
};

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
