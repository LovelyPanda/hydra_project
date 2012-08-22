//OpenALSound.cpp

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

#include "sound/OpenALSound.hpp"
#include "data/SoundTrack.hpp"

#include <vector>
#include <boost/foreach.hpp>
#include <cassert>
#include <stdexcept>

#include <al.h>
#include <alc.h>

using hydra::sound::OpenALSound;
using hydra::data::SoundTrack;

struct OpenALSound::Impl{
    ALCdevice* device;
    ALCcontext* context;
    ALuint source;
    std::vector<ALuint> buffers;

    ALenum toOpenALFormat(SoundTrack::Mode inMode){
        switch(inMode){
            case (SoundTrack::MONO_8): return AL_FORMAT_MONO8;
            case (SoundTrack::MONO_16): return AL_FORMAT_MONO16;
            case (SoundTrack::STEREO_8): return AL_FORMAT_STEREO8;
            case (SoundTrack::STEREO_16): return AL_FORMAT_STEREO16;
            default: return static_cast<ALenum>(0);
        }
    }
};

#include <iostream>

OpenALSound::OpenALSound(): mImpl(new OpenALSound::Impl()){
    assert(mImpl);
   
    //open default device
    mImpl->device = alcOpenDevice(0);
    if(!mImpl->device) throw std::runtime_error("OpenALSound: couldn't open apropriate device.");

    //create context
    mImpl->context = alcCreateContext(mImpl->device, 0);
    if(!mImpl->context) throw std::runtime_error("OpenALSound: couldn't create sound rendering context.");

    alcMakeContextCurrent(mImpl->context);

    alGenSources(1, &mImpl->source);
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(mImpl->source, AL_POSITION, 0.0f, 0.0f, 0.0f);
}

#include <iostream>

OpenALSound::~OpenALSound(){
    assert(mImpl);

    //stop playing
    alSourceStop(mImpl->source);
    //detach buffer from source
    alSourcei(mImpl->source, AL_BUFFER, 0);
    
    //delete all buffers
    BOOST_FOREACH(ALuint buffer, mImpl->buffers){
        alDeleteBuffers(1, &buffer);
    }
    //delete source
    alDeleteSources(1, &mImpl->source);
    
    //clean
    alcMakeContextCurrent(0);
    alcDestroyContext(mImpl->context);
    alcCloseDevice(mImpl->device);
}

unsigned int OpenALSound::loadTrack(const SoundTrack& inTrack){
    assert(mImpl);

    ALuint newBufferID;
    alGenBuffers(1, &newBufferID);
    alBufferData(newBufferID, mImpl->toOpenALFormat(inTrack.getMode()), &(inTrack.getData()[0]), static_cast<ALsizei>(inTrack.getSize()), inTrack.getFrequency());
    //check error!!!
    if(!alIsBuffer(newBufferID)) throw std::runtime_error("OpenALSound: can't create buffer for specified sound track.");

    mImpl->buffers.push_back(newBufferID);

    return newBufferID;
}

void OpenALSound::playTrack(unsigned int inTrackName, bool inRepeatFlag){
    assert(mImpl);

    if(!alIsBuffer(inTrackName)) throw std::runtime_error("OpenALSound: unknown sound track name.");

    alSourcei(mImpl->source, AL_BUFFER, inTrackName);
    if(inRepeatFlag) alSourcei(mImpl->source, AL_LOOPING, AL_TRUE);

    alSourcePlay(mImpl->source);
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
