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


#ifndef OGG_SOUND_TRACK_LOADER_HPP__
#define OGG_SOUND_TRACK_LOADER_HPP__

/**
 * \class hydra::loading::OGGSoundTrackLoader
 * \brief Sound track loader implementation using libvorbis.
 * 
 * Sound track loader which interprets and decodes data using libvorbis.
 * It is cappable for loading ogg format data.
 * \warning You shouldn't create this object by hand
 * but use special factories to get instance.
 * You shouldn't create several instances and init them.
 * If you initialize few such objects results depends on libvorbis.
 * 
 * \author A.V.Medvedev
 * \date 02.08.2010
 */

#include "loading/Loader.hpp"
#include "common/SharedPtr.hpp"
#include "common/PimplPtr.hpp"

namespace hydra{

//forward declarations
namespace data{

class SoundTrack;
typedef hydra::common::SharedPtr<SoundTrack>::Type SoundTrackPtr;

}

namespace loading{

//forward declaration
class Manager;

class OGGSoundTrackLoader: public hydra::loading::Loader<hydra::data::SoundTrack>{

public:
    ///Default constructor. Does almost nothing. Does not provide init.
	OGGSoundTrackLoader();

    ///Destructor.
	virtual ~OGGSoundTrackLoader();

protected:
    ///Does main job. May throw std::runtime_error.
    virtual hydra::data::SoundTrackPtr doLoad(std::istream& inSource, hydra::loading::Manager& inManager, const std::string& inType);

    ///Initializes libvorbis library.
    virtual void init();

    ///Returns initialization status.
    virtual bool isInited() const;

private:
    ///Implemetation (pimpl idiom)
    struct Impl;
    hydra::common::PimplPtr<OGGSoundTrackLoader::Impl>::Type mImpl;
};

} //loading namespace

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
