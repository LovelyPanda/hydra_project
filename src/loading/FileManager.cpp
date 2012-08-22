//FileManager.cpp

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


#include "loading/FileManager.hpp"
#include "loading/Cache.hpp"
#include <string>
#include <cassert>
#include <algorithm>
#include <cstring>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using hydra::loading::FileManager;
using hydra::loading::Manager;
using hydra::loading::IStreamPtr;
using hydra::loading::ManagerPtr;
using hydra::loading::Cache;
using hydra::loading::CachePtr;

//alias for convenience
namespace fs = boost::filesystem;

struct FileManager::Impl{

    Impl(): mBusyFlag(false){}
    Impl(const std::string& inPath, CachePtr inCache): mCache(inCache), mPath(inPath), mBusyFlag(false){}

    CachePtr mCache;
    fs::path mPath;
    bool mBusyFlag;
};

FileManager::FileManager(CachePtr inCache): mImpl(new Impl("", inCache)){

}

FileManager::FileManager(const std::string& inPath, CachePtr inCache): mImpl(new Impl(inPath, inCache)){

}

FileManager::~FileManager(){

}

void FileManager::setPath(const std::string& inPath){
    assert(mImpl);
    mImpl->mPath = fs::path(getFullId(inPath));
}

void FileManager::setCache(CachePtr inCache){
    assert(mImpl);
    mImpl->mCache = inCache;
}

const CachePtr FileManager::getCache() const{
    assert(mImpl);
    return mImpl->mCache;
}

CachePtr FileManager::getCache(){
    assert(mImpl);
    return mImpl->mCache;
}

//creates data stream here and remember path to file (directory)
IStreamPtr FileManager::createStream(const std::string& whereFrom){
    assert(mImpl);
    //if first time loading
    if(mImpl->mPath.empty()){
        mImpl->mPath = fs::system_complete(fs::path(whereFrom));
		mImpl->mPath = mImpl->mPath.parent_path();
        IStreamPtr fileStream(new fs::ifstream(whereFrom, fs::ifstream::in | fs::ifstream::binary));
        if(!fileStream->good()) throw std::runtime_error("FileManager: can't create stream from next file: " + whereFrom);
        return fileStream;
    }
    //if spawned from another manager
    else{
        //mPath must contain current working directory (not a path to some file)
        //whereFrom may be absolute only 
        //(Manager calls this member function with absolute id only)
        fs::path whereFromPath(whereFrom);
        std::string pathToLoad;
        
        pathToLoad = whereFrom;
        mImpl->mPath = whereFromPath.parent_path();
        
        //load
		IStreamPtr fileStream(new fs::ifstream(pathToLoad, fs::ifstream::in | fs::ifstream::binary));
        if(!fileStream->good()) throw std::runtime_error("FileManager: can't create stream from next file: " + pathToLoad);
        return fileStream;
    }
}

//returns file type. If it is impossible to find out from its name returns empty string
std::string FileManager::getType(const std::string& what) const{
    fs::path temp_path(what);
    //boost::filesystem adds dot of extension to string, but we don't need it
    std::string extension = temp_path.extension().string();
    if(extension.size() >= 1) extension = extension.substr(1);
    std::transform(extension.begin(), extension.end(), 
                   extension.begin(), tolower);
    return extension;
}

std::string FileManager::getFullId(const std::string& what) const{
    assert(mImpl);
    fs::path temp_path(what);
    //if already absolute
    if(temp_path.is_complete()) return temp_path.string();
    else return (mImpl->mPath / temp_path).string();
}

ManagerPtr FileManager::spawnNewManager() const{
    //mPath contains current working directory
    //pass it to new spawned manager
    assert(mImpl);
    ManagerPtr newMan(new FileManager(mImpl->mPath.string(), mImpl->mCache));
    return newMan;
}

void FileManager::setBusy(bool inBusyFlag){
    assert(mImpl);
    mImpl->mBusyFlag = inBusyFlag;
}

bool FileManager::isBusy() const{
    assert(mImpl);
    return mImpl->mBusyFlag;
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
