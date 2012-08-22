//SoundTrack.cpp

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

#include "data/SoundTrack.hpp"

#include <cassert>
#include <cstring>

using hydra::data::SoundTrack;

struct SoundTrack::Impl{
    Impl(): data(0), size(0), mode(SoundTrack::NONE), freq(0){}

    Impl(unsigned char* inData, SoundTrack::Mode inMode, unsigned int inSize, unsigned int inFreq, const std::string& inName): data(inData), size(inSize), mode(inMode), freq(inFreq), name(inName){}

    ~Impl(){
        if(data) delete[] data;
    }

    unsigned char* data;
    unsigned int size;
    SoundTrack::Mode mode;
    unsigned int freq;
    std::string name;
};

SoundTrack::SoundTrack(): mImpl(new SoundTrack::Impl()){

}

SoundTrack::SoundTrack(unsigned char* inData, SoundTrack::Mode inMode, unsigned int inSize, unsigned int inFreq, const std::string& inName): mImpl(new SoundTrack::Impl(inData, inMode, inSize, inFreq, inName)){

}

SoundTrack::SoundTrack(const SoundTrack& inSoundTrack): mImpl(new SoundTrack::Impl()){
    const unsigned char* copyData = inSoundTrack.getData();

    //if any data to copy
    if(copyData){
        //allocate memory for new image
        mImpl->data = new unsigned char[inSoundTrack.getSize()];
        //copy image data
        memcpy(static_cast<void*>(mImpl->data), 
                static_cast<const void*>(copyData), 
                inSoundTrack.getSize());
    }
    mImpl->size = inSoundTrack.getSize();
    mImpl->freq = inSoundTrack.getFrequency();
    mImpl->mode = inSoundTrack.getMode();
    mImpl->name = inSoundTrack.getName();
}

SoundTrack::~SoundTrack(){

}

void SoundTrack::setData(unsigned char* inData){
    assert(mImpl);

    if(mImpl->data) delete[] mImpl->data;
    mImpl->data = inData;
}

const unsigned char* SoundTrack::getData() const{
    assert(mImpl);

    return mImpl->data;
}

unsigned int SoundTrack::getSize() const{
    assert(mImpl);

    return mImpl->size;
}

const std::string& SoundTrack::getName() const{
    assert(mImpl);

    return mImpl->name;
}

void SoundTrack::setName(const std::string& inName){
    assert(mImpl);

    mImpl->name = inName;
}

void SoundTrack::setSize(unsigned int inSize){
    assert(mImpl);

    mImpl->size = inSize;
}

SoundTrack::Mode SoundTrack::getMode() const{
    assert(mImpl);

    return mImpl->mode;
}

void SoundTrack::setMode(SoundTrack::Mode inMode){
    assert(mImpl);

    mImpl->mode = inMode;
}

unsigned int SoundTrack::getFrequency() const{
    assert(mImpl);

    return mImpl->freq;
}

void SoundTrack::setFrequency(unsigned int inFreq){
    assert(mImpl);

    mImpl->freq = inFreq;
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
