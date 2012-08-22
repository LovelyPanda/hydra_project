//TerrainLODManager.hpp

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

#ifndef TERRAIN_LOD_MANAGER_HPP__
#define TERRAIN_LOD_MANAGER_HPP__

/**
 * \class hydra::rendering::TerrainLODManager
 * \brief Class for managing terrain levels of details.
 *
 * Loads data, generates and unloads different levels of details and
 * chooses right one for rendering.
 * You must provide functors for loading chunks.
 *
 * \author A.V.Medvedev
 * \date 25.08.2010
 */

#include "data/TerrainChunkId.hpp"
#include "common/PimplPtr.hpp"
#include "common/SharedPtr.hpp"

#include <boost/noncopyable.hpp>

#include <list>

namespace hydra{

namespace data{

class ChunkedTerrain;
typedef hydra::common::SharedPtr<ChunkedTerrain>::Type ChunkedTerrainPtr;

}

namespace rendering{

class Camera;
typedef hydra::common::SharedPtr<Camera>::Type CameraPtr;

class TerrainLODManager: private boost::noncopyable{

public:

    /**
     * \brief Stores parameter values for TerrainLODManager
     *
     * Contains some coefficients which control TerrainLODManager
     * behaviour. Most of parameters are factors and error values
     * which are used to load/unload and set appropriate level
     * of details.
     */
    struct Parameters{
        /// viewportwidth / ( 2 * tan(horizontalfov / 2))
        /// See 'Chunked LOD' spec for more information.
        /// You can calculate it with utility function.
        /// \see calculatePerspectiveScallingFactor
        float perspectiveScallingFactor;

        /// maximum tolerable screen space error
        /// See 'Chunked LOD' spec for more information.
        float maxTolerableError;

        /** 
         * \brief unload factor
         *
         * To gain more stability during load/unload process
         * this factor is introduced. It is used to distance unload
         * position from load position.
         * This factor should be more than in interval (0.0, 1.0).
         */
        float unloadFactor;

        /**
         * \brief RAM loading factor
         *
         * Loading to RAM is calculated just like the calculation
         * of appropriate LOD is made. But the result (error)
         * is multiplied by this factor. This is done to force
         * the system to load data to RAM BEFORE it is used.
         * Of course this factor should be in interval (0.0, 1)
         */
        float RAMLoadFactor;

        /**
         * \brief video memory loading factor
         *
         * Loading to VRAM is calculated just like the calculation
         * of appropriate LOD is made. But the result (error)
         * is multiplied by this factor. This is done to force
         * the system to load data to VRAM BEFORE it is used.
         * Of course this factor should be in interval (RAMLoadFactor, 1)
         */
        float VRAMLoadFactor;

        /**
         * \brief fragment's maximum visible distance
         *
         * The fragment which is farther would not be loaded at all.
         * Fragment's metadata would be loaded when the observer
         * will approach to fragment at the distance lesser than 
         * this value.
         */
        float maxVisibleDistance;
    };

    /**
     * \brief interface for loader to RAM or VRAM entity
     *
     * This is an abstract interface for entity which is responsible
     * for loading terrain data to RAM or to VRAM.
     * Manager does not know anything about those things. So
     * you should implement your own load strategy.
     * You should implement at least 2 loaders:
     *  # From external source (hard drive, network, ...) to RAM
     *  # From RAM to VRAM
     *
     * Those loaders must be passed as arguments to constructor of manager.
     */
    class LoadStrategy{
    
    public:

        ///empty virtual destructor
        virtual ~LoadStrategy(){}

        ///main load function. Initiates asynchronous load process.
        ///Appropriate field of chunk's status in ChunkedTerrain object will be marked upon finish.
        virtual void startAsyncLoad(hydra::data::TerrainChunkId inId) = 0;
   
        ///unload function. Unload data with specified id and all its childs.
        ///Appropriate field of chunk's status in ChunkedTerrain object will be marked upon finish.
        ///\warning Unload process MUST work synchronously for stability!
        virtual void unload(hydra::data::TerrainChunkId inId) = 0; 
    };
    
    ///smart pointer to loader
    typedef hydra::common::SharedPtr<LoadStrategy>::Type LoadStrategyPtr;

    /**
     * \brief Abstract interface for fragment loader/unloader.
     *
     * It is capable of loading TerrainFragment objects. However
     * it does not capable of loading TerrainChunk data of fragment's 
     * tree.
     * You should implement your own loader.
     * The data is loading asynchronously. Proper TerrainFragment
     * object would be added to ChunkedTerrain object upon load.
     */
    class FragmentLoadStrategy{
    public:
        ///empty virtual destructor
        virtual ~FragmentLoadStrategy(){}

        ///fragment's load function. Some fragment's metadata would be
        ///loaded upon this call. Loading process is asynchronous.
        ///TerrainFragment (and its tree) will be built and inserted to
        ///ChunkedTerrain object. 
        virtual void startAsyncLoad(hydra::data::TerrainFragmentId inId) = 0;

        ///unloads fragment
        virtual void unload(hydra::data::TerrainFragmentId inId) = 0;
    };

    ///smart pointer to fragment loader
    typedef hydra::common::SharedPtr<FragmentLoadStrategy>::Type FragmentLoadStrategyPtr;


    /**
     * \brief Constructor. Builds manager.
     *
     * You should pass manager's parameters and 2 TerrainLODManager::LoadStrategy
     * implementations.
     *
     * \arg \b inParameters parameters of manager.
     * \arg \b inTerrain ChunkedTerrain object.
     * \arg \b inFragmentLoader load strategy for fragments
     * \arg \b inRAMLoader load strategy for chunks for loading to RAM
     * \arg \b inVRAMLoader load strategy for chunks for loading to video memory
     *
     * \see hydra::rendering::TerrainLODManager::LoadStrategy
     * \see hydra::rendering::TerrainLODManager::FragmentLoadStrategy
     * \see hydra::rendering::TerrainLODManager::Parameters
     */
    explicit TerrainLODManager(TerrainLODManager::Parameters inParameters, hydra::data::ChunkedTerrainPtr inTerrain, TerrainLODManager::FragmentLoadStrategyPtr inFragmentLoader, TerrainLODManager::LoadStrategyPtr inRAMLoader, TerrainLODManager::LoadStrategyPtr inVRAMLoader);

    ~TerrainLODManager();


    ///updates maanger's parameters
    void updateParameters(TerrainLODManager::Parameters inParameters);

    ///returns pointer to ChunkedTerrain object
    hydra::data::ChunkedTerrainPtr getTerrain();
    
    /**
     * \brief Initiates update process of terrain's state.
     *
     * Traverses all the needed fragments and chunks and initiates load/unload
     * proccesses. You should specify Camera object as manager needs it.
     * Parameters are used here.
     */
    void initiateUpdate(const hydra::rendering::Camera& inCamera);

    ///container for chunks to render (getChunksToRender return this)
    typedef std::list<hydra::data::TerrainChunkId> RenderChunkCont;

    /**
     * \brief Builds list of chunks to be rendered.
     *
     * Chunks are identified by ChunkIds. All of those chunks should be rendered.
     * However occlusion is not calculated for them.
     *
     * You should update terrain using update function before calling this function
     * if some chunks may be changed.
     * Routine traverses all fragments their chunks from low-detailed levels to high-detailed ones.
     * It finds needed chunks and builds list of their identificators
     */
    RenderChunkCont getChunksToRender(const hydra::rendering::Camera& inCamera) const;

    /**
     * \brief clears all the VRAM data
     *
     * You may call it while resizing or 'device lost' situation.
     */
    void dropVRAM();

private:
    //pimpl
    struct Impl;
    hydra::common::PimplPtr<Impl>::Type mImpl;
};

/**
 * \brief Utility function to calculate perspective scalling factor
 *
 * \arg \b inWidth viewport width
 * \arg \b inHorizontalFOV horizontal field of view in radians
 *
 * \see TerrainLODManager::Parameters
 */
float calculatePerspectiveScallingFactor(unsigned int inWidth, float inHorizontalFOV);

} //rendering namespace

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
