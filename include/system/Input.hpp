//Input.hpp

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



#ifndef INPUT_HPP__
#define INPUT_HPP__

/**
 * \class hydra::system::Input
 * \brief Abstract class for representing window's input.
 *
 * You may handle your window's input using apropriate 
 * implemetation of this abstract class.
 *
 * \see hydra::system::Window
 *
 * \todo Add joystick support
 *
 * \author A.V.Medvedev
 * \date 30.07.2010
 */

#include <boost/noncopyable.hpp>
#include "common/SharedPtr.hpp"

namespace hydra{
namespace system{

struct Event;

class Input: private boost::noncopyable{

public:
    ///destructor
    virtual ~Input(){}

    ///gets next event in event pool. If pool is empty returns empty event.
    virtual hydra::system::Event getNextEvent() = 0;

    ///returns next event. If pool is empty it will wait for event indefinitely long. If returned event is empty, error accured.
    virtual hydra::system::Event waitForNextEvent() = 0;

    /**
     * \brief Sets keyboard key repeat rate.
     * \param inDelay specifies how long the key must be pressed before it begins repeating (ms). If '0' is used default delay value will be set.
     * \param inInterval specifies interval between key repetitions (ms). If '0' is used default interval value will be set.
     */
    virtual void enableKeyRepeat(unsigned int inDelay = 0, unsigned int inInterval = 0) = 0;

    ///disables key repeat
    virtual void disableKeyRepeat() = 0;

    ///deletes all the pool's events
    virtual void dropEvents() = 0;

    ///sets cursor position to specified one
    virtual void warpMouse(unsigned int inX, unsigned int inY) = 0;

    ///grabs mouse input (mouse can't leave window while grabbed)
    virtual void grabMouse() = 0;

    ///ungrabs mouse back to normal
    virtual void ungrabMouse() = 0;

    ///whether input is grabbed
    virtual bool isGrabbed() = 0;

};

///pointer to input
typedef common::SharedPtr<hydra::system::Input>::Type InputPtr;

} //system namespace

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
