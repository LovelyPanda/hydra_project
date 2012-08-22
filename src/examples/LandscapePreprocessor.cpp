//LandscapePreprocessor.cpp

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

/**
 * This is a tool for preprocessing landscape given in a heightmap.
 * Tool decodes image (heightmap) and splits it into fragments. For each fragment
 * it generates tree of levels of details (using hydra::rendering::TerrainPreprocessor).
 * Then it saves binary data and special metainformation (see "terrain metadata spec"
 * in docs section). Those data is enough to render landscape (original heightmap
 * is not needed).
 * This tool should be used offline as it may need big amounts of memory and
 * CPU time.
 *
 * \author A.V.Medvedev
 * \date 14.10.2010
 */

//Hydra headers
#include "rendering/TerrainPreprocessor.hpp"
#include "data/TerrainFragmentId.hpp"
#include "data/TerrainFragment.hpp"
#include "data/TerrainChunk.hpp"
#include "data/HeightMap.hpp"
#include "data/Image.hpp"
#include "loading/LoadingMain.hpp"

//Boost headers
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

//3rd party headers
#include <zip.h>
#include "pugixml.hpp"

//C++ headers
#include <map>
#include <string>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <fstream>

//C headers
#include <cassert>
#include <cstring>
#include <cstdlib>

using namespace hydra::rendering;
using namespace hydra::data;
using namespace hydra::loading;

namespace options = boost::program_options;
namespace filesystem = boost::filesystem;

//========================================================================
//=================OUTPUT STUFF===========================================
//========================================================================

typedef hydra::common::SharedPtr<std::ostream>::Type OStreamPtr;


//abstract stream creator
class FileSystem{
public:
    virtual ~FileSystem(){

    }

    virtual void createDir(const std::string& inPath) = 0;
    virtual OStreamPtr createOutputStream(const std::string& inPath) = 0;
};

typedef hydra::common::SharedPtr<FileSystem>::Type FileSystemPtr;

//file implemetation
//simply works with files in directory
class FileFileSystem: public FileSystem, private boost::noncopyable{
public:
    FileFileSystem(const std::string& inPath): mPath(inPath){
        createDir("");
    }

    virtual ~FileFileSystem(){

    }
    
    virtual void createDir(const std::string& inPath){
        if(!filesystem::is_directory(mPath + '/' + inPath))
            filesystem::create_directory(mPath + '/' + inPath);
    }

    virtual OStreamPtr createOutputStream(const std::string& inPath){
        return OStreamPtr(new std::ofstream((mPath + '/' + inPath).c_str(), std::ofstream::out | std::ofstream::binary));
    }

private:
    std::string mPath;
};


//zip implementation
//We save all the data to temporary directory.
//When work ends we create zip archive and copy all data there
//and delete directory. This is done to reduce memory consumption
//due to the libzip limitations.
class ZipFileSystem : public FileSystem, boost::noncopyable{
public:
    ZipFileSystem(const std::string& inPathToArchive): mPathToArchive(inPathToArchive){
        //we should create temporary directory
        while(true){
            std::ostringstream stream;
            stream << inPathToArchive << "_temp__" << std::hex << static_cast<unsigned int>(rand()) << static_cast<unsigned int>(rand());
            if(!filesystem::is_directory(stream.str())){
                //name is ok
                filesystem::create_directory(stream.str());
                mPathToTempDir = stream.str();
                mTempDir = FileSystemPtr(new FileFileSystem(mPathToTempDir));
                break;
            }
        }
    }

    virtual ~ZipFileSystem(){
        close();
    }
    
    virtual void createDir(const std::string& inPath){
        assert(mTempDir);
        mTempDir->createDir(inPath);
    }

    virtual OStreamPtr createOutputStream(const std::string& inPath){
        assert(mTempDir);
        return mTempDir->createOutputStream(inPath);
    }

    //we should save all files to zip-archive
    //and then delete temporary dir along with files
    void close(){
        mTempDir.reset();
        
        try{
            zip* archive;
            int errorCode = 0;
            archive = zip_open(mPathToArchive.c_str(), ZIP_CREATE, &errorCode);
            if(archive == 0 || errorCode != 0){
                throw std::runtime_error("Error while opening zip-archive.");
            }

            //recursively save all content
            saveDirectoryContent(archive, mPathToTempDir);

            //write everything
            if(zip_close(archive) != 0){
                std::cerr << "Error while closing zip archive: " << zip_strerror(archive) << std::endl;
            }
        }
        catch(const std::runtime_error ex){
            std::cerr << "Error while saving zip-archive: " << ex.what() << std::endl;
        }

        filesystem::remove_all(mPathToTempDir);
    }

private:


    void saveDirectoryContent(zip* inArchive, const std::string& inDirPath){
        assert(inArchive);

        if(!filesystem::exists(inDirPath) || !filesystem::is_directory(inDirPath)){
            throw std::runtime_error("Can't find directory!");
        }

        std::string pathInArchive = "";
        //add current dir
        if(inDirPath != mPathToTempDir){
            assert(0 == inDirPath.find(mPathToTempDir));
            pathInArchive = inDirPath.substr(inDirPath.find_first_of("/\\", mPathToTempDir.size()) + 1);
            zip_add_dir(inArchive, pathInArchive.c_str());
        }

        filesystem::directory_iterator end_iter;
        for(filesystem::directory_iterator iter(inDirPath); iter != end_iter; ++iter){
            if(filesystem::is_directory(iter->status())){
                saveDirectoryContent(inArchive, iter->path().string());
            }
            else{
                //save file's content
                std::string filePathInArchive;
                if(pathInArchive.empty())  filePathInArchive = iter->path().filename().string();
                else filePathInArchive = pathInArchive + '/' + iter->path().filename().string();

                zip_source *fileToSave;
                fileToSave = zip_source_file(inArchive, iter->path().string().c_str(), 0, -1);
                if(fileToSave == 0){
                    //error
                    std::cerr << "Error: Could not create zip_source! File is not saved." << std::endl;
                    continue;
                }
                //try to add new file
                if(zip_add(inArchive, filePathInArchive.c_str(), fileToSave) < 0){
                    //error while add
                    std::cerr << "warning, file is present, rewriting!" << std::endl;
                    int errorCode = 0;
                    zip_error_get(inArchive, &errorCode, 0);
                    if(errorCode == ZIP_ER_EXISTS){
                        //we should replace file
                        int fileIndex = zip_name_locate(inArchive, filePathInArchive.c_str(), 0);
                        assert(fileIndex != -1);
                        if(zip_replace(inArchive, fileIndex, fileToSave) < 0){
                            zip_source_free(fileToSave);
                            std::cerr << "Error: Could not replace file." << zip_strerror(inArchive) << std::endl;
                            return;
                        }
                    }
                    else{
                        //fatal error
                        zip_source_free(fileToSave);
                        std::cerr << "Error while saving file to zip archive: " << zip_strerror(inArchive) << std::endl;
                        return;
                    }
                    
                }
            }

        }

    }


    std::string mPathToArchive;
    std::string mPathToTempDir;
    FileSystemPtr mTempDir;
};

//========================================================================
//=================END OF OUTPUT STUFF====================================
//========================================================================


//simple struct to store program configuration (obtained from console arguments)
struct Args{
    float lodError;
    float lodErrorFactor;
    float vertexLODFactor;
    float maxHeight;
    unsigned int lodNum;
    unsigned int threadNum;
    unsigned int fragmentSize;
    std::string inputHeightmap;
    std::string outDir;
    std::string outZip;
};


//function to handle command-line arguments.
//May throw std::runtime_error if some arguments are not present or wrong.
Args handleArguments(int inArgNum, char** inArgs){
    Args result;

    options::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("output-dir,d", options::value<std::string>()->default_value("."), "where to place output data (if 'z' option is not used)")
        ("output-zip,z", options::value<std::string>(), "what archive to use for output")
        ("lod-error,e", options::value<float>(), "initial LOD error (used for 1 level)")
        ("lod-error-factor,f", options::value<float>()->default_value(0.25f), "LOD error factor (used to compute each next level's of details error value)")
        ("lod-num,l", options::value<int>()->default_value(4), "number of levels per fragment")
        ("vertex-lod-factor,v", options::value<float>()->default_value(0.5f), "vertex LOD factor lies in interval (0.0, 1.0)")
        ("threads,t", options::value<int>()->default_value(4), "number of threads used")
        ("fragment-size,s", options::value<int>()->default_value(257), "size of fragment's 1 level [size == 2^n + 1 and size <= 257]")
        ("input-heightmap,i", options::value<std::string>(), "input heightmap (image)")
        ("max-height,m", options::value<float>()->default_value(100.0f), "Maximum height. Vertices' heights will be dequantized to interval [0.0, max-height].")
        ;//more options here...

    options::variables_map vm;
    options::store(options::parse_command_line(inArgNum, inArgs, desc), vm);
    options::notify(vm);

    //some checks
    if(vm.count("help")){
        std::cout << desc << "\n";
        throw std::runtime_error("help has been requested");
    }
    if(!vm.count("lod-error")){
        std::cerr << "You must specify initial LOD error value!\n";
        std::cout << desc << "\n";
        throw std::runtime_error("no lod-error option specified");
    }
    if(!vm.count("input-heightmap")){
        std::cerr << "You must specify input heightmap file name!\n";
        std::cout << desc << "\n";
        throw std::runtime_error("no input heightmap");
    }

    //copy data
    result.lodError = vm["lod-error"].as<float>();
    result.lodErrorFactor = vm["lod-error-factor"].as<float>();
    result.vertexLODFactor = vm["vertex-lod-factor"].as<float>();
    result.lodNum = vm["lod-num"].as<int>();
    result.threadNum = vm["threads"].as<int>();
    result.fragmentSize = vm["fragment-size"].as<int>();
    result.inputHeightmap = vm["input-heightmap"].as<std::string>();
    result.maxHeight = vm["max-height"].as<float>();

    if(vm.count("output-zip")){
        result.outZip = vm["output-zip"].as<std::string>();
        result.outDir = "";
    }
    else{
        result.outZip = "";
        result.outDir = vm["output-dir"].as<std::string>();
    }

    //some more checks
    if(result.lodError < 0.0){
        std::cerr << "lod-error has wrong value. It must be in interval [0, +infinity)." << std::endl;
        throw std::runtime_error("lod-error has wrong value");
    }
    if(result.lodNum <= 0 || result.lodNum > 8){
        std::cerr << "lod-num has wrong value. It must be in interval (0, 8]." << std::endl;
        throw std::runtime_error("lod-num has wrong value");
    }
    if(result.threadNum <= 0){
        std::cerr << "Number of threads has wrong value. It must lie in interval (0, +infinity)" << std::endl;
        throw std::runtime_error("threads has wrong value");
    }
    if(result.fragmentSize != 257 && result.fragmentSize != 129 && result.fragmentSize != 65){
        std::cerr << "Fragment size has wrong value. Currently supporte only {65, 129, 257}." << std::endl;
        throw std::runtime_error("fragment-size has wrong value");
    }
    if(result.lodErrorFactor <= 0 || result.lodErrorFactor >= 1.0){
        std::cerr << "LOD error factor has wrong value. It must be in interval (0.0, 1.0)" << std::endl;
        throw std::runtime_error("lod-error-factor has wrong value");
    }
    if(result.vertexLODFactor <= 0 || result.vertexLODFactor >= 1.0){
        std::cerr << "Vertex LOD error factor has wrong value. It must be in interval (0.0, 1.0)" << std::endl;
        throw std::runtime_error("vertex-lod-factor has wrong value");
    }


    return result;
}

//splits original image into square fragments of specified size
//creates HeightMap objects for every one and returns map of those objects
std::map<TerrainFragmentId, HeightMapPtr> splitIntoFragments(const ImagePtr inImage, int inFragmentSize, float inMaxHeight){
    std::map<TerrainFragmentId, HeightMapPtr> result;
    if((inImage->getWidth() - 1)  % (inFragmentSize - 1) != 0 ||
       (inImage->getHeight() - 1) % (inFragmentSize - 1) != 0 ){
        throw std::runtime_error("Specified image can't be splitted into equal fragments of specified size without loss. Take another image or change fragment's size.");
    }

    const unsigned int IMAGE_WIDTH = inImage->getWidth();
    const unsigned int IMAGE_HEIGHT = inImage->getHeight();
    const unsigned int VERT_NUMBER_OF_FRAGMENTS = (IMAGE_HEIGHT - 1) / (inFragmentSize - 1);
    const unsigned int HORIZ_NUMBER_OF_FRAGMENTS = (IMAGE_WIDTH - 1)/ (inFragmentSize - 1);
    std::cout << "splitted to " << VERT_NUMBER_OF_FRAGMENTS << 'x' << HORIZ_NUMBER_OF_FRAGMENTS << std::endl;
    const unsigned char* IMAGE_DATA = inImage->getData();

    const unsigned int STRIDE = (inImage->getMode() == Image::RGB) ? 3 : 4;
    
    for(unsigned int y = 0; y < VERT_NUMBER_OF_FRAGMENTS; ++y){ //Y
        for(unsigned int x = 0; x < HORIZ_NUMBER_OF_FRAGMENTS; ++x){ //X
            //for each fragment
            unsigned char* fragment_data = new unsigned char[inFragmentSize * inFragmentSize];
            //calculate start of the fragment in image data
            const unsigned char* FRAGMENT_START_IN_IMAGE_DATA = 
                IMAGE_DATA + STRIDE * y * IMAGE_WIDTH * (inFragmentSize - 1) + STRIDE * x * (inFragmentSize - 1);

            //copy fragment's data to its heightmap

            //for each fragment's line
            for(int k = 0; k < inFragmentSize; ++k){
                //for each byte
                for(int l = 0; l < inFragmentSize; ++l){
                    assert((FRAGMENT_START_IN_IMAGE_DATA + STRIDE * k * IMAGE_WIDTH + STRIDE * l) < (IMAGE_DATA + IMAGE_WIDTH * IMAGE_HEIGHT * STRIDE));
                    fragment_data[k * inFragmentSize + l] = FRAGMENT_START_IN_IMAGE_DATA[STRIDE * IMAGE_WIDTH * k + STRIDE * l];
                }
            }

            HeightMapGen<unsigned char>* heightmap= new HeightMapGen<unsigned char>(inFragmentSize, fragment_data);
            heightmap->setMinHeight(0.0f);
            heightmap->setMaxHeight(inMaxHeight);
            HeightMapPtr nextHeightMapFragment(heightmap);
            TerrainFragmentId nextId(x, y);
            result[nextId] = nextHeightMapFragment;
        }
    }

    return result;
}

typedef TerrainFragment::QuadTreeOfChunks QTree;

const char* nodePosToString(QTree::NodePos nodePos){
    switch(nodePos){
        case QTree::SOUTH_WEST_QUAD:
            return "SW";
        case QTree::NORTH_WEST_QUAD:
            return "NW";
        case QTree::SOUTH_EAST_QUAD:
            return "SE";
        case QTree::NORTH_EAST_QUAD:
            return "NE";
        default:
            assert(false);
            return "ERROR";
    }
    assert(false);
    return "ERROR";
}

void recursivelySaveChunks(pugi::xml_node& parentXMLNode, const QTree& tree, unsigned int position, const std::string& path, const std::string& filenamePostfix, FileSystemPtr inFileSystem){
    const QTree::Node& node = tree.getNode(position);

    pugi::xml_node quadNode = parentXMLNode.append_child();
    quadNode.set_name("quad");

    quadNode.append_attribute("index_level") = static_cast<int>(node.data.level);
    quadNode.append_attribute("vertex_level") = static_cast<int>(node.data.vertices);
    quadNode.append_attribute("max_error") = node.data.maxError;
    quadNode.append_attribute("indices_num") = static_cast<unsigned int>(node.data.ptr->indices.size());
    quadNode.append_attribute("position") = position;

    //add data node
    {
        pugi::xml_node data = quadNode.append_child();
        data.set_name("data");

        std::ostringstream sstream;
        sstream << "ilods/level_" << static_cast<int>(node.data.level) << '_' << filenamePostfix << ".bin";
        std::string filename = sstream.str();
        data.append_attribute("filename") = filename.c_str();

        if(!node.data.ptr->indices.empty()){
            filename = path + '/' + filename;
            OStreamPtr stream = inFileSystem->createOutputStream(filename);

            size_t indexSize = sizeof(TerrainChunk::IndexCont::value_type);
            stream->write(reinterpret_cast<const char*>(&node.data.ptr->indices[0]), node.data.ptr->indices.size() * indexSize);
        }
    }

    //add aabb node
    {
        pugi::xml_node aabb = quadNode.append_child();
        aabb.set_name("aabb");

        pugi::xml_node corner = aabb.append_child();
        corner.set_name("corner");

        corner.append_attribute("x") = node.data.aabb.getCorner().x();
        corner.append_attribute("y") = node.data.aabb.getCorner().y();
        corner.append_attribute("z") = node.data.aabb.getCorner().z();

        pugi::xml_node vector = aabb.append_child();
        vector.set_name("vector");

        vector.append_attribute("x") = node.data.aabb.getVector().x();
        vector.append_attribute("y") = node.data.aabb.getVector().y();
        vector.append_attribute("z") = node.data.aabb.getVector().z();
    }

    for(int i = 0; i < 4; ++i){
        if(node.nodes[i] != 0){
            recursivelySaveChunks(quadNode, tree, node.nodes[i], path, filenamePostfix + nodePosToString(static_cast<QTree::NodePos>(i)), inFileSystem);
        }
    }
}

std::string generateMetaFilename(TerrainFragmentId inId){
    std::ostringstream sstream;
    sstream << inId.getX() << '_' << inId.getY();
    return sstream.str();
}

//saves all fragment's data
//returns file's name
std::string saveFragment(TerrainFragmentId inId, TerrainFragmentPtr inFragment, FileSystemPtr inFileSystem){
    std::cout << "Fragment " << inId.getX() << 'x' << inId.getY() << '\n' <<
                 "number of lods: " << inFragment->tree->getResolution() << '\n' <<
                 "number of vertex lods: " << inFragment->vertexLODs.size() << std::endl << std::endl;

    //first we must generate DOM
    //we use pugixml

    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child();
    root.set_name("terrain_fragment");

    //create 'description' block
    {
        pugi::xml_node descr = root.append_child();
        descr.set_name("description");
        
        pugi::xml_node index_lods = descr.append_child();
        index_lods.set_name("index_lods");
        index_lods.append_attribute("number") = inFragment->tree->getResolution();

        pugi::xml_node vertex_lods = descr.append_child();
        vertex_lods.set_name("vertex_lods");
        vertex_lods.append_attribute("number") = static_cast<unsigned int>(inFragment->vertexLODs.size());

        pugi::xml_node position = descr.append_child();
        position.set_name("position");
        position.append_attribute("x") = inId.getX();
        position.append_attribute("y") = inId.getY();
    }

    std::string newPath = generateMetaFilename(inId);
    std::string filename = newPath + '/' + generateMetaFilename(inId) + ".tfmeta";
    
    inFileSystem->createDir(newPath);
    inFileSystem->createDir(newPath + "/vlods");
    inFileSystem->createDir(newPath + "/ilods");

    //recursively save quad tree of chunks
    recursivelySaveChunks(root, *inFragment->tree, 0, newPath, "", inFileSystem);

    //save vertex_lods
    for(size_t i = 0; i < inFragment->vertexLODs.size(); ++i){
        pugi::xml_node newVertexLod = root.append_child();
        newVertexLod.set_name("vertex_lod");

        newVertexLod.append_attribute("level") = static_cast<unsigned int>(i);
        newVertexLod.append_attribute("vertex_number") = static_cast<unsigned int>(inFragment->vertexLODs[i].size());

        pugi::xml_node dataNode = newVertexLod.append_child();
        dataNode.set_name("data");
        std::ostringstream sstream;
        sstream << "vlods/v_level" << i << ".bin";
        
        std::string vertexFilename = sstream.str();
        dataNode.append_attribute("filename") = vertexFilename.c_str();
        vertexFilename = newPath + '/' + vertexFilename;

        OStreamPtr ostream = inFileSystem->createOutputStream(vertexFilename);

        TerrainFragment::CompressedVertex sampleVertex;
        for(size_t j = 0; j < inFragment->vertexLODs[i].size(); ++j){
            //TODO: this code is dirty!
            ostream->write(reinterpret_cast<const char*>(&inFragment->vertexLODs[i][j].x), sizeof(sampleVertex.x));
            ostream->write(reinterpret_cast<const char*>(&inFragment->vertexLODs[i][j].z), sizeof(sampleVertex.z));
            ostream->write(reinterpret_cast<const char*>(&inFragment->vertexLODs[i][j].y), sizeof(sampleVertex.y));
            ostream->write(reinterpret_cast<const char*>(&inFragment->vertexLODs[i][j].normalX), sizeof(sampleVertex.normalX));
            ostream->write(reinterpret_cast<const char*>(&inFragment->vertexLODs[i][j].normalZ), sizeof(sampleVertex.normalZ));
        }
    }


    OStreamPtr ostream = inFileSystem->createOutputStream(filename);
    doc.save(*ostream);

    return filename;
}

void saveMainMeta(int inWidth, int inHeight, unsigned int inFragmentWidth, const std::vector<std::pair<TerrainFragmentId, std::string> >& inFileList, const std::string& landscapeName, FileSystemPtr inFileSystem){
    
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child();
    root.set_name("landscape");

    //add description
    {
        pugi::xml_node descr = root.append_child();
        descr.set_name("description");

        pugi::xml_node size = descr.append_child();
        size.set_name("size");
        size.append_attribute("x") = inWidth;
        size.append_attribute("y") = inHeight;

        descr.append_attribute("fragment_width") = inFragmentWidth;
        descr.append_attribute("fragment_num") = static_cast<unsigned int>(inFileList.size());
        descr.append_attribute("name") = landscapeName.c_str();
    }

    for(size_t i = 0; i < inFileList.size(); ++i){
        pugi::xml_node newFragmentNode = root.append_child();
        newFragmentNode.set_name("terrain_fragment");
        newFragmentNode.append_attribute("x") = inFileList[i].first.getX();
        newFragmentNode.append_attribute("y") = inFileList[i].first.getY();

        pugi::xml_node data = newFragmentNode.append_child();
        data.set_name("meta");

        data.append_attribute("filename") = inFileList[i].second.c_str();
    }

    OStreamPtr ostream = inFileSystem->createOutputStream("main.meta");
    doc.save(*ostream);
}


//functor for threads which are handling terrain fragment data
class ThreadStart{

public:
    ThreadStart(std::map<TerrainFragmentId, 
                HeightMapPtr>& inMap, 
                TerrainPreprocessor::Properties inProps, 
                FileSystemPtr inFileSystem): mMap(inMap), 
                                             mPreprocessor(inProps), 
                                             mFileSystem(inFileSystem){
    
    }

    ThreadStart(const ThreadStart& inStart): mMap(inStart.mMap), 
                                mPreprocessor(inStart.mPreprocessor.getProperties()), 
                                mFileSystem(inStart.mFileSystem){

    }

    void operator()(){
        try{
            std::pair<TerrainFragmentId, HeightMapPtr> nextFragmentData;
            nextFragmentData = getNextFragmentData();
            
            //while there is data to handle
            while(nextFragmentData.second){
                TerrainFragmentPtr fragment = mPreprocessor.process(*(nextFragmentData.second));

                //now we must save data
                std::string filename = saveFragment(nextFragmentData.first, fragment, mFileSystem);
                addToFragmentList(nextFragmentData.first, filename);

                nextFragmentData = getNextFragmentData();
            }
        }
        catch(const std::runtime_error& e){
            std::cerr << "WARNING: exception got in thread #" << boost::this_thread::get_id() <<
                " : " << e.what() << std::endl;
        }
        catch(const std::exception& e){
            std::cerr << "WARNING: exception got in thread #" << boost::this_thread::get_id() <<
                " : " << e.what() << std::endl;

            //std::cerr << "WARNING: unknown exception has been thrown in thread #" << boost::this_thread::get_id() << std::endl;
        }
    }

    //thread-safe function to get next fragment data from container and delete it
    std::pair<TerrainFragmentId, HeightMapPtr> getNextFragmentData(){
        boost::lock_guard<boost::mutex> lock(sMapMutex);

        //take beginning
        std::map<TerrainFragmentId, HeightMapPtr>::iterator begin = mMap.begin();
        //if nothing to work with, return empty pair
        if(begin == mMap.end()) return std::pair<TerrainFragmentId, HeightMapPtr>();

        //copy pair
        std::pair<TerrainFragmentId, HeightMapPtr> result = *begin;
        //erase from map
        mMap.erase(begin);

        //return result (Truly yours, Cpt.)
        return result;

        //unlock mutex
    }

public:
    static std::vector<std::pair<TerrainFragmentId, std::string> > sFileList;

private:
    ThreadStart& operator=(const ThreadStart& inStart); //NO IMPLEMENTATION

    //adds new file to fragmentlist (thread-safe)
    static void addToFragmentList(TerrainFragmentId inId, const std::string& inFilename){
        boost::lock_guard<boost::mutex> lock(sFileListMutex);

        sFileList.push_back(std::make_pair(inId, inFilename));
    }

    std::map<TerrainFragmentId, HeightMapPtr>& mMap;
    TerrainPreprocessor mPreprocessor;
    FileSystemPtr mFileSystem;

    static boost::mutex sMapMutex;
    static boost::mutex sFileListMutex;
};

boost::mutex ThreadStart::sMapMutex;
boost::mutex ThreadStart::sFileListMutex;
std::vector<std::pair<TerrainFragmentId, std::string> > ThreadStart::sFileList;


int main(int ac, char** av){
    
    //structure to contain arguments
    Args args;

    //parse console arguments
    try{
        args = handleArguments(ac, av);
    }
    catch(const std::runtime_error& e){
        //error message has already been printed
        return 1;
    }

    ImagePtr image;
    //init hydra factories
    initFactories();

    //if name of heightmap contains '?' we are loading
    //from zip archive
    bool fromZip = (args.inputHeightmap.find('?') != std::string::npos);
    std::string landscapeName;

    //load input heightmap
    try{
        if(fromZip){
            size_t questionSignPos = args.inputHeightmap.find('?');
            std::string pathToZip = args.inputHeightmap.substr(0, questionSignPos);
            std::string pathInZip = args.inputHeightmap.substr(questionSignPos);
            landscapeName = pathInZip.substr(pathInZip.find_last_of("/\\") + 1);
            image = loadFromZipFile<Image>(pathToZip, pathInZip);
        }
        else{
            landscapeName = args.inputHeightmap.substr(args.inputHeightmap.find_last_of("/\\") + 1);
            image = loadFromFile<Image>(args.inputHeightmap);
        }
    }
    catch(const std::runtime_error& e){
        std::cerr << "Error while reading heightmap file: " << e.what() << std::endl;
        return 1;
    }
    //drop hydra factories
    dropFactories();
    assert(image);

    const unsigned int IMAGE_WIDTH = image->getWidth();
    const unsigned int IMAGE_HEIGHT = image->getHeight();
    
    //map which contains data to be processed
    std::map<TerrainFragmentId, HeightMapPtr> fragmentData;

    try{
        //we should fill map with data by splitting original heightmap
        //into smaller ones (one for each fragment)
        fragmentData = splitIntoFragments(image, args.fragmentSize, args.maxHeight);
    }
    catch(const std::runtime_error& e){
        std::cerr << "Error while splitting heightmap into fragments: " << e.what() << std::endl;
        return 1;
    }

    assert(!fragmentData.empty());

    //we don't need image data anymore, all the data is stored in fragments
    image.reset();

    FileSystemPtr fileSystem;
    if(!args.outZip.empty()){
        fileSystem = FileSystemPtr(new ZipFileSystem(args.outZip));
    }
    else if(!args.outDir.empty()){
        fileSystem = FileSystemPtr(new FileFileSystem(args.outDir));
    }
    else{
        assert(false);
        return 1;
    }

    //now we should handle all the fragments and generate tree of LODs for each one.
    //Then we should save those trees to files and save some metadata to make
    //renderer's life easier.
    //All those stuff is done in multiple threads.
    boost::thread_group threadGroup;

    TerrainPreprocessor::Properties properties;
    properties.numOfLODs = args.lodNum;
    properties.maxError = args.lodError;
    properties.LODErrorFactor = args.lodErrorFactor; 
    properties.vertexLODFactor = args.vertexLODFactor;
    properties.generateSkirts = true;
    
    ThreadStart threadStart(fragmentData, properties, fileSystem);

    for(unsigned int i = 0; i < args.threadNum; ++i){
        threadGroup.create_thread(threadStart);
    }
    
    //wait till they finish
    threadGroup.join_all();

    //we should create and save main metadata file
    saveMainMeta(IMAGE_WIDTH, IMAGE_HEIGHT, args.fragmentSize, ThreadStart::sFileList, landscapeName, fileSystem);

    return 0;
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
