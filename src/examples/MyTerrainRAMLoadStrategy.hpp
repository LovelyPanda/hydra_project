//MyTerrainRAMLoadStrategy.hpp

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

#ifndef MY_TERRAIN_RAM_LOADER_HPP__
#define MY_TERRAIN_RAM_LOADER_HPP__

/**
 * \class MyTerrainRAMLoadStrategy
 *
 * Implementation of hydra::rendering::TerrainLODManager::LoadStrategy
 * and hydra::rendering::TerrainLODManager::FragmentLoadStrategy to
 * load data (fragments and chunks) from some external sources to RAM.
 * It also uses hydra::loading::Loader implementation.
 *
 * \see hydra::rendering::TerrainLODManager::LoadStrategy
 * \see hydra::rendering::TerrainLODManager::FragmentLoadStrategy
 *
 * \author A.V.Medvedev
 * \date 22.09.2010
 */

#include "rendering/TerrainLODManager.hpp"
#include "data/ChunkedTerrain.hpp"
#include "data/TerrainChunkId.hpp"
#include "common/PimplPtr.hpp"
#include "common/SharedPtr.hpp"
#include <boost/noncopyable.hpp>

namespace boost{
    namespace asio{
        class io_service;
    }
}

class MyTerrainRAMLoadStrategy: public hydra::rendering::TerrainLODManager::LoadStrategy, 
                          public hydra::rendering::TerrainLODManager::FragmentLoadStrategy,  
                          private boost::noncopyable{

public:
    ///constructs loader based on specified input service
    MyTerrainRAMLoadStrategy(hydra::data::ChunkedTerrainPtr inTerrain, boost::asio::io_service& inIOService, const std::string& inPath);

    ///cleans everything up
    virtual ~MyTerrainRAMLoadStrategy();

    ///main load function. Initiates asynchronous load process.
    ///Appropriate field of chunk's status in ChunkedTerrain object will be marked upon finish.
    virtual void startAsyncLoad(hydra::data::TerrainChunkId inId);

    ///fragment's load function. Some fragment's metadata would be
    ///loaded upon this call. Loading process is asynchronous.
    ///TerrainFragment (and its tree) will be built and inserted to
    ///ChunkedTerrain object. 
    virtual void startAsyncLoad(hydra::data::TerrainFragmentId inId);

    ///unload function. Unload data with specified id.
    ///Appropriate field of chunk's status in ChunkedTerrain object will be marked upon finish.
    ///\warning Unload process MUST work synchronously for stability!
    virtual void unload(hydra::data::TerrainChunkId inId); 

    ///unloads fragment's metadata
    virtual void unload(hydra::data::TerrainFragmentId inId);

    unsigned int getTerrainWidth() const;
    unsigned int getTerrainHeight() const;

private:
    //pimpl
    struct Impl;
    hydra::common::PimplPtr<Impl>::Type mImpl;
};

///smart pointer to MyTerrainRAMLoadStrategy
typedef hydra::common::SharedPtr<MyTerrainRAMLoadStrategy>::Type MyTerrainRAMLoadStrategyPtr;

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
