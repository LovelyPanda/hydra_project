//ChunkedTerrain.cpp

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

#include "data/ChunkedTerrain.hpp"
#include <stdexcept>

using hydra::data::ChunkedTerrain;
using hydra::data::TerrainFragment;

TerrainFragment& ChunkedTerrain::getFragment(hydra::data::TerrainFragmentId inId){
    FragmentCont::iterator iter = mField.find(inId); 
    if(iter == mField.end()) 
        throw std::out_of_range("TerrainFragmentId has wrong value or fragment has not been loaded yet.");
    return iter->second;
}

const TerrainFragment& ChunkedTerrain::getFragment(hydra::data::TerrainFragmentId inId) const{
    FragmentCont::const_iterator iter = mField.find(inId); 
    if(iter == mField.end() || !(iter->second.tree)) 
        throw std::out_of_range("TerrainFragmentId has wrong value or fragment has not been loaded yet.");
    return iter->second;
}

void ChunkedTerrain::setFragment(hydra::data::TerrainFragmentId inId, const hydra::data::TerrainFragment& inFragment){
    mField[inId] = inFragment;
}


const hydra::data::TerrainChunk& ChunkedTerrain::getChunk(hydra::data::TerrainChunkId inId) const{
    const hydra::data::TerrainFragment& fragment = getFragment(inId.getFragmentId());
    try{
        const hydra::data::TerrainFragment::ChunkData& chunkData = 
            fragment.tree->getNode(inId.getQuadTreePos()).data;
        if(chunkData.status == hydra::data::TerrainFragment::UNLOADED){
            throw std::runtime_error("TerrainChunk is marked as UNLOADED. No data to return."); //error no data to return 
        }
        else return *chunkData.ptr;
    }
    catch(const std::out_of_range& exception){
        throw std::out_of_range("TerrainChunkId has wrong value. No such element in tree.");
    }
}

void ChunkedTerrain::putChunk(hydra::data::TerrainChunkId inId, hydra::data::TerrainChunkPtr inChunk){
    try{
        TerrainFragment& fragment = getFragment(inId.getFragmentId());
        if(!fragment.tree){
            throw std::runtime_error("Can't put chunk. Tree is unloaded.");
        }
        
        hydra::data::TerrainFragment::ChunkData& chunkData = 
                fragment.tree->getNode(inId.getQuadTreePos()).data;
        chunkData.ptr = inChunk;
        if(inChunk) chunkData.status = hydra::data::TerrainFragment::RAM;
        else chunkData.status = hydra::data::TerrainFragment::UNLOADED;
    }
    catch(const std::out_of_range& exception){
        throw std::out_of_range("TerrainChunkId has wrong value. Can't put data.");
    }
}

void ChunkedTerrain::setStatus(hydra::data::TerrainChunkId inId, hydra::data::TerrainFragment::Status inStatus){
    FragmentCont::iterator iter = mField.find(inId.getFragmentId()); 
    if(iter == mField.end() || !iter->second.tree) throw std::out_of_range("TerrainFragmentId has wrong value or fragment has not been loaded yet.");
    try{
        //may also throw std::out_of_range
        iter->second.tree->getNode(inId.getQuadTreePos()).data.status = inStatus;
    }
    catch(const std::out_of_range& exception){
        throw std::out_of_range("TerrainChunkId has wrong value. Can't set status.");
    }
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
