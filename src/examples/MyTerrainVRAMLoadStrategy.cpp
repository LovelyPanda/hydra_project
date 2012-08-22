//MyTerrainVRAMLoadStrategy.cpp

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

//hydra headers
#include "MyTerrainVRAMLoadStrategy.hpp"
#include "rendering/TerrainLODManager.hpp"
#include "data/ChunkedTerrain.hpp"
#include "data/TerrainChunkId.hpp"
#include "data/TerrainFragmentId.hpp"
#include "data/Vertex.hpp"
#include "math/Vector3D.hpp"

//boost headers
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

//STL headers
#include <stdexcept>
#include <string>
#include <iostream>

//c headers
#include <cassert>

using hydra::data::ChunkedTerrain;
using hydra::data::ChunkedTerrainPtr;
using hydra::data::TerrainFragment;
using hydra::data::TerrainFragmentPtr;
using hydra::data::TerrainFragmentId;
using hydra::data::TerrainChunk;
using hydra::data::TerrainChunkPtr;
using hydra::data::TerrainChunkId;
using hydra::data::Vertex;
using hydra::math::Vector3D;

typedef TerrainFragment::QuadTreeOfChunksPtr QTreePtr;
typedef TerrainFragment::QuadTreeOfChunks QTree;

typedef MyTerrainVRAMLoadStrategy::IndexBufferHandle IBHandle;
typedef MyTerrainVRAMLoadStrategy::VertexBufferHandle VBHandle;

struct VertexLODInfo{
    unsigned int level;
    VBHandle handle;

    std::vector<unsigned short> refCounters;
};

typedef std::map<TerrainChunkId, IBHandle> IndexHandleCont;
typedef std::map<TerrainFragmentId, VertexLODInfo> VertexHandleCont;

struct MyTerrainVRAMLoadStrategy::Impl{
    Impl(ChunkedTerrainPtr inTerrain, boost::asio::io_service& inIOService): terrain(inTerrain), io_service(inIOService){

    }

    ChunkedTerrainPtr terrain;
    boost::asio::io_service& io_service;

    IndexHandleCont ihandles;
    VertexHandleCont vhandles;

    boost::mutex vlodMutex;
    boost::mutex chunkMutex;
    boost::mutex OpenGLMutex;
};

//utility functions

/*
 *  =========  L O A D    V L O D  =========
 */

//loads vertex level if needed
static void loadVertexLevel(TerrainFragment& fragment, VertexLODInfo& vlodInfo, unsigned int vertexLevel, unsigned int fragmentWidth, boost::mutex& vlodMutex, boost::mutex& inOpenGLMutex){
    bool shouldLoad = false;

    //synchronously check if we need to load
    //and change vlod status
    if(fragment.vertexLODsStatuses[vertexLevel] == TerrainFragment::RAM){
        boost::lock_guard<boost::mutex> lock(vlodMutex);

        if(fragment.vertexLODsStatuses[vertexLevel] == TerrainFragment::RAM){
            fragment.vertexLODsStatuses[vertexLevel] = TerrainFragment::LOADING;
            shouldLoad = true;
        }
    }

    if(shouldLoad){

        std::vector<Vertex> vertices;
        bool someDataIsNotPresent = false;
        for(unsigned int i = 0; i < (vertexLevel + 1); ++i){
            if(fragment.vertexLODsStatuses[i] == TerrainFragment::UNLOADED ||
                    fragment.vertexLODs[i].size() == 0){
                someDataIsNotPresent = true;
                break;
            }

            //decompress vertices
            size_t previousSize = vertices.size();
            vertices.resize(previousSize + fragment.vertexLODs[i].size());
            for(unsigned int j = 0; j < fragment.vertexLODs[i].size(); ++j){
                vertices[previousSize + j].mCoord.x = fragment.vertexLODs[i][j].x;
                vertices[previousSize + j].mCoord.y = fragment.vertexLODs[i][j].y;
                vertices[previousSize + j].mCoord.z = fragment.vertexLODs[i][j].z;

                vertices[previousSize + j].mTexCoord.x = (float)fragment.vertexLODs[i][j].z / fragmentWidth;
                vertices[previousSize + j].mTexCoord.y = (float)fragment.vertexLODs[i][j].x / fragmentWidth;
                //recalculate y component of normal
                //we know that y is positive
                float normalY = sqrtf(1.0f - fragment.vertexLODs[i][j].normalX * fragment.vertexLODs[i][j].normalX - fragment.vertexLODs[i][j].normalZ * fragment.vertexLODs[i][j].normalZ);
                vertices[previousSize + j].mNormal = Vector3D(fragment.vertexLODs[i][j].normalX, normalY, fragment.vertexLODs[i][j].normalZ);
            }
        }

        if(!someDataIsNotPresent){
            //now load vertices to VRAM

            VBHandle handle;
            {
                boost::lock_guard<boost::mutex> lock(inOpenGLMutex);
                glGenBuffersARB(1, &handle);
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, handle);
                glBufferDataARB(GL_ARRAY_BUFFER_ARB, static_cast<GLsizeiptr>(sizeof(Vertex) * vertices.size()), (const void*)(&vertices[0]), GL_STATIC_DRAW_ARB);
                glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
            }

            //stop loadings of lower levels (synchronously)
            for(unsigned int i = 0; i < (vertexLevel + 1); ++i){
                //cancel LOADING process of lower levels (if any)
                //we assume, that LOADING means 'loading to VRAM' here
                if(i < vertexLevel && fragment.vertexLODsStatuses[i] == TerrainFragment::LOADING){
                    boost::lock_guard<boost::mutex> lock(vlodMutex);
                    if(i < vertexLevel && fragment.vertexLODsStatuses[i] == TerrainFragment::LOADING){
                        //cancel load
                        fragment.vertexLODsStatuses[i] = TerrainFragment::RAM;
                    }
                }
            }

            VBHandle oldHandle = vlodInfo.handle;
            
            bool transactionSuccess = false;
            {
                boost::lock_guard<boost::mutex> lock(vlodMutex);
                //it could be canceled
                if(fragment.vertexLODsStatuses[vertexLevel] == TerrainFragment::LOADING){
                    
                    fragment.vertexLODsStatuses[vertexLevel] = TerrainFragment::VRAM;

                    vlodInfo.handle = handle;
                    vlodInfo.level = vertexLevel;

                    //TODO: what about canceled loads???!
                    //for(unsigned int i = 0; i < (vertexLevel + 1); ++i)
                    //    fragment.vertexLODsStatuses[i] = TerrainFragment::VRAM;
                    
                    transactionSuccess = true;
                }
            }

            if(transactionSuccess){
                //unload old vlod
                //TODO:
                //WARNING: this handle may be in use at this point
                //         so we should not unload it immediately.
                if(oldHandle != 0){
                    //we sync with rendering thread. If oldHandle is in use at the moment
                    //this will ensure that rendering thread will finish its operations.
                    //(Rendering thread must update vlod handle every time)
                    boost::lock_guard<boost::mutex> lock(inOpenGLMutex);
                    if(glIsBufferARB(oldHandle))
                        glDeleteBuffersARB(1, &oldHandle);
                }
            }
            else{
                //remove previously loaded data
                boost::lock_guard<boost::mutex> lock(inOpenGLMutex);
                if(glIsBufferARB(handle))
                    glDeleteBuffersARB(1, &handle);
            }
        }
        else{
            std::cerr << "Aaaaa, I can't load VLOD to VRAM, it is not in RAM yet..." << std::endl;
            fragment.vertexLODsStatuses[vertexLevel] = TerrainFragment::RAM; //we can't load it now
        }
    } //end 'shouldLoad'
   
    //check (ref counter container may be empty) 
    if(vlodInfo.refCounters.empty()) vlodInfo.refCounters.resize(fragment.vertexLODs.size(), 0);

    //increase ref counter
    ++vlodInfo.refCounters[vertexLevel];
}



/*
 *  =========  U N L O A D   V L O D  =========
 */

// unload vertex lod (if present) and load the highest needed level to
// swap the current one
static void unloadVertexLevel(TerrainFragment& fragment, VertexLODInfo& vlodInfo, unsigned int vertexLevel, unsigned int fragmentWidth, boost::mutex& vlodMutex, boost::mutex& inOpenGLMutex){
    assert(vlodInfo.refCounters[vertexLevel] > 0);

    //TODO: this function may cause DESYNC!
    
    //first decrement ref counter
    --vlodInfo.refCounters[vertexLevel];

    //check whether we should unload this vertex level
    if(vlodInfo.refCounters[vertexLevel] == 0){
        
        int newLevel = -1;
        VertexLODInfo newVLODInfo = vlodInfo;
        
        //find lower level with positive ref counter value
        for(int i = vertexLevel - 1; i >= 0; --i){
            if(vlodInfo.refCounters[i] > 0){
                newLevel = i;
                break;
            }
        }
        
        //check for empty one
        if(vlodInfo.refCounters[vertexLevel] != 0) return;

        //change status
        {
            boost::lock_guard<boost::mutex> lock(vlodMutex);
            fragment.vertexLODsStatuses[vertexLevel] = TerrainFragment::TerrainFragment::RAM;
        }

        //cancel loadings
        for(int i = vertexLevel - 1; i >= 0; --i){
            boost::lock_guard<boost::mutex> lock(vlodMutex);
            if(fragment.vertexLODsStatuses[i] == TerrainFragment::LOADING)
                fragment.vertexLODsStatuses[i] = TerrainFragment::RAM;
        }

        if(newLevel >= 0){
            //load new level and unload old one
            //hack
            --newVLODInfo.refCounters[newLevel];
            loadVertexLevel(fragment, newVLODInfo, newLevel, fragmentWidth, vlodMutex, inOpenGLMutex);
        }
        else{
            //no more levels left in this fragment

            boost::lock_guard<boost::mutex> lock(inOpenGLMutex);

            //just unload level
            glDeleteBuffersARB(1, &vlodInfo.handle);

            newVLODInfo.handle = 0;
            newVLODInfo.level = 0;
        }

        //change data
        vlodInfo = newVLODInfo; //desync?
    }
}



/*
 *  ========= D O   L O A D =========
 */

static void doLoadChunk(TerrainChunkId inId, ChunkedTerrainPtr inTerrain, IndexHandleCont* ihandles, VertexHandleCont* vhandles, boost::mutex* inChunkMutex, boost::mutex* inVLODMutex, boost::mutex* inOpenGLMutex){
    assert(ihandles);
    assert(vhandles);
    assert(inVLODMutex);

    try{

        //first get binary data (it must be in RAM)
        TerrainFragment& fragment = inTerrain->getFragment(inId.getFragmentId());
        TerrainFragment::QuadTreeOfChunksPtr tree = fragment.tree; //TODO: desync?

        if(fragment.status != TerrainFragment::RAM || !tree){
            return;
        }

        //may throw
        TerrainFragment::ChunkData& chunkData = tree->getNode(inId.getQuadTreePos()).data;

        //canceled
        if(chunkData.status == TerrainFragment::RAM) return;
        //unloaded
        if(chunkData.status != TerrainFragment::LOADING || !chunkData.ptr){
            return;
        }

        TerrainChunkPtr chunkPtr = chunkData.ptr;
        if(!chunkPtr) return;

        TerrainChunk& chunk = *chunkPtr;

        IBHandle handle;
        {
            boost::lock_guard<boost::mutex> lock(*inOpenGLMutex);
            glGenBuffersARB(1, &handle);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, handle);
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, static_cast<GLsizeiptr>(sizeof(TerrainChunk::index_t) * chunk.indices.size()),
                                reinterpret_cast<const void*>(&chunk.indices[0]), GL_STATIC_DRAW_ARB);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        }


        //may be canceled
        if(chunkData.status != TerrainFragment::LOADING){
            //unload and exit
            glDeleteBuffersARB(1, &handle);
            return;
        }

        //check and load needed vertex levels
        loadVertexLevel(fragment, (*vhandles)[inId.getFragmentId()], chunkData.vertices, inTerrain->getFragmentWidth(), *inVLODMutex, *inOpenGLMutex);

        //check if it was canceled
        //critical section
        bool shouldUnload = false;
        {
            boost::lock_guard<boost::mutex> lock(*inChunkMutex);
        
            if(chunkData.status == TerrainFragment::LOADING){
                assert((*ihandles)[inId] == 0);
                (*ihandles)[inId] = handle;
                chunkData.status = TerrainFragment::VRAM;
            }
            else{
                //we should unload everything
                shouldUnload = true;
            }
        }

        if(shouldUnload){
            //unload index level
            glDeleteBuffersARB(1, &handle);

            //unload vertex level
            unloadVertexLevel(fragment, (*vhandles)[inId.getFragmentId()], chunkData.vertices, inTerrain->getFragmentWidth(), *inVLODMutex, *inOpenGLMutex);
        }

    }
    catch(const std::runtime_error& inRuntimeError){
        std::cerr << "VRAMLoadStrategy: doLoadChunk: std::runtime_error got: " << inRuntimeError.what() << std::endl;
        return;
    }
    catch(const std::exception& inException){
        std::cerr << "VRAMLoadStrategy: doLoadChunk: std::exception got: " << inException.what() << std::endl;
        return;
    }
    catch(...){
        std::cerr << "VRAMLoadStrategy: doLoadChunk: unknown exception got." << std::endl;
        return;
    }
}



//implementations of member functions

MyTerrainVRAMLoadStrategy::MyTerrainVRAMLoadStrategy(ChunkedTerrainPtr inTerrain, boost::asio::io_service& inIOService): mImpl(new MyTerrainVRAMLoadStrategy::Impl(inTerrain, inIOService)){

}

MyTerrainVRAMLoadStrategy::~MyTerrainVRAMLoadStrategy(){
    //clean
    assert(mImpl);

    for(IndexHandleCont::iterator iter = mImpl->ihandles.begin(); iter != mImpl->ihandles.end(); ++iter){
        if(iter->second != 0) unload(iter->first);
    }
    //vlods would be unloaded automatically
}


/*
 *  =========  S T A R T   A S Y N C   L O A D  =========
 */

void MyTerrainVRAMLoadStrategy::startAsyncLoad(TerrainChunkId inId){
    assert(mImpl);

    if(!mImpl->terrain) throw std::runtime_error("Can't load anything. You must bind ChunkedTerrain object first.");

    //first check if we can load
    ChunkedTerrain::iterator iter(inId.getFragmentId(), *mImpl->terrain);
    if(iter == mImpl->terrain->end()){
        //ignore
        assert(!"Wrong fragment id!");
        return;
    }

    //find chunk's position
    TerrainFragment& fragment = (*iter).second;
    if(fragment.status != TerrainFragment::RAM || !fragment.tree){
        assert(!"Fragment has been unloaded before chunk's load to VRAM finish.");
        return;
    }

    TerrainFragment::QuadTreeOfChunks::Node& node = fragment.tree->getNode(inId.getQuadTreePos());
    if(node.data.status != TerrainFragment::RAM) return;

    node.data.status = TerrainFragment::LOADING;

    //mImpl->io_service.post(boost::bind(doLoadChunk, inId, mImpl->terrain, &mImpl->ihandles, &mImpl->vhandles, &mImpl->chunkMutex, &mImpl->vlodMutex, &mImpl->OpenGLMutex));
    doLoadChunk(inId, mImpl->terrain, &mImpl->ihandles, &mImpl->vhandles, &mImpl->chunkMutex, &mImpl->vlodMutex, &mImpl->OpenGLMutex);
}

/*
 *  =========  U N L O A D =========
 */

void MyTerrainVRAMLoadStrategy::unload(TerrainChunkId inId){
    assert(mImpl);

    if(!mImpl->terrain) throw std::runtime_error("Can't load anything. You must bind ChunkedTerrain object first.");

    //first check if we can unload
    ChunkedTerrain::iterator iter(inId.getFragmentId(), *mImpl->terrain);
    if(iter == mImpl->terrain->end()){
        //ignore
        assert(!"Wrong fragment id!");
        return;
    }

    //find chunk's position
    TerrainFragment& fragment = (*iter).second;
    if(fragment.status != TerrainFragment::RAM || !fragment.tree){
        assert(!"Fragment has been unloaded before chunk's unload from VRAM finish.");
        return;
    }

    //may throw
    TerrainFragment::QuadTreeOfChunks::Node& node = fragment.tree->getNode(inId.getQuadTreePos());
    if(node.data.status == TerrainFragment::LOADING){
        //critical section
        {
            boost::lock_guard<boost::mutex> lock(mImpl->chunkMutex);
        
            if(node.data.status == TerrainFragment::LOADING){
                //stop loading process
                node.data.status = TerrainFragment::RAM;
                return;
            }
        }
    }

    if(node.data.status != TerrainFragment::VRAM) return;

    IBHandle handle = mImpl->ihandles[inId];

    node.data.status = TerrainFragment::RAM; //no desync
    if(handle != 0) glDeleteBuffersARB(1, &handle);
    mImpl->ihandles[inId] = 0;

    //unload childs
    for(int i = 0; i < 4; ++i){
        if(node.nodes[i] != 0){
            //recursive call
            unload(TerrainChunkId(inId.getFragmentId(), node.nodes[i]));
        }
    }

    //unload vlods!
    unloadVertexLevel(fragment, mImpl->vhandles[inId.getFragmentId()], node.data.vertices, mImpl->terrain->getFragmentWidth(), mImpl->vlodMutex, mImpl->OpenGLMutex);
}

//may return 0
IBHandle MyTerrainVRAMLoadStrategy::getIndexBuffer(TerrainChunkId inId) const{
    assert(mImpl);

    //TODO check?
    return mImpl->ihandles[inId];
}


VBHandle MyTerrainVRAMLoadStrategy::getVertexBuffer(TerrainFragmentId inId, unsigned int inLevel) const{
    assert(mImpl);

    VertexLODInfo& vinfo = mImpl->vhandles[inId];

    if(vinfo.level < inLevel) throw std::runtime_error("Requested vertex level is not loaded.");
    //TODO or should we return 0?


    return vinfo.handle;
}

boost::mutex& MyTerrainVRAMLoadStrategy::getOpenGLMutex(){
    assert(mImpl);

    return mImpl->OpenGLMutex;
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
