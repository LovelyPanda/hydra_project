//Sound.hpp

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


#ifndef SOUND_HPP__
#define SOUND_HPP__

/**
 * \class hydra::sound::Sound
 * \brief Class for representing sound rendering context.
 *
 * This abstract class is a wrapper over some platform of API specific entity
 * which is able to render sound.
 *
 * \todo add new features as it has minimum functionality for now.
 *
 * \author A.V.Medvedev
 * \date 02.08.2010
 */

#include "common/SharedPtr.hpp"
#include <boost/noncopyable.hpp>

namespace hydra{

namespace data{

class SoundTrack;

}

namespace sound{

class Sound: private boost::noncopyable{

public:

///destructor. Cleans everything.
virtual ~Sound(){}

///loads sound track. Returns sound track's name. If 0 is returned - error acqured.
virtual unsigned int loadTrack(const hydra::data::SoundTrack& inTrack) = 0;

///plays previously loaded sound track
virtual void playTrack(unsigned int inTrackName, bool inRepeatFlag) = 0;

};

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
