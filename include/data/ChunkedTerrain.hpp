//ChunkedTerrain.hpp

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

#ifndef CHUNKED_TERRAIN_HPP__
#define CHUNKED_TERRAIN_HPP__

/**
 * \class hydra::data::ChunkedTerrain
 * \brief Stores terrain's fragments and chunks.
 *
 * Some other entities may load/unload data.
 * May be used to traverse all data. 
 *
 * \see hydra::common::QuadTree
 * \see hydra::rendering::TerrainLODManager
 * \see hydra::data::TerrainChunk
 * \see hydra::data::TerrainFragment
 *
 * \author A.V.Medvedev
 * \date 31.08.2010
 */

#include "common/SharedPtr.hpp"
#include "common/QuadTree.hpp"
#include "data/TerrainChunkId.hpp"
#include "data/TerrainChunk.hpp"
#include "data/TerrainFragmentId.hpp"
#include "data/TerrainFragment.hpp"

#include <map>
#include <boost/noncopyable.hpp>
#include <cassert>

namespace hydra{

namespace data{

class ChunkedTerrain: private boost::noncopyable{

public:


    ///builds empty terrain. Some other entities should fill it with data upon creation.
    inline ChunkedTerrain(){

    }

    ///returns reference to fragment with specified id.
    ///Throws std::out_of_range if data can't be retrieved
    hydra::data::TerrainFragment& getFragment(hydra::data::TerrainFragmentId inId);

    ///returns const reference to fragment with specified id.
    ///Throws std::out_of_range if data can't be retrieved
    const hydra::data::TerrainFragment& getFragment(hydra::data::TerrainFragmentId inId) const;

    ///sets fragment with specified id
    ///\warning Dangerous operation, use with caution.
    void setFragment(hydra::data::TerrainFragmentId inId, const hydra::data::TerrainFragment& inFragment);

    ///returns read-only reference for chunk with specified id.
    ///Throws std::out_of_range or std::runtime_error if data can't be retrieved.
    const hydra::data::TerrainChunk& getChunk(hydra::data::TerrainChunkId inId) const;
        
    ///puts chunk's data to terrain. Presence of previous data is not checked. Status will be changed to RAM.
    ///May throw std::out_of_range if you've specified wrong id
    ///and std::runtime_error if can't create quadtree because of wrong LOD number.
    void putChunk(hydra::data::TerrainChunkId inId, hydra::data::TerrainChunkPtr inChunk);

    ///sets status for specified chunk. Does now check or change any other data.
    ///You can't set status for chunk situated in unloaded fragment (you'll get std::out_of_range)
    void setStatus(hydra::data::TerrainChunkId inId, hydra::data::TerrainFragment::Status inStatus);

    ///sets fragment's width value
    inline void setFragmentWidth(unsigned int inFragmentWidth){
        mFragmentWidth = inFragmentWidth;
    }

    ///get fragment's width value
    inline unsigned int getFragmentWidth() const{
        return mFragmentWidth;
    }
    

    //see below
    class Iterator;
    class ConstIterator;

    ///returns iterator to begining
    inline Iterator begin(){
        return Iterator(mField.begin()->first, *this);
    }

    ///returns constant iterator to begining
    inline ConstIterator begin() const{
        return ConstIterator(mField.begin()->first, *this);
    }

    ///returns iterator to the end
    inline Iterator end(){
        return Iterator(mField.end()->first, *this);
    }

    ///returns constant iterator to the end
    inline ConstIterator end() const{
        return ConstIterator(mField.end()->first, *this);
    }

private:

    //Other threads may insert new fragments to this map dynamicaly even during our fragment
    //traversing. So FragmentCont's iterators MUST NOT be invalidated by insert operation.
    //std::map is such kind of container.
    typedef std::map<hydra::data::TerrainFragmentId, hydra::data::TerrainFragment> FragmentCont;
    unsigned int mFragmentWidth;
    FragmentCont mField;

//======================================================================
//=========================ITERATORS====================================
//======================================================================

public:


    /**
     * \brief Simple iterator for traversing fragments of ChunkedTerrain.
     *
     * It has simple functionality. But it is possible to traverse fragments
     * in terrain. You should not use same iterator entities for different ChunkedTerrains.
     */
    class Iterator{
    friend class ChunkedTerrain;
    
    public:
        ///Builds iterator pointed to specified terrain fragment.
        ///If specified id is wrong it will be pointed to end iterator.
        inline Iterator(hydra::data::TerrainFragmentId inFragId, ChunkedTerrain& inTerrain): 
                                              mIter(inTerrain.mField.find(inFragId)),
                                              mTerrain(inTerrain){
            
        }

        ///copy-constructor
        inline Iterator(const Iterator& inIter): mIter(inIter.mIter), mTerrain(inIter.mTerrain){

        }

        ///assign operator
        inline Iterator& operator=(const Iterator& inIter){
            if(this == &inIter) return *this;

            mIter = inIter.mIter;

            assert(&mTerrain == &inIter.mTerrain && "You can't assign iterators from different entities");
        }

        ///next element
        inline Iterator& operator++(){
            ++mIter;

            return *this;
        }

        ///\warning do not try to dereference iterator which points to end
        inline ChunkedTerrain::FragmentCont::value_type& operator*(){
            return *mIter;
        }

        ///equality comparison
        inline bool operator==(const Iterator& inIter) const{
            assert(&mTerrain == &inIter.mTerrain && "You can't compare iterators from different entities");
            return mIter == inIter.mIter;
        }

        ///nonequality comparison
        inline bool operator!=(const Iterator& inIter) const{
            assert(&mTerrain == &inIter.mTerrain && "You can't compare iterators from different entities");
            return mIter != inIter.mIter;
        }

        ///returns fragment id
        inline TerrainFragmentId getId() const{
            return mIter->first;
        }

    private:
        ChunkedTerrain::FragmentCont::iterator mIter;
        ChunkedTerrain& mTerrain;
    };


    /**
     * \brief Simple const iterator for traversing fragments of ChunkedTerrain.
     *
     * It has simple functionality. But it is possible to traverse fragments
     * in terrain. You should not use same iterator entities for different ChunkedTerrains.
     */
    class ConstIterator{
    friend class ChunkedTerrain;
    
    public:
        ///Builds iterator pointed to specified terrain fragment.
        ///If specified id is wrong it will be pointed to end iterator.
        inline ConstIterator(hydra::data::TerrainFragmentId inFragId, const ChunkedTerrain& inTerrain): 
                                              mIter(inTerrain.mField.find(inFragId)),
                                              mTerrain(inTerrain){
            
        }

        ///copy-constructor
        inline ConstIterator(const ConstIterator& inIter): mIter(inIter.mIter), mTerrain(inIter.mTerrain){

        }

        ///costructor from Iterator
        inline ConstIterator(const Iterator& inIter): mIter(inIter.mIter), mTerrain(inIter.mTerrain){

        }

        ///assign operator
        inline ConstIterator& operator=(const ConstIterator& inIter){
            if(this == &inIter) return *this;

            mIter = inIter.mIter;

            assert(&mTerrain == &inIter.mTerrain && "You can't assign iterators from different entities");
        }

        ///next element
        inline ConstIterator& operator++(){
            ++mIter;

            return *this;
        }

        ///\warning do not try to dereference iterator which points to end
        inline const ChunkedTerrain::FragmentCont::value_type& operator*() const{
            return *mIter;
        }

        ///equality comparison
        inline bool operator==(const ConstIterator& inIter) const{
            assert(&mTerrain == &inIter.mTerrain && "You can't compare iterators from different entities");
            return mIter == inIter.mIter;
        }

        ///nonequality comparison
        inline bool operator!=(const ConstIterator& inIter) const{
            assert(&mTerrain == &inIter.mTerrain && "You can't compare iterators from different entities");
            return mIter != inIter.mIter;
        }

        ///returns fragment id
        inline TerrainFragmentId getId() const{
            return mIter->first;
        }


    private:
        ChunkedTerrain::FragmentCont::const_iterator mIter;
        const ChunkedTerrain& mTerrain;
    };

   
    ///alias
    typedef Iterator iterator;
    ///alias
    typedef ConstIterator const_iterator;
};

///pointer (smart) to terrain object
typedef hydra::common::SharedPtr<ChunkedTerrain>::Type ChunkedTerrainPtr;

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
