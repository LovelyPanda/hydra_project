//DummyManager.hpp

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


#ifndef LOAD_DUMMY_MANAGER_HPP__
#define LOAD_DUMMY_MANAGER_HPP__

/**
 * \class hydra::loading::DummyManager
 * \brief DummyManager is dummy implementation of Manager's interface. Does nothing except logging load requests.
 * 
 * It may be usefull for using Loader's manually. You may pass DummyManager object
 * as Manager for such loading. All the requests for additional data loadings
 * from Loader you use will be ignored (DummyLoaders will be called, empty pointers will be returned without
 * any loadings) but saved. You will be able to get all the requests and accomplish them
 * manually after loading your main goal.
 * Cache is not used automatically, you may use it yourself if needed.
 *
 * \see hydra::loading::Manager
 * \see hydra::loading::DummyLoader
 *
 * \author A.V.Medvedev
 * \date 11.07.2010
 */

#include "common/SharedPtr.hpp"
#include "loading/Manager.hpp"
#include <vector>
#include <boost/noncopyable.hpp>

namespace hydra{

namespace loading{

class DummyManager: public hydra::loading::Manager, private boost::noncopyable{

public:

    ///dummy type identificator
    const std::string DUMMY_TYPE_ID;

    ///creates DummyManager
    DummyManager(): DUMMY_TYPE_ID("/?dummy_type?\\"){
        mMaxRequests = 100;
    }

    ///cleans everything
    virtual ~DummyManager(){

    }

    ///does not set Cache object
    virtual void setCache(hydra::loading::CachePtr inCache){

    }

    ///returns 0 pointer
    virtual const hydra::loading::CachePtr getCache() const{
        return hydra::loading::CachePtr();
    }

    ///returns 0 pointer
    virtual hydra::loading::CachePtr getCache(){
        return hydra::loading::CachePtr();
    }

    ///function to get full id (always returns same string)
    virtual std::string getFullId(const std::string& what) const{
        return what;
    }

    ///get current value of maximum requested ids in memory at a time
    inline size_t getMaxRequests() const{
        return mMaxRequests;
    }

    ///set new value of maximum requested ids in memory at a time
    inline void setMaxRequests(size_t inMaxRequests){
        mMaxRequests = inMaxRequests;
    }

    ///clear all saved reqests
    inline void clearRequests(){
        mRequests.clear();
    }

    ///get request with specified number
    inline std::string getRequest(size_t inIndex) const{
        if(inIndex < mRequests.size()) return mRequests[inIndex];
        else return "";
    }

    ///get total number of requsts
   inline size_t getRequestNum() const{
        return mRequests.size();
   }

protected:
    ///returnes empty pointer, but saves id for future usage
    virtual IStreamPtr createStream(const std::string& whereFrom){
        if(mRequests.size() < mMaxRequests)
            mRequests.push_back(whereFrom);
        return IStreamPtr();
    }

    ///always returns DUMMY_TYPE_ID
    virtual std::string getType(const std::string& what) const{
        return DUMMY_TYPE_ID;
    }

    ///does nothing
    virtual hydra::loading::ManagerPtr spawnNewManager() const{
        return hydra::loading::ManagerPtr();
    }

    ///does nothing
    virtual void setBusy(bool inBusyFlag){

    }

    ///always returns false
    virtual bool isBusy() const{
        return false;
    }

private:
    ///container for ids which were requested by loaders during loadings
    std::vector<std::string> mRequests;

    ///maximum requested ids in container
    size_t mMaxRequests;
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
