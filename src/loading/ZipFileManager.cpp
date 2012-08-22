//ZipFileManager.cpp

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

#include "loading/ZipFileManager.hpp"
#include "common/SharedPtr.hpp"
#include <zip.h>
#include <boost/noncopyable.hpp>

#include <stdexcept>
#include <string>
#include <algorithm>

#include <cassert>
#include <cstring>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/concepts.hpp>

using hydra::loading::ZipFileManager;
using hydra::loading::IStreamPtr;
using hydra::loading::ManagerPtr;

struct ZipFileManager::Archive: private boost::noncopyable{
    Archive(const std::string& inPath): mPtr(0){
        int errorCode = 0;
        mPtr = zip_open(inPath.c_str(), 0, &errorCode);

        if(mPtr == 0 || errorCode != 0){
            throw std::runtime_error("ZipFileManager: Error while opening zip-archive.");
        }
    }

    ~Archive(){
        if(mPtr) zip_close(mPtr);
    }

    zip* mPtr;
};

typedef hydra::common::SharedPtr<ZipFileManager::Archive>::Type ArchivePtr;

//struct for representing a file handle in zip-archive
struct ZipFile: private boost::noncopyable{
    ZipFile(ArchivePtr inArchive, const std::string& inPath){
        mPtr = zip_fopen(inArchive->mPtr, inPath.c_str(), 0);

        //check for error
        if(mPtr == 0){
            //get error string
            const char* zipError = zip_strerror(inArchive->mPtr);
            std::string error = "Error while openning zipped file for read: ";
            error += zipError;
            throw std::runtime_error(error);
        }

    }

    ~ZipFile(){
        if(mPtr){
            zip_fclose(mPtr);
            mPtr = 0;
        }
    }
    
    zip_file* mPtr;
};

typedef hydra::common::SharedPtr<ZipFile>::Type ZipFilePtr;

//boost source for reading from zip file
class ZipFileSource{
public:
    struct Category: public boost::iostreams::source_tag, 
                     public boost::iostreams::closable_tag{
    };

    typedef char char_type;
    typedef Category category;
    
    
    ///constructor
    ZipFileSource(ArchivePtr inArchive, const std::string& inPath): mArchive(inArchive), mFile(new ZipFile(inArchive, inPath)){

    }

    ///Source's read function. Tries to read data and stores it in outData pointer
    std::streamsize read(char* outData, std::streamsize inNum){
        //TODO: error handling must be here to separate EOF and errors
        return zip_fread(mFile->mPtr, outData, inNum);
    }

    void close(){
        mFile.reset();
    }

private:
    ArchivePtr mArchive;
    ZipFilePtr mFile;
};

//input stream based on ZipFileSource
typedef boost::iostreams::stream<ZipFileSource> ZipInputStream;


struct ZipFileManager::Impl{

    Impl(const std::string& inPathToZipArchive, hydra::loading::CachePtr inCache): mCache(inCache), mArchive(ArchivePtr()), mArchivePath(inPathToZipArchive), mBusyFlag(false){
        
    }

    //convert path separators to '/'
    inline void convertSeparators(std::string& inPath){
        std::replace(inPath.begin(), inPath.end(), '\\', '/');
    }

    inline bool isAbsolute(const std::string& inPath){
        return (inPath.size() > 0 && inPath[0] == '/');
    }

    inline bool isFull(const std::string& inPath){
        return (inPath.find('?') != std::string::npos);
    }

    std::string getUpper(const std::string& inPath){
        size_t lastSeparatorPos = inPath.find_last_of('/');
        if(lastSeparatorPos != std::string::npos){
            return inPath.substr(0, lastSeparatorPos);
        }
        else return std::string();
    }

    std::string getArchiveFromPath(const std::string& inPath){
        size_t separatorPos = std::string::npos;
        separatorPos = inPath.find_last_of('?');
        if(separatorPos == std::string::npos) throw std::runtime_error("Can't get archive path from specified id. Wrong format.");
        
        return inPath.substr(0, separatorPos);
    }

    std::string getFileFromPath(const std::string& inPath){
        size_t separatorPos = std::string::npos;
        separatorPos = inPath.find_last_of('?');
        if(separatorPos == std::string::npos) throw std::runtime_error("Can't get file path from specified id. Wrong format.");
        
        return inPath.substr(separatorPos + 1);
    }

    //deletes '..' and '.' stuff properly converting path string
    bool deleteRelativeLinks(std::string& inPath){
        size_t pos = std::string::npos;

        //delete '.' (current dir)
        while((pos = inPath.find("/./")) != std::string::npos){
            inPath.erase(pos, 2); //we delete '/.'
        }

        pos = std::string::npos;

        //delete '..' (parent dir)
        while((pos = inPath.find("/../")) != std::string::npos){

            if(pos == 0) return false;
            inPath.erase(pos, 3); //we delete '/..'

            //now we should delete parent directory (till preceding '/')
            size_t parentStartPos = inPath.find_last_of('/', pos - 1);
            if(parentStartPos == std::string::npos){
                parentStartPos = 0;
            }

            inPath.erase(parentStartPos, (pos - parentStartPos));
        }

        return true;
    }    

    hydra::loading::CachePtr mCache;
    ArchivePtr mArchive;
    const std::string& mArchivePath;
    std::string mPath;
    bool mBusyFlag;
};

//public constructor
ZipFileManager::ZipFileManager(const std::string& inPathToZipArchive, hydra::loading::CachePtr inCache): mImpl(new Impl(inPathToZipArchive, inCache)){
    mImpl->mArchive = ArchivePtr(new ZipFileManager::Archive(inPathToZipArchive));
}

//private constructor
ZipFileManager::ZipFileManager(ArchivePtr inArchive, const std::string& inPathToZipArchive, const std::string& inPathInArchive, hydra::loading::CachePtr inCache): mImpl(new Impl(inPathToZipArchive, inCache)){
    mImpl->mArchive = inArchive;
    mImpl->mPath = '/' + inPathInArchive;
}

ZipFileManager::~ZipFileManager(){
    
}


void ZipFileManager::setCache(hydra::loading::CachePtr inCache){
    assert(mImpl);
    mImpl->mCache = inCache;
}

const hydra::loading::CachePtr ZipFileManager::getCache() const{
    assert(mImpl);
    return mImpl->mCache;
}

hydra::loading::CachePtr ZipFileManager::getCache(){
    assert(mImpl);
    return mImpl->mCache;
}

std::string ZipFileManager::getFullId(const std::string& what) const{
    assert(mImpl);

    if(mImpl->isFull(what)) return what;

    std::string path = what;
    //first we should convert separators
    mImpl->convertSeparators(path);

    //next we should find out whether input path is relative or absolute
    //(absolute start with '/')
    if(mImpl->isAbsolute(path)){
        //change '/' sign with '?'
        assert(path[0] == '/');
        path[0] = '?';
        return mImpl->mArchivePath + path;
    }
    else{
        std::string newPathInArchive = mImpl->mPath;
        if(mImpl->mPath.size() == 1) newPathInArchive += path;
        else newPathInArchive += '/' + path;

        //we should '..' and '.' like links in our id
        if(!mImpl->deleteRelativeLinks(newPathInArchive)){
            //error acquired
            return what;
        }

        //change '/' sign with '?'
        assert(newPathInArchive[0] == '/');
        newPathInArchive[0] = '?';
        return mImpl->mArchivePath + newPathInArchive;
    }
}

IStreamPtr ZipFileManager::createStream(const std::string& whereFrom){
    assert(mImpl);
    //we should save path
    std::string filePath = mImpl->getFileFromPath(whereFrom);
    mImpl->mPath = '/' + mImpl->getUpper(filePath);

    //create stream
    IStreamPtr newStreamPtr (new ZipInputStream(mImpl->mArchive, filePath));
    
    if(!newStreamPtr->good()) throw std::runtime_error("ZipFileManager: error, created steam is in bad condition.");

    return newStreamPtr;
}

std::string ZipFileManager::getType(const std::string& what) const{
    assert(mImpl);

    size_t dotPos = what.find_last_of('.');
    if(dotPos == std::string::npos) return std::string();

    std::string extension = what.substr(dotPos + 1);

    std::transform(extension.begin(), extension.end(), extension.begin(), tolower);

    return extension;
}

hydra::loading::ManagerPtr ZipFileManager::spawnNewManager() const{
    assert(mImpl);

    ManagerPtr newManager(new ZipFileManager(mImpl->mArchive, mImpl->mArchivePath, mImpl->mPath, mImpl->mCache));
    return newManager;
}

void ZipFileManager::setBusy(bool inBusyFlag){
    assert(mImpl);

    mImpl->mBusyFlag = inBusyFlag;
}

bool ZipFileManager::isBusy() const{
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
