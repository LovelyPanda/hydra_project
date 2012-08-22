//Image.cpp

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



#include "data/Image.hpp"
#include <cassert>
#include <cstring>

using hydra::data::Image;

struct Image::ImageImpl{

    ImageImpl():mData(0), mMode(Image::NONE), mWidth(0), mHeight(0), mName(""){
    
    }

    ImageImpl(unsigned char* inData, Image::Mode inMode, 
    unsigned int inWidth, unsigned int inHeight, const std::string& inName):
    mData(inData), mMode(inMode), mWidth(inWidth), mHeight(inHeight), mName(inName){
    
    }

    ~ImageImpl(){
        if(mData != 0) delete[] mData;
    }

    unsigned char* mData;
    Image::Mode mMode;
    unsigned int mWidth;
    unsigned int mHeight;
    std::string mName;
};

Image::Image(): mImpl(new Image::ImageImpl()){

}

Image::Image(unsigned char* inData, Image::Mode inMode, 
    unsigned int inWidth, unsigned int inHeight, const std::string& inName):
    mImpl(new Image::ImageImpl(inData, inMode, inWidth, inHeight, inName)){

}

Image::Image(const Image& inImage): mImpl(new ImageImpl()){
    const unsigned char* copyData = inImage.getData();
    //if any data to copy
    if(copyData){
        int bpp = 0; //bytes per pixel
        if(inImage.getMode() == RGB) bpp = 3;
        else if(inImage.getMode() == RGBA) bpp = 4;
        //allocate memory for new image
        mImpl->mData = new unsigned char[inImage.getWidth()*inImage.getHeight()*bpp];
        //copy image data
        memcpy(static_cast<void*>(mImpl->mData), 
                static_cast<const void*>(copyData), 
                inImage.getWidth()*inImage.getHeight()*bpp);
    }
    mImpl->mWidth = inImage.getWidth();
    mImpl->mHeight = inImage.getHeight();
    mImpl->mMode = inImage.getMode();
    mImpl->mName = inImage.getName();
}

Image::~Image(){

}

void Image::setData(unsigned char* inData){
    assert(mImpl);
    if(mImpl->mData != 0) delete[] mImpl->mData;
    mImpl->mData = inData;
}

const unsigned char* Image::getData() const{
    assert(mImpl);
    return mImpl->mData;
}

unsigned int Image::getWidth() const{
    assert(mImpl);
    return mImpl->mWidth;
}

unsigned int Image::getHeight() const{
    assert(mImpl);
    return mImpl->mHeight;
}

const std::string& Image::getName() const{
    assert(mImpl);
    return mImpl->mName;
}

void Image::setName(const std::string& inName){
    assert(mImpl);
    mImpl->mName = inName;
}

void Image::setWidth(unsigned int inWidth){
    assert(mImpl);
    mImpl->mWidth = inWidth;
}

void Image::setHeight(unsigned int inHeight){
    assert(mImpl);
    mImpl->mHeight = inHeight;
}

Image::Mode Image::getMode() const{
    assert(mImpl);
    return mImpl->mMode;
}

void Image::setMode(Image::Mode inMode){
    assert(mImpl);
    mImpl->mMode = inMode;
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
