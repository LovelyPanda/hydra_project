//Cache.hpp

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


#ifndef LOADING_CACHE_HPP__
#define LOADING_CACHE_HPP__

/**
 * \class hydra::loading::Cache
 * \brief Template class for representing a %cache (composition of pools for storing loaded objects)
 *
 * CachePool stores different objects of 1 type which were loaded using this library.
 * Cache uses CachePools to store object of different types. All those types must be
 * put into boost::mpl's type sequence and passed to Cache template.
 * The idea of cache is : if needed object is present in CachePool
 * it won't be loaded second time but will be taken from cache.
 *
 * \author A.V.Medvedev
 * \date 07.06.2010
 */

//mpl's transform
#include <boost/mpl/transform.hpp>

//fusion
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/pair.hpp>

//convert from MPL to fusion
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>

#include <string>
#include "common/SharedPtr.hpp"
#include "loading/CachePool.hpp"
#include <map>
#include <cassert>

namespace hydra{

namespace loading{

template <typename TYPELIST>
class Cache{

public:

   /** \brief Get object with specified id.
     *
     *  Creates copy of object and returns pointer 
     *  or empty pointer if object is not present.
     */

    template <typename T>
    typename hydra::common::SharedPtr<T>::Type get(const std::string& inId);

    /**
     * \brief Get object with specified id.
     * 
     * Returns const pointer to object with specified id.
     * Does not create a copy. If you are not gonna modify 
     * result use this function to retrieve data from cache.
     */
    template <typename T>
    const typename hydra::common::SharedPtr<T>::Type getConst(const std::string& inId);

    /**
     *  Function adds new cached object by making 
     *  a copy (with use of copy-constructor) of specified one.
     *  It uses specified id to store object in pool*
     */
    template <typename T>
    void add(typename hydra::common::SharedPtr<T>::Type inPtr, const std::string& inId);

    ///grops cache content for specified type (drops all data at apropriate CachePool)
    template <typename T>
    void drop(){
        typename hydra::common::SharedPtr<CachePool<T> >::Type cachePool = getCachePool<T>();
        if(cachePool) cachePool->drop();
    }

    ///drops cache content for specified type and id (drops 1 object)
    template <typename T>
    void drop(const std::string& inId){
        typename hydra::common::SharedPtr<CachePool<T> >::Type cachePool = getCachePool<T>();
        if(cachePool) cachePool->drop(inId);
    }

    /**
     * Returns cache pool for specified type.
     * Creates new one if no any.
     */
    template <typename T>
    typename hydra::common::SharedPtr<CachePool<T> >::Type getCachePool(){
        //get type of T
        //if no any return empty pointer
        return boost::fusion::at_key<T>(mMap);
    }

    /**
     * Adds new CachePool object.
     */
    template <typename T>
    void addCachePool(){
        boost::fusion::at_key<T>(mMap) = typename hydra::common::SharedPtr<CachePool<T> >::Type(new CachePool<T>());
    }

private:
    //metafunction for generating pair from given type
    struct MakePair{
        template<typename T>
        struct apply{
            typedef boost::fusion::pair<T, typename hydra::common::SharedPtr<CachePool<T> >::Type > type;
        };
    };
    //generate pairs
    typedef typename boost::mpl::transform<TYPELIST, MakePair>::type PairedCacheableTypes;
    //convert to fusion::map
    typedef typename boost::fusion::result_of::as_map<PairedCacheableTypes>::type CacheMap;

    //the map itself
    CacheMap mMap;
};

///pointer (smart) to Cache object
///typedef hydra::common::SharedPtr<Cache>::Type CachePtr;

template <typename TYPELIST>
template <typename T>
typename hydra::common::SharedPtr<T>::Type Cache<TYPELIST>::get(const std::string& inId){
    //first get appropriate cache pool
    typename hydra::common::SharedPtr<CachePool<T> >::Type cachePool = getCachePool<T>();
    if(!cachePool) return typename hydra::common::SharedPtr<T>::Type();
    return cachePool->get(inId);
}

template <typename TYPELIST>
template <typename T>
const typename hydra::common::SharedPtr<T>::Type Cache<TYPELIST>::getConst(const std::string& inId){
    //first get appropriate cache pool
    typename hydra::common::SharedPtr<CachePool<T> >::Type cachePool = getCachePool<T>();
    if(!cachePool) return typename hydra::common::SharedPtr<T>::Type();
    return cachePool->getConst(inId);
}

template <typename TYPELIST>
template <typename T>
void Cache<TYPELIST>::add(typename hydra::common::SharedPtr<T>::Type inPtr, const std::string& inId){
    //first get appropriate cache pool
    typename hydra::common::SharedPtr<CachePool<T> >::Type cachePool = getCachePool<T>();
    if(!cachePool) addCachePool<T>();
    cachePool = getCachePool<T>();
    assert(cachePool);
    cachePool->add(inPtr, inId);
}

} //loading namespace

} //hydra

#include "data/CacheableTypes.hpp"

namespace hydra{

namespace loading{

///pointer to Cache's instance
typedef hydra::common::SharedPtr<Cache<data::CacheableTypes> >::Type CachePtr;

}

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
