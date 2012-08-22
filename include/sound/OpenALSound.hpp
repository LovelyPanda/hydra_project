//OpenALSound.hpp

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


#ifndef OPEN_AL_SOUND_HPP__
#define OPEN_AL_SOUND_HPP__

/** 
 * \class hydra::sound::OpenALSound
 * \brief Implementation of Sound interface using OpenAL.
 *
 * Renders sound using OpenAL.
 *
 * \author A.V.Medvedev
 * \date 02.08.2010
 */

#include "sound/Sound.hpp"
#include "common/SharedPtr.hpp"
#include "common/PimplPtr.hpp"

namespace hydra{

namespace sound{

class OpenALSound: public hydra::sound::Sound{

public:
    ///default constructor
    OpenALSound();

    ///destructor
    virtual ~OpenALSound();

    ///loads sound track
    virtual unsigned int loadTrack(const hydra::data::SoundTrack& inTrack);

    ///plays sound track loaded with loadTrack function
    virtual void playTrack(unsigned int inTrackName, bool inRepeatFlag);

private:
    //pimpl idiom
    struct Impl;
    hydra::common::PimplPtr<Impl>::Type mImpl;

};

///pointer to OpenALSound object
typedef hydra::common::SharedPtr<hydra::sound::OpenALSound>::Type OpenALSoundPtr;

} //sound namespace

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
