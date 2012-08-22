//Animation.hpp

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


#ifndef ANIMATION_HPP__
#define ANIMATION_HPP__

/**
 * \struct hydra::data::Animation
 * \brief struct for representing animation or sequence of frames.
 *
 * Contains data to animate mesh:
 *  - frames (Skeleton's);
 *  - framerate
 *  - name
 *  - number of frames
 *
 *  May be applied to Model to be used for animating model's meshes.
 *  In this case bones of frames' skeletons must correspond bones
 *  of model's "Bind position skeleton" (at least the number must be equal).
 *  If you'll try to animate model's meshes with wrong animations you'll
 *  get undefined behaviour.
 *
 * \author A.V.Medvedev
 * \date 09.07.2010
 */

#include <vector>
#include <string>
#include "common/SharedPtr.hpp"
#include "data/Skeleton.hpp"

namespace hydra{

namespace data{

struct Animation{

    ///returns duration of single frame from current animation sequence
    inline float getFrameDuration() const{
        if(mFramerate < 0.001f) return 0.0f;
        else return 1.0f/mFramerate;
    }

    ///returns total number of frames in this animation
    inline size_t getFrameNum() const{
        return mFrames.size();
    }

    ///container for frames
    typedef std::vector<hydra::data::Skeleton> FrameCont;

    ///frame data
    FrameCont mFrames;

    ///framerate
    float mFramerate;

    ///name
    std::string mName;
};

///pointer (smart) to Animation
typedef common::SharedPtr<hydra::data::Animation>::Type AnimationPtr;

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
