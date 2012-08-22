//FileManager.hpp

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


#ifndef LOAD_FILE_MANAGER_HPP__
#define LOAD_FILE_MANAGER_HPP__

/**
 * \class hydra::loading::FileManager
 * \brief Implementation of load manager for managing loadings from files.
 * 
 * FileManager loads data from files.
 * 1 manager object loads only 1 file.
 * More managers spawned for recursive loadings.
 *
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

#include "common/PimplPtr.hpp"
#include "loading/Manager.hpp"
#include <boost/noncopyable.hpp>

namespace hydra{

namespace loading{

class FileManager: public hydra::loading::Manager, private boost::noncopyable{

public:
    ///creates FileManager with cache
    explicit FileManager(hydra::loading::CachePtr inCache = hydra::loading::CachePtr());

    ///creates FileManager with path and cache
    FileManager(const std::string& inPath, hydra::loading::CachePtr inCache);
    
    virtual ~FileManager();

    ///sets path (current dir). Think twice before using this.
    void setPath(const std::string& inPath);

    ///sets Cache object
    virtual void setCache(hydra::loading::CachePtr inCache);

    ///returns Cache object pointer (const)
    virtual const hydra::loading::CachePtr getCache() const;

    ///returns Cache object pointer
    virtual hydra::loading::CachePtr getCache();
   
    ///function to get full id from relative id specified as parameter
    virtual std::string getFullId(const std::string& what) const; 

protected:
    ///creates stream from specified file. Also sets path first time.
    virtual IStreamPtr createStream(const std::string& whereFrom);

    ///returns file's type from its name. If impossible - returns empty string
    virtual std::string getType(const std::string& what) const;

    ///spawns new manager
    virtual hydra::loading::ManagerPtr spawnNewManager() const;

    ///sets busy flag to specified value
    virtual void setBusy(bool inBusyFlag);

    ///returns value of busy flag
    virtual bool isBusy() const;

private:
    ///pimpl idiom
    struct Impl;
    hydra::common::PimplPtr<Impl>::Type mImpl;
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
