//MyTerrainVRAMLoadStrategy.hpp

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

#ifndef MY_TERRAIN_VRAM_LOADER_HPP__
#define MY_TERRAIN_VRAM_LOADER_HPP__

#include "rendering/TerrainLODManager.hpp"
#include "data/ChunkedTerrain.hpp"
#include "data/TerrainChunkId.hpp"
#include "data/TerrainFragmentId.hpp"
#include "common/PimplPtr.hpp"
#include "common/SharedPtr.hpp"
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

#include <GL/glew.h>
#include <GL/gl.h>

namespace boost{
    namespace asio{
        class io_service;
    }
}

class MyTerrainVRAMLoadStrategy: public hydra::rendering::TerrainLODManager::LoadStrategy, private boost::noncopyable{
public:
    typedef GLuint IndexBufferHandle;
    typedef GLuint VertexBufferHandle;

    ///constructs strategy
    MyTerrainVRAMLoadStrategy(hydra::data::ChunkedTerrainPtr inTerrain, boost::asio::io_service& inIOService);

    ///cleans everything up
    virtual ~MyTerrainVRAMLoadStrategy();

    ///main load function. Initiates asynchronous load process.
    ///Appropriate field of chunk's status in ChunkedTerrain object will be marked upon finish.
    ///Loaded data may be retrieved by getIndexBuffer
    virtual void startAsyncLoad(hydra::data::TerrainChunkId inId);

    ///unload function. Unload data with specified id.
    ///Appropriate field of chunk's status in ChunkedTerrain object will be marked upon finish.
    ///\warning Unload process MUST work synchronously for stability!
    virtual void unload(hydra::data::TerrainChunkId inId); 

    ///Retrieves handle to chunk's index data in video memory.
    ///Throws exception if data is not present at the moment.
    ///\warning Do not save this handle as it may be invalidated (by unload routine).
    /// Update handles as frequently as possible.
    virtual IndexBufferHandle getIndexBuffer(hydra::data::TerrainChunkId inId) const;

    ///Retrieves handle to fragment's vertex data in video memory.
    ///Throws exception if data is not present at the moment.
    virtual VertexBufferHandle getVertexBuffer(hydra::data::TerrainFragmentId inId, unsigned int inLevel) const;

    ///Returns reference to mutex object. 
    ///You should sync your rendering using this mutex.
    virtual boost::mutex& getOpenGLMutex();
    
private:
    //pimpl
    struct Impl;
    hydra::common::PimplPtr<Impl>::Type mImpl;
};

typedef hydra::common::SharedPtr<MyTerrainVRAMLoadStrategy>::Type MyTerrainVRAMLoadStrategyPtr;

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
