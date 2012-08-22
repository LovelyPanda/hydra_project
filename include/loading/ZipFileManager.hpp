//ZipFileManager.hpp

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


#ifndef ZIP_FILE_MANAGER_HPP__
#define ZIP_FILE_MANAGER_HPP__

/**
 * \class hydra::loading::ZipFileManager
 * \brief Class is a Manager implementation to create streams from Zip-archive.
 *
 * ZipFileManager is created for some zip-archive and it is able
 * to create streams from files inside it.
 * Current implementation uses libzip.
 *
 * \see hydra::loading::Manager
 *
 * \author A.V.Medvedev
 * \date 26.08.2010
 */

#include "loading/Manager.hpp"
#include "common/PimplPtr.hpp"
#include "common/SharedPtr.hpp"
#include <boost/noncopyable.hpp>

namespace hydra{

namespace loading{

class ZipFileManager: public hydra::loading::Manager, private boost::noncopyable{

public:
    ///creates ZipFileManager for specified zip archive (with optional cache)
    explicit ZipFileManager(const std::string& inPathToZipArchive, hydra::loading::CachePtr inCache = hydra::loading::CachePtr());

    ///destroys object
    virtual ~ZipFileManager();

    ///sets Cache object
    virtual void setCache(hydra::loading::CachePtr inCache);

    ///returns Cache object pointer (const)
    virtual const hydra::loading::CachePtr getCache() const;

    ///returns Cache object pointer
    virtual hydra::loading::CachePtr getCache();
   
    ///function to get full id from relative id specified as parameter
    virtual std::string getFullId(const std::string& what) const; 
    
    ///struct to represent zip-archive handle
    struct Archive;
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
    ///shared pointer to archive
    typedef hydra::common::SharedPtr<Archive>::Type ArchivePtr;

    ///private constructor for internal use
    explicit ZipFileManager(ArchivePtr inArchive, const std::string& inPathToArchive, const std::string& inPathInArchive, CachePtr inCache);



    //pimpl idiom
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
