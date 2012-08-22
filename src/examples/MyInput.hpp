//MyInput.hpp

#ifndef MY_INPUT_HPP__
#define MY_INPUT_HPP__

//boost headers
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

//stl headers
#include <stdexcept>
#include <string>
#include <istream>
#include <iosfwd>
#include <iostream>
#include <fstream>

//libzip
#include <zip.h>


typedef std::istream IStream;
typedef hydra::common::SharedPtr<IStream>::Type IStreamPtr;

class FileSystem{
public:
    virtual ~FileSystem(){

    }

    virtual IStreamPtr createInputStream(const std::string& inPath) = 0;
};

typedef hydra::common::SharedPtr<FileSystem>::Type FileSystemPtr;

class ZipSource{
public:
    struct Category: public boost::iostreams::source_tag, 
                     public boost::iostreams::closable_tag{

    };
    typedef char char_type;
    typedef Category category;

    ZipSource(zip_file* inZipFile, boost::mutex* inMutex): mZipFile(inZipFile), mMutex(inMutex){

    }

    void close(){
        if(mZipFile){
            boost::lock_guard<boost::mutex> lock(*mMutex);
            
            if(zip_fclose(mZipFile) != 0){
                std::cerr << "Error while closing zip file stream in archive." << std::endl;
            }
            mZipFile = 0;
        }
    }

    std::streamsize read(char* buffer, std::streamsize upTo){
        assert(mZipFile);
        boost::lock_guard<boost::mutex> lock(*mMutex);
        return zip_fread(mZipFile, buffer, upTo);
    }

private:
    zip_file* mZipFile;
    boost::mutex* mMutex;
};

typedef boost::iostreams::stream<ZipSource> ZipInputStream;
typedef hydra::common::SharedPtr<ZipInputStream>::Type ZipInputStreamPtr;

class ZipFileSystem: public FileSystem, private boost::noncopyable{
public:
    ZipFileSystem(const std::string& inPathToArchive){
        //open archive
        int errorCode = 0;
        mArchive = zip_open(inPathToArchive.c_str(), 0, &errorCode);
        if(mArchive == 0 || errorCode != 0){
            throw std::runtime_error("Error while openning zip-archive");
        }
    }

    virtual ~ZipFileSystem(){
        if(mArchive){
            boost::lock_guard<boost::mutex> lock(mArchiveMutex);
            if(zip_close(mArchive) != 0){
                std::cerr << "Error while closing archive!" << std::endl;
            }
            mArchive = 0;
        }
    }

    virtual IStreamPtr createInputStream(const std::string& inPath){
        assert(mArchive);

        zip_file* file;
        {
            boost::lock_guard<boost::mutex> lock(mArchiveMutex);
            file = zip_fopen(mArchive, inPath.c_str(), 0);
        }

        if(!file) std::cerr << "Can't create stream: " << inPath << std::endl;
        if(!file) throw std::runtime_error("Can't create stream from archive.");

        IStreamPtr result(new ZipInputStream(file, &mArchiveMutex));
        return result;
    }

private:
    boost::mutex mArchiveMutex;
    zip* mArchive;
};

class FileFileSystem: public FileSystem, private boost::noncopyable{
public:
    FileFileSystem(const std::string& inPath = "."): mPath(inPath){
        if(!boost::filesystem::is_directory(inPath.c_str())){
            throw std::runtime_error("Can't open directory.");
        }
    }

    virtual IStreamPtr createInputStream(const std::string& inPath){
        hydra::common::SharedPtr<std::ifstream>::Type fstream(new std::ifstream((mPath + "/" + inPath).c_str(), std::fstream::in | std::fstream::binary));
        if(!fstream->good()){
            std::cerr << "Can't open file: " << mPath << '/' << inPath << std::endl;
            throw std::runtime_error("Can't open file for reading.");
        }
        return fstream;
    }

private:
    std::string mPath;
};

#endif
