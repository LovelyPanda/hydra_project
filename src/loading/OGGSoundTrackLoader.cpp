//OGGSoundTrackLoader.hpp

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

#include "loading/OGGSoundTrackLoader.hpp"
#include "data/SoundTrack.hpp"
#include "loading/Manager.hpp"

#include <cassert>
#include <cstring>
#include <istream>
#include <string>
#include <stdexcept>
#include <vector>

#include <vorbis/vorbisfile.h>
//vorbis needs errno (its ugly, I know)
#include <cerrno>

using hydra::loading::OGGSoundTrackLoader;
using hydra::data::SoundTrack;
using hydra::data::SoundTrackPtr;
using hydra::loading::Manager;

struct OGGSoundTrackLoader::Impl{

    Impl(): initFlag(false){}

    //callback structure for libvorbisfile
    ov_callbacks callbacks;

    //read function (callback)
    typedef size_t (*read_func) (void *ptr, size_t size, size_t nmemb, void *datasource);

    bool initFlag;

};

//Function for vorbis ov_callback structure.
//It is able to read data from std::istream.
//Returns 0 if EOF. Returns 0 with errno set to non-zero when error acqured.
size_t readFromStream(void* ptr, size_t size, size_t nmemb, void* istream){
    assert(ptr);
    assert(istream);

    //this function works only with std::istream
    std::istream* inputStream = static_cast<std::istream*>(istream);

    if(inputStream->eof()) return 0;
    if(!inputStream->good()){
        errno = 1;
        return 0;
    }

    //we should try to read 'nmemb' elements of 'size' size each.

    for(size_t i = 0; i < nmemb; ++i){
        inputStream->read(static_cast<char*>(ptr) + (i * size), size);

        if(!inputStream->good()) return i;
    }

    return nmemb;
}


OGGSoundTrackLoader::OGGSoundTrackLoader(): mImpl(new OGGSoundTrackLoader::Impl()){

}

OGGSoundTrackLoader::~OGGSoundTrackLoader(){
    
}

void OGGSoundTrackLoader::init(){
    assert(mImpl);

    if(isInited()) return;

    //init callbacks struct
    mImpl->callbacks.read_func = (OGGSoundTrackLoader::Impl::read_func)(&readFromStream);
    mImpl->callbacks.seek_func = 0;
    mImpl->callbacks.close_func = 0;
    mImpl->callbacks.tell_func = 0;

    mImpl->initFlag = true;
}

bool OGGSoundTrackLoader::isInited() const{
    return mImpl->initFlag;
}


SoundTrackPtr OGGSoundTrackLoader::doLoad(std::istream& inSource, Manager& inManager, const std::string& inType){
    assert(mImpl);

    if(!inSource.good()) throw std::runtime_error("OGGSoundTrackLoader: bad input stream. Can't load data.");

    vorbis_info* oggInfo;
    OggVorbis_File oggFile;

    int resultCode = ov_open_callbacks(static_cast<void*>(&inSource), &oggFile, 0, 0, mImpl->callbacks);

    //error
    if(resultCode){
        std::string errorString = "OGGSoundTrackLoader: ";
        switch (resultCode){
            case OV_EREAD: errorString += "A read from media returned an error.";
            break;
            case OV_ENOTVORBIS: errorString += "Bitstream does not contain any Vorbis data.";
            break;
            case OV_EVERSION: errorString += "Vorbis version mismatch.";
            break;
            case OV_EFAULT: errorString += "Internal logic fault; indicates a bug or heap/stack corruption.";
            break;
            default: errorString += "Unknown Vorbis error";
            break;
        }

        ov_clear(&oggFile);

        throw std::runtime_error(errorString);
    }

    SoundTrackPtr soundTrack(new SoundTrack());

    //Get some information about the OGG file
    oggInfo = ov_info(&oggFile, -1);

    //Check the number of channels... always use 16-bit samples
    if (oggInfo->channels == 1)
        soundTrack->setMode(SoundTrack::MONO_16);
    else
        soundTrack->setMode(SoundTrack::STEREO_16);

    soundTrack->setFrequency(oggInfo->rate);

    long readBytesNum;
    int bitStreamNum;
    const size_t BUFFER_SIZE = 4096; //4 KB
    char buffer[BUFFER_SIZE];
    std::vector<unsigned char> data;

    //TODO: big endian doen not work now!
    do{
        // Read up to a buffer's worth of decoded sound data
        readBytesNum = ov_read(&oggFile, buffer, BUFFER_SIZE, 0, 2, 1, &bitStreamNum);

        //error
        if(readBytesNum < 0){
            ov_clear(&oggFile);
            throw std::runtime_error("OGGSoundTrackLoader: error while decoding vorbis data.");
        }

        // Append to end of buffer
        data.insert(data.end(), buffer, buffer + readBytesNum);
    }
    while (readBytesNum > 0);

    //copy data to SoundTrack object
    size_t trackSize = data.size();
    unsigned char* newData = new unsigned char[trackSize];
    memcpy(static_cast<void*>(newData), static_cast<const void*>(&data[0]), trackSize);

    soundTrack->setSize(trackSize);
    soundTrack->setData(newData);

    // Clean up!
    ov_clear(&oggFile);

    return soundTrack;
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
