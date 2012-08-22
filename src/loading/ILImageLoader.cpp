//ILImageLoader.cpp

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

#include "loading/ILImageLoader.hpp"
#include "data/Image.hpp"
#include "loading/Manager.hpp"
#include <istream>
#include <algorithm>
#include <vector>
#include <map>
#include <stdexcept>
#include <new>
#include <cstring>
#include "common/SharedPtr.hpp"
#include "IL/il.h"

using hydra::data::Image;
using hydra::data::ImagePtr;
using hydra::loading::ILImageLoader;
using hydra::loading::Manager;

struct ILImageLoader::ILImpl{
    ILImpl():mInitFlag(false){
  
    }

    ///loads all data from the stream. Returns data and length.
    ILubyte* loadDataFromStream(std::istream& inSource, int& sourceDataLength){

        //we don't know how much data will we get from stream
        //so we should read all the data first
        std::vector<char> data;
        const unsigned int BUFFER_SIZE = 10240; //10 Kb

        while(inSource.good() && (!inSource.eof())){
            char buffer[BUFFER_SIZE];
            //read some
            inSource.read(&buffer[0], BUFFER_SIZE);
            std::streamsize last_read = inSource.gcount();
 
            //try to resize vector
            try{
                data.resize(data.size() + last_read);
            }
            catch(const std::bad_alloc& exc){
                throw std::runtime_error("Not enough memory while reading an image data from stream.");
            }
 
            //copy new data from buffer to vector
            memcpy(static_cast<void*>(&data[data.size() - last_read]), static_cast<const void*>(&buffer[0]), last_read);
        }

        ILubyte* sourceData = new ILubyte[data.size()];
        memcpy(static_cast<void*>(&sourceData[0]), static_cast<const void*>(&data[0]), data.size());
        sourceDataLength = static_cast<int>(data.size());

        return sourceData; 
    }

    //members
    bool mInitFlag;
    std::map<std::string, ILenum> mTypeMap;
};



ILImageLoader::ILImageLoader(): mImpl(new ILImageLoader::ILImpl()){
}

ILImageLoader::~ILImageLoader(){
}

bool ILImageLoader::isInited() const{
    return mImpl->mInitFlag;
}

void ILImageLoader::init(){
    //do nothing if already inited
    if(isInited()) return;
    
    ilInit();
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

    //init type map
     mImpl->mTypeMap["bmp"] = IL_BMP;
     mImpl->mTypeMap["dds"] = IL_DDS;
     mImpl->mTypeMap["gif"] = IL_GIF;
     mImpl->mTypeMap["ico"] = IL_ICO;
     mImpl->mTypeMap["jpg"] = IL_JPG;
     mImpl->mTypeMap["gif"] = IL_GIF;
     mImpl->mTypeMap["mdl"] = IL_MDL;
     mImpl->mTypeMap["mng"] = IL_MNG;
     mImpl->mTypeMap["pcd"] = IL_PCD;
     mImpl->mTypeMap["pcx"] = IL_PCX;
     mImpl->mTypeMap["pic"] = IL_PIC;
     mImpl->mTypeMap["png"] = IL_PNG;
     mImpl->mTypeMap["pbm"] = IL_PNM;
     mImpl->mTypeMap["pgm"] = IL_PNM;
     mImpl->mTypeMap["ppm"] = IL_PNM;
     mImpl->mTypeMap["psd"] = IL_PSD;
     mImpl->mTypeMap["pxr"] = IL_PXR;
     mImpl->mTypeMap["raw"] = IL_RAW;
     mImpl->mTypeMap["rgb"] = IL_SGI;
     mImpl->mTypeMap["bw"] = IL_SGI;
     mImpl->mTypeMap["rgba"] = IL_SGI;
     mImpl->mTypeMap["sgi"] = IL_SGI;
     mImpl->mTypeMap["tga"] = IL_TGA;
     mImpl->mTypeMap["tif"] = IL_TIF;
     mImpl->mTypeMap["tiff"] = IL_TIF;
     mImpl->mTypeMap["wal"] = IL_WAL;
     mImpl->mTypeMap["xpm"] = IL_XPM;
     mImpl->mTypeMap["default_type"] = IL_TYPE_UNKNOWN;
     mImpl->mTypeMap["unknown"] = IL_TYPE_UNKNOWN;
     mImpl->mTypeMap["auto"] = IL_TYPE_UNKNOWN;
}

ImagePtr ILImageLoader::doLoad(std::istream& inSource, Manager& inManager, const std::string& inImageType){
    if(!inSource.good() || inSource.eof()){
        throw std::runtime_error("ILImageLoader: input stream is in bad condition.");
    }

    //for data loaded from stream
    ILubyte* sourceData = 0;
    int sourceDataLength = 0;

    try{
        sourceData = mImpl->loadDataFromStream(inSource, sourceDataLength);
    }
    catch(const std::exception& exc){
        throw std::runtime_error(std::string("ILImageLoader: error while loading data from stream: ") + exc.what());
    }
    assert(sourceData);

    //now load the image using DevIL
    ILuint ILImage;
    
    ilGenImages(1, &ILImage);

 	//get image's type from input string
    std::string sourceDataType = inImageType;
    //transform input string to lower case
    std::transform(sourceDataType.begin(), sourceDataType.end(), 
                   sourceDataType.begin(), tolower);

    ILenum ILtype = mImpl->mTypeMap[sourceDataType];
    
    ilBindImage(ILImage);

    //if can't load image
    if (!ilLoadL(ILtype, sourceData, sourceDataLength)) {
    	ilDeleteImages(1, &ILImage);
        delete[] sourceData;
    	throw std::runtime_error("ILImageLoader: can't load image from specified stream.");
    }
    delete[] sourceData;
    
    //get all neccessary data from DevIL image to create data::Image object
    unsigned int imageHeight = static_cast<unsigned int>(ilGetInteger(IL_IMAGE_HEIGHT));
    unsigned int imageWidth = static_cast<unsigned int>(ilGetInteger(IL_IMAGE_WIDTH));
    
    //TODO: think about representating RGB
    ILenum ILmode = IL_RGBA;
    Image::Mode imgMode = Image::RGBA;
    int ILimageDataSize = imageWidth * imageHeight * 4;

    //allocate data for data::Image object
    unsigned char* data = new unsigned char[ILimageDataSize];
    
    ilCopyPixels(0, 0, 0, imageWidth, imageHeight, 1, ILmode, IL_UNSIGNED_BYTE, data);

    //we don't need ILImage any more
    ilDeleteImages(1, &ILImage);
    if (ilGetError() != IL_NO_ERROR) {
        delete[] data;
        throw std::runtime_error("ILImageLoader: error while generating an image object");
    }

    ImagePtr resImage (new Image(data, imgMode, imageWidth, imageHeight));
    //we do not delete data here. It will be deleted by Image's destructor.
    return resImage;
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
