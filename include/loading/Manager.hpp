//Manager.hpp

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


#ifndef LOAD_MANAGER_HPP__
#define LOAD_MANAGER_HPP__

/**
 * \class hydra::loading::Manager
 * \brief Abstract manager for loading different stuff.
 *  
 * Manager controls loading operations:
 *  - creates input streams with data to load 
 *  - chooses right Loader object from factories
 *  - passes streams to loaders
 *  - spawns more managers to load additional data (if needed by some loader)
 * 
 * Real managers must be derived from this class.
 * Example of managers: FileManager (loads data from files),
 * NetworkManager (loads data from network connection/connections),
 * ZippedDirectoryLoader (loads data from zipped archive),  etc.
 *
 * Some loaders may want to load additional data from some external sources.
 * In this case loader asks its manager to load this data. In such way we gain full 
 * separation of data sources from data. Loaders may load data from any source (if we have apropriate manager).
 * To add new source (networking, compressed data, etc) we shouldn't change any Loader's code
 * but create new Manager implementation.
 *
 * Every manager controls only 1 loading process. To make recursive loadings
 * manager spawns its children. However some implementations may not spawn
 * new managers if they don't need it. Override isBusy function to return always 'false'
 * and all the work of loading the object will be done by 1 manager.
 *
 * \warning all the needed factories must be inited before using a manager
 * 
 * \todo Decompose Manager into 2 entities: stream creator and manager (think about it!)
 *
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

#include "common/SharedPtr.hpp"
#include <string>
#include <stdexcept>
#include "common/Singleton.hpp"
#include "common/Factory.hpp"
#include "loading/Loader.hpp"
#include <istream>
#include "loading/Cache.hpp"

namespace hydra{

namespace loading{

///typedef for pointer (smart) to input stream
typedef hydra::common::SharedPtr<std::istream>::Type IStreamPtr;
class Manager;

///typedef for pointer (smart) to Manager object.
typedef hydra::common::SharedPtr<Manager>::Type ManagerPtr;

class Manager{

public:
    virtual ~Manager(){}

    /**
     *  Load function makes all needed actions to load hole object and return object.
     *  'load' is template function so you should specify type of object to load.
     *  Also you should identify what object to load by string parameter (id).
     *  Implementaitions of Manager should interpret that string id as they like.
     */
    template <typename T>
    typename hydra::common::SharedPtr<T>::Type load(const std::string& whatToLoad);

    ///function returns Cache object pointer
    virtual const hydra::loading::CachePtr getCache() const = 0;

    ///function returns Cache object pointer (const)
    virtual hydra::loading::CachePtr getCache() = 0;

    ///function sets new Cache object
    virtual void setCache(hydra::loading::CachePtr inCache) = 0;

    ///function to get full id from relative id specified as parameter
    ///full id may be used as unique identifier for loaded object (for caching)
    virtual std::string getFullId(const std::string& what) const = 0;

protected:

    ///abstract declaration of function which creates streams
    virtual IStreamPtr createStream(const std::string& whereFrom) = 0;
    
    ///abstract declaration of function which obtains type of source from string id
    virtual std::string getType(const std::string& what) const = 0;
    
    ///abstract declaration of function which spawns new manager to load 'sub-objects'
    virtual ManagerPtr spawnNewManager() const = 0;

    ///abstract declaration of function which marks manager as busy
    virtual void setBusy(bool inBusyFlag) = 0;

    ///abstract declaration of function which shows whether manager is busy or can it be 
    ///used to load object
    virtual bool isBusy() const = 0;

};

///implementation of load function
template <typename T>
typename hydra::common::SharedPtr<T>::Type hydra::loading::Manager::load(const std::string& whatToLoad){
        //get full id of object ot load
        std::string fullId = getFullId(whatToLoad);
        //if this manager is already loading some object and busy
        if(isBusy()){
            //spawn new manager and tell it to load
            hydra::loading::ManagerPtr newMan = spawnNewManager();
            return newMan->load<T>(fullId);
        }

        //otherwise mark this manager as busy
        setBusy(true);

        //check if there is needed object in cache
        hydra::loading::CachePtr cache = getCache();
        if(cache){
            typename hydra::common::SharedPtr<T>::Type cachedObject = cache->get<T>(fullId);
            if(cachedObject) return cachedObject;
        }
        //if there is no apropriate cached object, we have to load
        
        //create stream
        IStreamPtr dataStream = createStream(fullId);
        
        //interpret string parameter and get type (for factory) of object to load
        std::string type = getType(fullId);
        typename hydra::common::SharedPtr< hydra::loading::Loader<T> >::Type loader = 
            hydra::common::Singleton<common::Factory< hydra::loading::Loader<T> > >::instance()->get(type);
        
        //if there are no loaders in factory for specified type throw error
        if(!loader) 
            throw std::runtime_error("LoadingManager: ERROR: unknown type of data \"" +
                type + "\" at source: " + fullId);
        
        //load from stream using proper loader
        typename hydra::common::SharedPtr<T>::Type loadedObject = loader->load(*dataStream, *this, type);
        if(cache) cache->add<T>(loadedObject, fullId);
        return loadedObject;
}

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
