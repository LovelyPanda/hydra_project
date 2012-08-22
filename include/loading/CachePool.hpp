//CachePool.hpp

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


#ifndef LOADING_CACHE_POOL_HPP__
#define LOADING_CACHE_POOL_HPP__

/**
 * \class hydra::loading::CachePool
 * \brief Class for representing a %cache pool (pool for storing loaded objects)
 *
 * CachePool stores different objects of same type which were loaded using this library.
 * CachePool object is used by Cache. Objects have their ids.
 * If needed object is present in CachePool it won't be loaded 
 * second time but will be taken from cache.
 * Cache stores copyes of loaded data as it may be changed after loading
 * by client's code.
 *
 * \author A.V.Medvedev
 * \date 07.06.2010
 */

#include "common/SharedPtr.hpp"
#include <string>
#include <map>

namespace hydra{

namespace loading{

template <typename T, typename Id = std::string>
class CachePool{

public:
    ///type of pointer (smart) to argument type object
    typedef typename hydra::common::SharedPtr<T>::Type TPtr;

    /** \brief Get object with specified id.
     *
     *  Creates copy of cached object (if found) and returns pointer 
     *  or empty pointer if object is not present.
     */
    TPtr get(const Id& inId){
        TPtr result = mCachePool[inId];
        //make a copy if found
        if(result) result = TPtr(new T(*result));
        return result;
    }

    /**
     * \brief Get object with specified id.
     * 
     * Returns const pointer to object with specified id.
     * Does not create a copy. If you are not gonna modify 
     * result use this function to retrieve data from cache.
     */
    const TPtr getConst(const Id& inId){
        return mCachePool[inId];
    }

    /// Function adds new cached object by making 
    /// a copy (with use of copy-constructor) of specified one.
    /// It uses specified id to store object in pool
    void add(TPtr inPtr, const Id& inId){
        //first we make a copy (need copy-constructor)
        TPtr copy(new T(*inPtr));
        mCachePool[inId] = copy;
    }

    ///function clears all the objects which were cached so far
    void drop(){
        mCachePool.clear();
    }

    ///drops object with specified id if any
    void drop(const Id& inId){
        mCachePool[inId].reset();
    }

private:
    ///container type for pool
    typedef std::map<Id, TPtr> CachePoolCont;
    ///container for pool
    CachePoolCont mCachePool;
};

} //loading namespace

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
