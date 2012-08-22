//MyTerrainRAMLoadStrategy.cpp

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

#include "MyTerrainRAMLoadStrategy.hpp"

//boost headers
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
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
#include <map>

//c headers
#include <cassert>

//3rd party headers
#include <zip.h>
#include "pugixml.hpp"

using hydra::data::ChunkedTerrain;
using hydra::data::ChunkedTerrainPtr;
using hydra::data::TerrainFragment;
using hydra::data::TerrainFragmentPtr;
using hydra::data::TerrainFragmentId;
using hydra::data::TerrainChunk;
using hydra::data::TerrainChunkPtr;
using hydra::data::TerrainChunkId;

typedef TerrainFragment::QuadTreeOfChunksPtr QTreePtr;
typedef TerrainFragment::QuadTreeOfChunks QTree;

//=============INPUT STUFF=====================
//=============================================

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
//============END OF INPUT STUFF===============
//=============================================

struct MetaData{
    struct Chunk{
        unsigned int indicesNum;
        std::string dataFile;
    };

    struct VLOD{
        unsigned short refCounter;
        unsigned int vertexNum;
        std::string fileName;
    };

    struct Fragment{
        std::string metaFile;
        std::vector<MetaData::VLOD> vlods;
        std::vector<MetaData::Chunk> chunks;
    };

    typedef std::map<TerrainFragmentId, MetaData::Fragment> FragmentCont;

    unsigned int fragmentWidth;
    unsigned int terrainWidth;
    unsigned int terrainHeight;
    FragmentCont fragments;
};

typedef hydra::common::SharedPtr<MetaData>::Type MetaDataPtr;

struct MyTerrainRAMLoadStrategy::Impl{
    Impl(hydra::data::ChunkedTerrainPtr inTerrain, boost::asio::io_service& inIOService, const std::string& inPath): terrain(inTerrain), io_service(inIOService){
        //first we check path
        //it may be a path to zip-archive or to some directory
        if(boost::filesystem::is_directory(inPath.c_str())){
            //directory
            fileSystem = FileSystemPtr(new FileFileSystem(inPath));
        }
        else{
            //zip archive
            fileSystem = FileSystemPtr(new ZipFileSystem(inPath));
        }

        //now we should read main.meta file
        metaData = readMainMeta(fileSystem->createInputStream("main.meta"));

        terrain->setFragmentWidth(metaData->fragmentWidth);

        //fill chunked terrain with empty fragments
        TerrainFragment emptyFragment;
        emptyFragment.status = TerrainFragment::UNLOADED;
        for(MetaData::FragmentCont::const_iterator iter = metaData->fragments.begin(); iter != metaData->fragments.end(); ++iter){
            terrain->setFragment(iter->first, emptyFragment);
        }
    }

    MetaDataPtr readMainMeta(IStreamPtr inStream){
        if(!inStream->good()) throw std::runtime_error("Can't read main.meta file (bad stream)");

        MetaDataPtr result = MetaDataPtr(new MetaData());

        //use pugixml to parse xml and fill MetaData
        pugi::xml_document document;
        pugi::xml_parse_result parseResult = document.load(*inStream);
        if(!parseResult){
            std::cerr << "Error while parsing xml: " << parseResult.description() << std::endl;
            throw std::runtime_error("xml parsing error!");
        }

        pugi::xml_node root = document.root().child("landscape");
        
        if(!root){
            throw std::runtime_error("Wrong xml format. Root node must have 'landscape' name.");
        }

        pugi::xml_node_iterator iter = root.begin();

        //first node must be a decription node
        pugi::xml_node descr = *iter;
        if(std::string(descr.name()) != "description"){
            throw std::runtime_error("Wrong xml format. First node must have 'description' name.");
        }
        unsigned int fragmentWidth = descr.attribute("fragment_width").as_uint();
        result->fragmentWidth = fragmentWidth;
        result->terrainWidth = descr.child("size").attribute("x").as_uint();
        result->terrainHeight = descr.child("size").attribute("y").as_uint();
        //unsigned int fragmentNum = descr.attribute("fragment_num").as_uint();
        //ignore other attributes

        //read terrain_fragment's
        while(++iter != root.end()){
            if(std::string(iter->name()) != "terrain_fragment"){
                throw std::runtime_error("Unknown node found in xml file. terrain_fragment expected.");
            }

            MetaData::Fragment fragmentMetaData;
            TerrainFragmentId id(iter->attribute("x").as_int(), iter->attribute("y").as_int());
            fragmentMetaData.metaFile = iter->child("meta").attribute("filename").value();
            result->fragments[id] = fragmentMetaData;
        }

        return result;
    }

    MetaDataPtr metaData;
    hydra::data::ChunkedTerrainPtr terrain;
    boost::asio::io_service& io_service;
    FileSystemPtr fileSystem;

    boost::mutex fragmentMutex;
    boost::mutex chunkMutex;
    boost::mutex vlodMutex;
};

static TerrainChunkPtr getChunk(TerrainChunkId inId, const MetaData::Fragment& meta, FileSystemPtr fileSystem){
    TerrainChunkPtr chunk(new TerrainChunk());

    //get filename
    //.tfmeta directory + '/' + path
    std::string filename = meta.metaFile.substr(0, meta.metaFile.find_last_of("/\\"));
    filename += ('/' + meta.chunks[inId.getQuadTreePos()].dataFile);

    //create stream
    IStreamPtr stream = fileSystem->createInputStream(filename);

    unsigned int indicesNum = meta.chunks[inId.getQuadTreePos()].indicesNum;
    //reserve space
    chunk->indices.resize(indicesNum);

    //read data 
    //ugly a bit...
    stream->read(reinterpret_cast<char*>(&chunk->indices[0]), indicesNum * sizeof(TerrainChunk::index_t));

    return chunk;
}

//utility function to load VLOD from stream
static void loadVertexLOD(IStreamPtr inSource, TerrainFragment::VertexCont& outData, const unsigned int vertexNum){
    outData.resize(vertexNum);
                
    //read data
    //TODO: refactor this shit
    TerrainFragment::CompressedVertex compVert;
    for(unsigned int i = 0; i < vertexNum; ++i){

        if(!inSource->good()){
            throw std::runtime_error("Unexpected EOF while reading vertex lod data");
        }
        inSource->read(reinterpret_cast<char*>(&(outData[i].x)), sizeof(compVert.x));
        if(!inSource->good()){
            throw std::runtime_error("Unexpected EOF while reading vertex lod data");
        }
        inSource->read(reinterpret_cast<char*>(&(outData[i].z)), sizeof(compVert.z));
        if(!inSource->good()){
            throw std::runtime_error("Unexpected EOF while reading vertex lod data");
        }

        inSource->read(reinterpret_cast<char*>(&(outData[i].y)), sizeof(compVert.y));
        if(!inSource->good()){
            throw std::runtime_error("Unexpected EOF while reading vertex lod data");
        }
        inSource->read(reinterpret_cast<char*>(&(outData[i].normalX)), sizeof(compVert.normalX));
        if(!inSource->good()){
            throw std::runtime_error("Unexpected EOF while reading vertex lod data");
        }

        inSource->read(reinterpret_cast<char*>(&(outData[i].normalZ)), sizeof(compVert.normalZ));
    }

}

static void unloadVertexLOD(TerrainFragment& fragment, MetaData::Fragment& fragmentMeta, unsigned int vertices, boost::mutex& vlodMutex){
    
    for(unsigned int i = 0; i < (vertices + 1); ++i){
        --fragmentMeta.vlods[i].refCounter;

        if(fragmentMeta.vlods[i].refCounter == 0){
            //unload vlod
            //critical section
            {
                boost::lock_guard<boost::mutex> lock(vlodMutex);

                if(fragment.vertexLODsStatuses[i] == TerrainFragment::RAM){
                    fragment.vertexLODsStatuses[i] = TerrainFragment::UNLOADED;
                    fragment.vertexLODs[i].clear();
                }
            }
        }
    }
}

//function to actually load chunk data (throws out_of_range, runtime_error)
static void doLoadChunk(TerrainChunkId inId, MetaDataPtr meta, ChunkedTerrainPtr inTerrain, FileSystemPtr fileSystem, boost::mutex* inChunkMutex, boost::mutex* inVLODMutex){
    assert(inChunkMutex);
    assert(inVLODMutex);
    try{
        //may throw
        TerrainFragment& fragment = inTerrain->getFragment(inId.getFragmentId());
        //make a copy of pointer to tree
        TerrainFragment::QuadTreeOfChunksPtr tree = fragment.tree; //fragment ref may be not valid here...
        
        if(!tree){
            return;
            //throw std::runtime_error("Can't load chunk, fragment's tree is unloaded.");
        }

        //may throw
        TerrainFragment::ChunkData& chunkData = tree->getNode(inId.getQuadTreePos()).data;
        if(chunkData.status == TerrainFragment::UNLOADED){
            //canceled
            return;
        }

        MetaData::Fragment& fragmentMeta = meta->fragments[inId.getFragmentId()];
        //meta may be empty
        //TODO
        
        //load chunk data
        TerrainChunkPtr chunk = getChunk(inId, fragmentMeta, fileSystem);

        //check needed vertex levels
        for(unsigned int i = 0; i < (chunkData.vertices + 1); ++i){
            //increment reference counter
            ++fragmentMeta.vlods[i].refCounter;

            //check status and load (if neeeded)
            if(fragment.vertexLODsStatuses[i] == TerrainFragment::UNLOADED){
                //load vlod
                //critical section
                {
                    boost::lock_guard<boost::mutex> lock(*inVLODMutex);
                    if(fragment.vertexLODsStatuses[i] == TerrainFragment::UNLOADED)
                        fragment.vertexLODsStatuses[i] = TerrainFragment::LOADING;
                    else continue;
                }

                //get filename
                std::string filename = fragmentMeta.metaFile.substr(0, fragmentMeta.metaFile.find_last_of("/\\"));
                filename += ('/' + fragmentMeta.vlods[i].fileName);

                //create stream
                IStreamPtr stream = fileSystem->createInputStream(filename);

                //load data
                loadVertexLOD(stream, fragment.vertexLODs[i], fragmentMeta.vlods[i].vertexNum);
                
                fragment.vertexLODsStatuses[i] = TerrainFragment::RAM;
            }
        }

        bool shouldUnload = false;
        
        //critical section
        //(we are checking if we need to rollback our transaction)
        {
            boost::lock_guard<boost::mutex> lock(*inChunkMutex);

            //if canceled
            if(chunkData.status == TerrainFragment::UNLOADED){
                shouldUnload = true;
            }
            else{
                //change status and set data
                chunkData.ptr = chunk;
                chunk.reset();
                chunkData.status = TerrainFragment::RAM;
            }
        }
        

        //undo all the things we made before
        if(shouldUnload){
            unloadVertexLOD(fragment, fragmentMeta, chunkData.vertices, *inVLODMutex);
        }
    }
    catch(const std::out_of_range& inOutOfRange){
        std::cerr << "RAMLoadStrategy::doLoadChunk std::out_of_range exception got: " << inOutOfRange.what() << std::endl;
        //ignore
        return;
    }
    catch(const std::runtime_error& inRuntimeError){
        std::cerr << "RAMLoadStrategy::doLoadChunk std::runtime_error exception got: " << inRuntimeError.what() << std::endl;
        //ignore
        return;
    }
    catch(const std::exception& inException){
        std::cerr << "RAMLoadStrategy::doLoadChunk std::exception got: " << inException.what() << std::endl;
        return;
    }
    catch(...){
        std::cerr << "RAMLoadStrategy::doLoadChunk Unknown exception got. Ignoring." << std::endl;
        return;
    }
}

static void traverseFragmentMeta(QTreePtr tree, pugi::xml_node& xmlNode, std::vector<MetaData::Chunk>& meta){
    //first we load metadata of current node
    TerrainFragment::ChunkData chunkData;
    chunkData.vertices = xmlNode.attribute("vertex_level").as_uint();
    chunkData.maxError = xmlNode.attribute("max_error").as_float();
    chunkData.level = xmlNode.attribute("index_level").as_uint();
    unsigned int indicesNum = xmlNode.attribute("indices_num").as_uint();
    unsigned int position = xmlNode.attribute("position").as_uint();

    //get data filename
    meta[position].dataFile = xmlNode.child("data").attribute("filename").value();
    meta[position].indicesNum = indicesNum;

    //get AABB
    pugi::xml_node aabbNode = xmlNode.child("aabb");
    hydra::math::Vector3D corner(aabbNode.child("corner").attribute("x").as_float(), 
                                 aabbNode.child("corner").attribute("y").as_float(),
                                 aabbNode.child("corner").attribute("z").as_float());
    hydra::math::Vector3D vector(aabbNode.child("vector").attribute("x").as_float(), 
                                 aabbNode.child("vector").attribute("y").as_float(),
                                 aabbNode.child("vector").attribute("z").as_float());
    hydra::math::AABB aabb(corner, vector);
    chunkData.aabb = aabb;
    chunkData.status = TerrainFragment::UNLOADED;

    tree->getNode(position).data = chunkData;

    //handle children
    pugi::xml_node_iterator iter = xmlNode.begin();
    ++iter; //aabb
    while(++iter != xmlNode.end()){
        //check name (quad)
        if(std::string(iter->name()) != "quad"){
            throw std::runtime_error("Wrong xml format. 'quad' node expected.");
        }

        traverseFragmentMeta(tree, *iter, meta);
    }
}

static QTreePtr getFragmentTree(TerrainFragmentId inId, MetaDataPtr meta, FileSystemPtr fileSystem){
    //read metadata using pugixml and metadata from main.meta
    std::string metaFilePath = meta->fragments[inId].metaFile;
    //get fragment's directory
    std::string fragmentDir = metaFilePath.substr(0, metaFilePath.find_last_of("/\\"));
    
    //parse metafile
    pugi::xml_document document;
    pugi::xml_parse_result parseResult = document.load(*fileSystem->createInputStream(metaFilePath));
    if(!parseResult) throw std::runtime_error("Error while parsing xml file.");

    pugi::xml_node root = document.root().child("terrain_fragment");
    if(!root){
        throw std::runtime_error("Wrong xml format. Root node must have 'terrain_fragment' name.");
    }

    pugi::xml_node_iterator iter = root.begin();
    
    //read description
    pugi::xml_node descr = *iter;
    if(std::string(descr.name()) != "description"){
        throw std::runtime_error("Wrong xml format. First node must be a 'description'.");
    }
    unsigned int ilodNum = descr.child("index_lods").attribute("number").as_uint();
    unsigned int vlodNum = descr.child("vertex_lods").attribute("number").as_uint();

    //calculate number of chunks
    unsigned int numberOfChunks = 0;
    {
        unsigned int nextNum = 1;
        for(size_t i = 0; i < ilodNum; ++i){
            numberOfChunks += nextNum;
            nextNum <<= 2; // 1 4 16 64 ...
        }
    }
    meta->fragments[inId].chunks.resize(numberOfChunks);

    MetaData::VLOD emptyVLOD;
    emptyVLOD.vertexNum = 0;
    emptyVLOD.refCounter = 0;
    emptyVLOD.fileName = "";
    meta->fragments[inId].vlods.resize(vlodNum, emptyVLOD);

    //check position
    assert(descr.child("position").attribute("x").as_int() == inId.getX());
    assert(descr.child("position").attribute("y").as_int() == inId.getY());

    QTreePtr tree(new QTree(ilodNum));

    //read tree

    ++iter;

    //must be quad node
    if(std::string(iter->name()) != "quad") 
        throw std::runtime_error("Wrong xml format. 'quad' node expected.");

    traverseFragmentMeta(tree, *iter, meta->fragments[inId].chunks);

    //load metadata for vlods
    while(++iter != root.end()){
        //check name
        if(std::string(iter->name()) != "vertex_lod"){
            std::runtime_error("Wrong xml format. 'vertex_lod' expected.");
        }

        unsigned int vlevel = iter->attribute("level").as_uint();
        meta->fragments[inId].vlods[vlevel].fileName = iter->child("data").attribute("filename").value();
        meta->fragments[inId].vlods[vlevel].vertexNum = iter->attribute("vertex_number").as_uint();
    }

    return tree;
}

//function to actually load fragment data
static void doLoadFragment(TerrainFragmentId inId, MetaDataPtr meta, ChunkedTerrainPtr inTerrain, FileSystemPtr fileSystem, boost::mutex* inFragmentMutex){
    assert(inFragmentMutex);
    try{
        TerrainFragment& fragment = inTerrain->getFragment(inId);

        //loading may be canceled
        if(fragment.status == TerrainFragment::LOADING){
            QTreePtr tree = getFragmentTree(inId, meta, fileSystem);
            fragment.vertexLODs.resize(meta->fragments[inId].vlods.size());
            fragment.vertexLODsStatuses.resize(meta->fragments[inId].vlods.size(), TerrainFragment::UNLOADED);
            bool loaded = false;

            //critical section
            {
                boost::lock_guard<boost::mutex> lock(*inFragmentMutex);
                if(fragment.status == TerrainFragment::LOADING){
                    fragment.tree = tree;
                    fragment.status = TerrainFragment::RAM;
                    loaded = true;
                }
                else{
                    loaded = false;
                }
            }

            if(!loaded){
                fragment.vertexLODs.clear();
                fragment.vertexLODsStatuses.clear();
            }
        }
    }
    catch(const std::out_of_range& inOutOfRange){
        std::cerr << "Warning: fragment not found: " << inId.getX() << 'X' << inId.getY() << std::endl;
        //fragment not found
        //ignore
        return;
    }
    catch(const std::runtime_error& inRuntimeError){
        std::cerr << "RAMLoadStrategy::doLoadFragment std::runtime_error exception got: " << inRuntimeError.what() << std::endl;
        //ignore
        return;
    }
    catch(const std::exception& inException){
        std::cerr << "RAMLoadStrategy::doLoadFragment std::exception got: " << inException.what() << std::endl;
        return;
    }
    catch(...){
        std::cerr << "RAMLoadStrategy::doLoadFragment Unknown exception got. Ignoring." << std::endl;
        return;
    }
}

MyTerrainRAMLoadStrategy::MyTerrainRAMLoadStrategy(hydra::data::ChunkedTerrainPtr inTerrain, boost::asio::io_service& inIOService, const std::string& inPath): mImpl(new MyTerrainRAMLoadStrategy::Impl(inTerrain, inIOService, inPath)){

}

MyTerrainRAMLoadStrategy::~MyTerrainRAMLoadStrategy(){

}


//===========================================================
//===============START CHUNK LOAD============================
//===========================================================

void MyTerrainRAMLoadStrategy::startAsyncLoad(hydra::data::TerrainChunkId inId){
    assert(mImpl);
    if(!mImpl->terrain) throw std::runtime_error("Can't load anything. You must bind ChunkedTerrain object first.");

    //TODO: check all parent chunks (they should not be unloaded)

    //find chunk's position
    TerrainFragment& fragment = mImpl->terrain->getFragment(inId.getFragmentId());
    if(fragment.status != TerrainFragment::RAM || !fragment.tree) throw std::runtime_error("Can't load chunk: fragment is unloaded.");
    TerrainFragment::QuadTreeOfChunks::Node& node = fragment.tree->getNode(inId.getQuadTreePos());
    if(node.data.status != TerrainFragment::UNLOADED) return;

    node.data.status = TerrainFragment::LOADING; //no desync here

    mImpl->io_service.post(boost::bind(doLoadChunk, inId, mImpl->metaData, mImpl->terrain, mImpl->fileSystem, &mImpl->chunkMutex, &mImpl->vlodMutex));
}



//===========================================================
//===============START FRAGMENT LOAD=========================
//===========================================================

void MyTerrainRAMLoadStrategy::startAsyncLoad(hydra::data::TerrainFragmentId inId){
    assert(mImpl);

    if(!mImpl->terrain) throw std::runtime_error("Can't load anything. You must bind ChunkedTerrain object first.");

    //first check if we can load
    ChunkedTerrain::iterator iter(inId, *mImpl->terrain);
    if(iter == mImpl->terrain->end()){
        //ignore
        return;
    }

    if((*iter).second.status != TerrainFragment::UNLOADED) return;

    (*iter).second.status = TerrainFragment::LOADING;
    
    mImpl->io_service.post(boost::bind(doLoadFragment, inId, mImpl->metaData, mImpl->terrain, mImpl->fileSystem, &mImpl->fragmentMutex));
}



//we unload chunk and all its child chunks recursively
void MyTerrainRAMLoadStrategy::unload(hydra::data::TerrainChunkId inId){
    assert(mImpl);

    if(!mImpl->terrain) throw std::runtime_error("Can't unload anything. You must bind ChunkedTerrain object first.");

    ChunkedTerrain::iterator iter(inId.getFragmentId(), *mImpl->terrain);
    if(iter == mImpl->terrain->end()){
        //ignore
        return;
    }

    TerrainFragment& fragment = (*iter).second;

    if(!fragment.tree || fragment.status == TerrainFragment::UNLOADED){
        return;
    }

    //may throw
    TerrainFragment::QuadTreeOfChunks::Node& node = fragment.tree->getNode(inId.getQuadTreePos());
    
    if(node.data.status == TerrainFragment::UNLOADED) return;
    
    bool shouldUnload = false;

    //critical section
    {
        //what if the data is in VRAM??
        boost::lock_guard<boost::mutex> lock(mImpl->chunkMutex);
        if(node.data.status == TerrainFragment::UNLOADED) return; //strange
        if(node.data.status == TerrainFragment::LOADING){
            shouldUnload = false;
            node.data.status = TerrainFragment::UNLOADED;
        }
        else{
            shouldUnload = true;
            node.data.status = TerrainFragment::UNLOADED;
            node.data.ptr.reset(); //TODO: desync
        }
    }
    //unload childs
    for(int i = 0; i < 4; ++i){
        if(node.nodes[i] != 0){
            //recursive call
            unload(TerrainChunkId(inId.getFragmentId(), node.nodes[i]));
        }
    }
    
    //unload vertex lods (if needed)
    if(shouldUnload){
        MetaData::Fragment& fragmentMeta = mImpl->metaData->fragments[inId.getFragmentId()];
        unloadVertexLOD(fragment, fragmentMeta, node.data.vertices, mImpl->vlodMutex);
    }
}



void MyTerrainRAMLoadStrategy::unload(hydra::data::TerrainFragmentId inId){
    assert(mImpl);

    if(!mImpl->terrain) throw std::runtime_error("Can't unload anything. You must bind ChunkedTerrain object first.");

    //check if we have such fragment
    ChunkedTerrain::iterator iter(inId, *mImpl->terrain);
    //no such fragment
    if(iter == mImpl->terrain->end()){
        //just ignore
        return;
    }

    TerrainFragment& fragment = (*iter).second;
    
    //check for childs
    if(fragment.tree){
        if(fragment.tree->getRoot().data.status != TerrainFragment::UNLOADED){
            //unload all chunks by deleting the root
            unload(TerrainChunkId(inId, 0));
        }
        //vlods will be unloaded automatically
    }
    
    fragment.status = TerrainFragment::UNLOADED;

    //unload metadata
    mImpl->metaData->fragments[inId].vlods.clear();
    mImpl->metaData->fragments[inId].chunks.clear();
    
    //unload
    fragment.tree.reset();
}

unsigned int MyTerrainRAMLoadStrategy::getTerrainWidth() const{
    assert(mImpl);

    return mImpl->metaData->terrainWidth;
}

unsigned int MyTerrainRAMLoadStrategy::getTerrainHeight() const{
    assert(mImpl);

    return mImpl->metaData->terrainHeight;
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
