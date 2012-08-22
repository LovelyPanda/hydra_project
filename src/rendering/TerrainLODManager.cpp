//TerrainLODManager.cpp

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

#include "rendering/TerrainLODManager.hpp"
#include "data/TerrainChunk.hpp"
#include "data/TerrainChunkId.hpp"
#include "common/QuadTree.hpp"
#include "rendering/Camera.hpp"
#include "data/ChunkedTerrain.hpp"

#include "data/TerrainFragmentId.hpp"
#include "data/TerrainFragment.hpp"

#include <cassert>
#include <list>

#include <boost/foreach.hpp>

#include <cmath>

using hydra::data::TerrainChunkId;
using hydra::data::TerrainChunk;
using hydra::data::TerrainFragment;
using hydra::data::TerrainFragmentId;
using hydra::data::ChunkedTerrainPtr;
using hydra::data::ChunkedTerrain;
using hydra::rendering::Camera;
using hydra::rendering::CameraPtr;
using hydra::rendering::TerrainLODManager;

typedef TerrainFragment::QuadTreeOfChunks QTree;

struct TerrainLODManager::Impl{

    ///simple function to calculate screen space error for given chunk and camera
    float calcError(const QTree::Node& node, const Camera& inCamera, const TerrainFragmentId& fragId) const{
        //we should convert aabb fron chunk data to global coordinates
        hydra::math::AABB chunkAABB(
                    node.data.aabb.getCorner() + 
                        hydra::math::Vector3D(static_cast<float>(fragId.getY() * terrain->getFragmentWidth()),
                                              0.0f, 
                                              static_cast<float>(fragId.getX() * terrain->getFragmentWidth())
                        ),
                    node.data.aabb.getVector()
        );
        float distance = chunkAABB.getShortestDistance(hydra::math::Point(inCamera.getPos().x(), inCamera.getPos().y(), inCamera.getPos().z()));
        //if distance is very smal return huge error value
        if(distance <= 0.0001f) return 100000.0f;

        return (node.data.maxError * parameters.perspectiveScallingFactor / distance);
    }

    ///recursive function to update the chunks in the tree.
    ///It stops when it founds totally unloaded chunk.
    void recursiveUpdate(QTree& tree, unsigned short nodeId, const Camera& inCamera, const TerrainFragmentId& fragId){
        QTree::Node& node = tree.getNode(nodeId);

        //for root chunk
        if(nodeId == 0){
            if(node.data.status == TerrainFragment::UNLOADED){
                //load to RAM
                RAMLoader->startAsyncLoad(TerrainChunkId(fragId, nodeId));
            }
            else if(node.data.status == TerrainFragment::RAM){
                //load to VRAM
                VRAMLoader->startAsyncLoad(TerrainChunkId(fragId, nodeId));
            }
        }

        //get child's statuses
        TerrainFragment::Status statuses[4];

        for(int i = 0; i < 4; ++i){
            if(node.nodes[i] == 0) return;
            else statuses[i] = tree.getNode(node.nodes[i]).data.status;
        }
        
        //get average status (all the statuses must be same, sometimes there may be few
        //LOADING statuses)
        TerrainFragment::Status averageStatus;
        for(int i = 0; i < 4; ++i){
            if(statuses[i] == TerrainFragment::LOADING){
                averageStatus = TerrainFragment::LOADING;
                break;
            }
            else{
                assert(i == 0 || averageStatus == statuses[i]);
                averageStatus = statuses[i];
            }
        }

        //we can skip this chunk if loading has not been finished yet
        if(averageStatus == TerrainFragment::LOADING){
            //check childs
            for(int i = 0; i < 4; ++i){
                assert(node.nodes[i] != 0);
                recursiveUpdate(tree, node.nodes[i], inCamera, fragId);
            }

            return;
        }
        
        //calculate screen space error for current chunk
        float error = calcError(node, inCamera, fragId);

        //std::cout << "error is: " << error << std::endl;

        //flag determines whether we should continue traversing childs
        bool shouldContinue = false;

        TerrainChunkId chunkIds[4];
        for(int i = 0; i < 4; ++i)
            chunkIds[i] = TerrainChunkId(fragId, node.nodes[i]);

        //---------- CHUNKS ARE UNLOADED------------------
        if(averageStatus == TerrainFragment::UNLOADED){
            //check if we should load them to RAM
            if(parameters.maxTolerableError * parameters.RAMLoadFactor < error){
                for(int i = 0; i < 4; ++i)
                    RAMLoader->startAsyncLoad(chunkIds[i]);
                shouldContinue = false;
            }
        }
        //---------- CHUNKS ARE IN RAM --------------------
        else if(averageStatus == TerrainFragment::RAM){
            //check if we should unload them
            if(parameters.maxTolerableError * parameters.RAMLoadFactor * parameters.unloadFactor > error){
                //unload (with all loaded childs)
                for(int i = 0; i < 4; ++i)
                    RAMLoader->unload(chunkIds[i]);
                shouldContinue = false;
            }
            else{
                //check if we should load them to VRAM
                if(parameters.maxTolerableError * parameters.VRAMLoadFactor < error){
                    for(int i = 0; i < 4; ++i)
                        VRAMLoader->startAsyncLoad(chunkIds[i]);
                }
                shouldContinue = true;
            }
        }
        //---------- CHUNK ARE IN VRAM --------------------
        else if(averageStatus == TerrainFragment::VRAM){
            //check if we should unload them to RAM
            shouldContinue = true;
            
            if(parameters.maxTolerableError * parameters.VRAMLoadFactor * parameters.unloadFactor > error){
                //unload from VRAM (together with all their children)
                for(int i = 0; i < 4; ++i)
                    VRAMLoader->unload(chunkIds[i]);
                
                //check if we should unload them (totally)
                if(parameters.maxTolerableError < parameters.unloadFactor * parameters.RAMLoadFactor * error){
                    for(int i = 0; i < 4; ++i)
                        RAMLoader->unload(chunkIds[i]);
                    shouldContinue = false;
                }
            }
        }

        //recursively update children (if needed)
        if(shouldContinue){
            for(int i = 0; i < 4; ++i){
                if(node.nodes[i] == 0) return;
                recursiveUpdate(tree, node.nodes[i], inCamera, fragId);
            }
        }

    }


    /// utility function to traverse subtree of specified quadtree
    /// and to add enabled chunks to list
    /// (the most detalised chunks present in VRAM)
    /// returns bool flag (true - childs were enabled, false - nothing was added to list)
    /// \warning function may be called only for enabled quads
    bool getEnabledQuads(const TerrainFragment::QuadTreeOfChunks& inTree, std::list<unsigned short>& enabledQuads, unsigned short inRoot, const Camera& inCam, const TerrainFragmentId& inFragId, const TerrainFragment& fragment) const{
        //we check whether childs are enabled
        const TerrainFragment::QuadTreeOfChunks::Node& root = inTree.getNode(inRoot);
        //end of tree check
        if(root.nodes[0] == 0 || root.nodes[1] == 0 || root.nodes[2] == 0 || root.nodes[3] == 0) return false;

        bool allChildsEnabled = true;
        for(int i = 0; i < 4; ++i){
            const TerrainFragment::QuadTreeOfChunks::Node& child = inTree.getNode(root.nodes[i]);    
            allChildsEnabled = allChildsEnabled && 
                ((child.data.status == TerrainFragment::VRAM) && 
                 (fragment.vertexLODsStatuses[child.data.vertices] == TerrainFragment::VRAM));
        }
        //check error metric
        float error = calcError(root, inCam, inFragId);

        //if all of them are enabled we recursively call this function for them
        if(allChildsEnabled){
            for(int i = 0; i < 4; ++i){
                bool areChildChildsEnabled = false;
                if(error > parameters.maxTolerableError){ 
                    areChildChildsEnabled = getEnabledQuads(inTree, enabledQuads, root.nodes[i], inCam, inFragId, fragment);
                }

                //if subchilds were not enabled we should add child
                //as chunk to render
                if(!areChildChildsEnabled) enabledQuads.push_back(root.nodes[i]);
            }
        }

        return allChildsEnabled;
    }


    ChunkedTerrainPtr terrain;
    TerrainLODManager::Parameters parameters;

    TerrainLODManager::FragmentLoadStrategyPtr fragmentLoader;
    TerrainLODManager::LoadStrategyPtr RAMLoader;
    TerrainLODManager::LoadStrategyPtr VRAMLoader;
};

TerrainLODManager::TerrainLODManager(TerrainLODManager::Parameters inParams, ChunkedTerrainPtr inTerrain, TerrainLODManager::FragmentLoadStrategyPtr inFragmentLoader, TerrainLODManager::LoadStrategyPtr inRAMLoader, TerrainLODManager::LoadStrategyPtr inVRAMLoader): mImpl(new TerrainLODManager::Impl()){
    mImpl->terrain = inTerrain;

    mImpl->parameters = inParams;
    mImpl->fragmentLoader = inFragmentLoader;
    mImpl->RAMLoader = inRAMLoader;
    mImpl->VRAMLoader = inVRAMLoader;
}

TerrainLODManager::~TerrainLODManager(){

}

void TerrainLODManager::updateParameters(TerrainLODManager::Parameters inParams){
    mImpl->parameters = inParams;
}

ChunkedTerrainPtr TerrainLODManager::getTerrain(){
    assert(mImpl);

    return mImpl->terrain;
}

void TerrainLODManager::initiateUpdate(const Camera& inCamera){
    assert(mImpl);

    unsigned int fragmentWidth = mImpl->terrain->getFragmentWidth();

    //we traverse all the enabled fragments using iterators and their trees    
    //and initiate asynchronous load/unload processes using Loaders
    for(ChunkedTerrain::const_iterator fragIter = mImpl->terrain->begin();
            fragIter != mImpl->terrain->end(); ++fragIter){
       
        const TerrainFragment& fragment = (*fragIter).second;
        //we skip empty fragments as they will be handled in next step
        if(!fragment.tree) continue;

        //For every fragment traverse its quadtree searching for chunks
        //that need to be [un]loaded.
        //Unload chunks immediately and initiate asynchronous load processes.
        mImpl->recursiveUpdate(*fragment.tree, 0, inCamera, fragIter.getId());
    }

    //we search for the fragments we can unload
    for(ChunkedTerrain::iterator fragIter = mImpl->terrain->begin();
            fragIter != mImpl->terrain->end(); ++fragIter){
        TerrainFragment& fragment = (*fragIter).second;
        
        //we can't unload unloaded fragment
        if(!fragment.tree) continue;

        hydra::math::AABB fragmentAABB(
                hydra::math::Point(
					static_cast<float>(fragIter.getId().getY() * fragmentWidth), 
					0.0f, 
					static_cast<float>(fragIter.getId().getX() * fragmentWidth)
				),
                hydra::math::Vector3D(static_cast<float>(fragmentWidth), 0.0f, static_cast<float>(fragmentWidth))
		);
        float distance = fragmentAABB.getDistance(hydra::math::Point(inCamera.getPos().x(), inCamera.getPos().y(), inCamera.getPos().z()));

        //totally unload fragment (even its metadata)
        if(distance > mImpl->parameters.maxVisibleDistance){
            //first unload chunks from VRAM
            mImpl->VRAMLoader->unload(TerrainChunkId(fragIter.getId(), 0));
            //unload chunks from RAM
            mImpl->RAMLoader->unload(TerrainChunkId(fragIter.getId(), 0));
            mImpl->fragmentLoader->unload(fragIter.getId());
        }
    }


    // We start traversing all the adjacent fragments (from camera position)
    // using coordinates (not iterators) searching for disabled ones.
    // This search is finished once we get too far from camera
    // This looks like that:
    //   __ __ __ __ __ __ 
    //  |  |  |  |  |  |  | Distance is too big to continue traversing
    //  |__|__|__|__|__|__| so we stopped at 24th square
    //  |24|9 |10|11|12|  |
    //  |__|__|__|__|__|__|
    //  |23|8 | 1| 2|13|  |
    //  |__|__|__|__|__|__|
    //  |22|7 |C | 3|14|  | C - fragment where observer is standing
    //  |__|__|__|__|__|__|
    //  |21|6 |5 | 4|15|  |
    //  |__|__|__|__|__|__|
    //  |20|19|18|17|16|  |
    //  |__|__|__|__|__|__|
    //  
    // We enable found fragments by adding them to ChunkedTerrain and by 
    // initiating asynchronous load of needed LODs.

    //first we find the nearest fragment's id
    TerrainFragmentId center(
            static_cast<int>(inCamera.getPos().z() / fragmentWidth),
            static_cast<int>(inCamera.getPos().x() / fragmentWidth)
    );

    //shows whether there were enabled fragments on the last spiral's circle
    bool wereEnabledFragments = false;

    //check center first
    {
        //calc distance from camera to current fragment
        hydra::math::AABB fragmentAABB(
                hydra::math::Point(static_cast<float>(center.getY() * fragmentWidth), 0.0f, static_cast<float>(center.getX() * fragmentWidth)), 
                hydra::math::Vector3D(static_cast<float>(fragmentWidth), 0.0f, static_cast<float>(fragmentWidth))
        );

        float distance = fragmentAABB.getDistance(hydra::math::Point(inCamera.getPos().x(), inCamera.getPos().y(), inCamera.getPos().z()));

        //check distance
        if(distance <= mImpl->parameters.maxVisibleDistance){
            //mark flag
            wereEnabledFragments = true;

            //check status
            ChunkedTerrain::iterator iter(center, *mImpl->terrain);
            if(iter != mImpl->terrain->end()){
                if((*iter).second.status == TerrainFragment::UNLOADED){
                    //initiate load
                    mImpl->fragmentLoader->startAsyncLoad(center);
                }
            }
        }
    }

    bool tooFar = false;
    TerrainFragmentId currentFragment = center;
    
    //shows how we should move to get proper spiral
    enum SpiralState{
        Y_INCREASE = 0,
        X_INCREASE = 1,
        Y_DECREASE = 2,
        X_DECREASE = 3
    };
    SpiralState currentState = Y_INCREASE;

    //current size of spiral
    int spiralWidth = 1;

    while(!tooFar){
        for(int i = 0; i < spiralWidth; ++i){
            //move
            switch (currentState){
                case Y_INCREASE: currentFragment.increaseY(); break;
                case X_INCREASE: currentFragment.increaseX(); break;
                case Y_DECREASE: currentFragment.decreaseY(); break;
                case X_DECREASE: currentFragment.decreaseX(); break;
                default: assert(!"currentState got wrong value!");
            }

            //calc distance from camera to current fragment
            hydra::math::AABB fragmentAABB(
                    hydra::math::Point(static_cast<float>(currentFragment.getY() * fragmentWidth), 0.0f, static_cast<float>(currentFragment.getX() * fragmentWidth)), 
                    hydra::math::Vector3D(static_cast<float>(fragmentWidth), 0.0f, static_cast<float>(fragmentWidth)));
            float distance = fragmentAABB.getDistance(hydra::math::Point(inCamera.getPos().x(), inCamera.getPos().y(), inCamera.getPos().z()));

            //check distance
            if(distance <= mImpl->parameters.maxVisibleDistance){
                //check status
                ChunkedTerrain::iterator iter(currentFragment, *mImpl->terrain);
                //such fragment does not exsist (may be we are out of the field)
                if(iter == mImpl->terrain->end()) continue;
                
                assert(iter != mImpl->terrain->end());
                if((*iter).second.status == TerrainFragment::UNLOADED){
                    //initiate load
                    mImpl->fragmentLoader->startAsyncLoad(currentFragment);
                }

                //mark flag
                wereEnabledFragments = true;
            }
             

        }

        //change spiral state
        if(currentState == X_DECREASE) currentState = Y_INCREASE;
        else currentState = static_cast<SpiralState>(currentState + 1);

        //increase spiral width if needed
        if(currentState == Y_DECREASE || currentState == Y_INCREASE){
            ++spiralWidth;
        }

        //should we continue?
        if(currentState == X_DECREASE){
            tooFar = !wereEnabledFragments;
            wereEnabledFragments = false;
        }
    
    }

}


TerrainLODManager::RenderChunkCont TerrainLODManager::getChunksToRender(const Camera& inCamera) const{
    assert(mImpl);

    RenderChunkCont chunks;

    //traverse all fragments
    for(ChunkedTerrain::const_iterator iter =  mImpl->terrain->begin();
            iter != mImpl->terrain->end(); ++iter){
        const TerrainFragment& fragment = (*iter).second;
        if(!fragment.tree || fragment.status != TerrainFragment::RAM) continue;

        if(fragment.tree->getRoot().data.status != TerrainFragment::VRAM) continue;

        //traverse tree and find the most detalised enabled chunks
        std::list<unsigned short> quadIdList;
        if(!mImpl->getEnabledQuads(*fragment.tree, quadIdList, 0, inCamera, (*iter).first, fragment))
            quadIdList.push_back(0);

        //push all ids to result list
        BOOST_FOREACH(unsigned short nextId, quadIdList){
            chunks.push_back(TerrainChunkId((*iter).first, nextId));
        }
    }
   
   return chunks; 
}

void TerrainLODManager::dropVRAM(){
    assert(mImpl);
    assert(mImpl->VRAMLoader);

    for(ChunkedTerrain::const_iterator iter =  mImpl->terrain->begin();
        iter != mImpl->terrain->end(); ++iter){
        //for each fragment unload root chunk from VRAM
        const TerrainFragment& fragment = (*iter).second;
        if(!fragment.tree || fragment.status != TerrainFragment::RAM) continue;
        if(fragment.tree->getRoot().data.status != TerrainFragment::VRAM) continue;
        
        mImpl->VRAMLoader->unload(TerrainChunkId((*iter).first, 0));
    }
}

float hydra::rendering::calculatePerspectiveScallingFactor(unsigned int inWidth, float inHorizontalFOV){
    return (inWidth / (2.0f * tan(inHorizontalFOV / 2.0f)));
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
