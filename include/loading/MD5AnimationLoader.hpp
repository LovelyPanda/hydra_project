//MD5AnimationLoader.hpp

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


#ifndef MD5_ANIMATION_LOADER_HPP__
#define MD5_ANIMATION_LOADER_HPP__

/**
 * \class hydra::loading::MD5AnimationLoader
 * \brief class for loading Animation from md5anim data sources
 *
 * Animation objects should be added to model after loading using
 * addAnimation function.
 *
 * \see hydra::data::Animation
 * \see hydra::loading::MD5ModelLoader
 *
 * \author A.V.Medvedev
 * \date 09.07.2010
 */

#include "loading/Loader.hpp"
#include "common/SharedPtr.hpp"
#include "common/PimplPtr.hpp"

namespace hydra{

namespace data{

struct Animation;
typedef hydra::common::SharedPtr<hydra::data::Animation>::Type AnimationPtr;

}

namespace loading{

//forward decl
class Manager;

class MD5AnimationLoader: public hydra::loading::Loader<hydra::data::Animation>{

public:
    ///creates empty object
    MD5AnimationLoader();

    ///clears everything up
    virtual ~MD5AnimationLoader();

protected:
    ///does real work. Called by inherited load function.
    virtual hydra::data::AnimationPtr doLoad(std::istream& source, hydra::loading::Manager& inMan, const std::string& inType);
    ///some initialization routines (if any)
    virtual void init();

    ///whether object is inited.
    virtual bool isInited() const;


private:
    ///implementation (pimpl idiom)
    struct MD5AnimImpl;
    ///pimpl
    hydra::common::PimplPtr<MD5AnimImpl>::Type mImpl;


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
