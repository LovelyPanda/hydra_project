//SoundTrack.hpp

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


#ifndef SOUND_TRACK_HPP__
#define SOUND_TRACK_HPP__

/**
 * \class hydra::data::SoundTrack
 * \brief Class for representing a data needed to play some sound track.
 *
 * SoundTrack is like an hydra::data::Image but for sound. It stores 
 * bytes of sound data (buffer) and some variables to interpret this data in
 * a right way (like frequency, format, etc)
 *
 * \author A.V.Medvedev
 * \date 02.08.2010
 */

#include "common/SharedPtr.hpp"
#include "common/PimplPtr.hpp"
#include <string>

namespace hydra{

namespace data{

class SoundTrack{

public:

    ///format of sound data
    enum Mode{
        NONE = 0,
        MONO_8,
        MONO_16,
        STEREO_8,
        STEREO_16
    };

    ///creates empty SoundTrack object
    SoundTrack();

    ///creates SoundTrack object
    SoundTrack(unsigned char* inData, SoundTrack::Mode inMode, 
        unsigned int inSize, unsigned int inFreq, const std::string& inName = "");

    ///copy-constructor
    explicit SoundTrack(const SoundTrack& inSoundTrack);

    ///destructor
    ~SoundTrack();

    ///sets new data. Old data (if any) is deleted.
    void setData(unsigned char* inData);
    
    ///\brief Returns sound data pointer
    ///\warning Do not delete Image object 
    ///or set new data if someone still uses pointer returned from here.
    const unsigned char* getData() const;

    ///returns sound data length (in bytes)
    unsigned int getSize() const;

    ///returns SoundTrack's name (it may be its path)
    const std::string& getName() const;

    ///sets SoundTrack's name
    void setName(const std::string& inName);
    
    ///sets new sound data length (in bytes)
    ///\warning Be carefull with this function
    void setSize(unsigned int inSize);

    ///returns sound data format
    SoundTrack::Mode getMode() const;

    ///sets sound data format
    void setMode(SoundTrack::Mode inMode);

    ///returns sound track frequency
    unsigned int getFrequency() const;

    ///sets sound track frequency
    void setFrequency(unsigned int inFreq);

private:
    ///pimpl idiom
    struct Impl;
    hydra::common::PimplPtr<Impl>::Type mImpl;
};

///pointer to sound track
typedef hydra::common::SharedPtr<hydra::data::SoundTrack>::Type SoundTrackPtr;

} //data namespace

} //hydra
#endif

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
