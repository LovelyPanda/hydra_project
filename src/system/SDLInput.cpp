//SDLInput.cpp

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

#include "system/SDLInput.hpp"
#include "system/Event.hpp"

#include <cassert>
#include <stdexcept>

#include "SDL/SDL.h"

using hydra::system::SDLInput;
using hydra::system::Input;
using hydra::system::Event;

struct SDLInput::Impl{

    Event::KeyModifiers convertSDLModifiers(SDLMod inMod){
        if(inMod == KMOD_NONE) return Event::NONE;

        int ourMod = Event::NONE;
        if(inMod & KMOD_NUM) ourMod |= Event::NUM_MASK;
        if(inMod & KMOD_CAPS) ourMod |= Event::CAPS_MASK;
        if(inMod & KMOD_LCTRL) ourMod |= Event::LCTRL_MASK;
        if(inMod & KMOD_RCTRL) ourMod |= Event::RCTRL_MASK;
        if(inMod & KMOD_LALT) ourMod |= Event::LALT_MASK;
        if(inMod & KMOD_RALT) ourMod |= Event::RALT_MASK;
        if(inMod & KMOD_LSHIFT) ourMod |= Event::LSHIFT_MASK;
        if(inMod & KMOD_RSHIFT) ourMod |= Event::RSHIFT_MASK;

        return static_cast<Event::KeyModifiers>(ourMod);
    }

    Event translateSDLEvent(const SDL_Event& inEvent){
        Event event;
        switch (inEvent.type){
            case SDL_ACTIVEEVENT:
                //mouse focus
                if(inEvent.active.state == SDL_APPMOUSEFOCUS){
                    if(inEvent.active.gain == 0) event.type = Event::MOUSE_FOCUS_LEAVE;
                    else event.type = Event::MOUSE_FOCUS_GAIN;
                }
                //input focus (activation/deactivation)
                else if(inEvent.active.state == SDL_APPINPUTFOCUS){
                    if(inEvent.active.gain == 0) event.type = Event::ACTIVATE;
                    else event.type = Event::DEACTIVATE;
                }
                //handle nothing more
                else event.type = Event::EMPTY;
                break;
            case SDL_KEYDOWN:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_KEYUP:
            case SDL_MOUSEBUTTONUP:
                if(inEvent.type == SDL_KEYDOWN || inEvent.type == SDL_MOUSEBUTTONDOWN) 
                    event.type = Event::KEY_PRESS;
                else event.type = Event::KEY_RELEASE;
                if(SDL_KEYDOWN == inEvent.type || SDL_KEYUP == inEvent.type){
                    event.key = static_cast<Event::KeyCode>(inEvent.key.keysym.sym); //we have same codes with SDL
                    //modifiers however must be converted
                    event.mod = convertSDLModifiers(inEvent.key.keysym.mod);
                }
                else{
                    //save mouse key code
                    event.key = static_cast<Event::KeyCode>(Event::MOUSE_LEFT + inEvent.button.button);
                    //think about modifiers here...
                    event.mod = Event::NONE;

                    //save mouse coordinates when click acqured
                    event.x = inEvent.button.x;
                    event.y = inEvent.button.y;
                }
                break;
            case SDL_MOUSEMOTION: {
                event.type = Event::MOUSE_MOTION;
                event.x = inEvent.motion.x;
                event.y = inEvent.motion.y;
                int i = 0;
                const int MAX_MOUSE_KEY_NUMBER = 7;
                for(i = 1; i < MAX_MOUSE_KEY_NUMBER; ++i)
                    if(inEvent.motion.state & SDL_BUTTON(i)) break;
                if(i < MAX_MOUSE_KEY_NUMBER) event.key = static_cast<Event::KeyCode>(Event::MOUSE_LEFT + (i - 1));
                else event.key = Event::UNKNOWN;

                break;
                                  }
            case SDL_QUIT:
                event.type = Event::QUIT;
                break;
            case SDL_VIDEORESIZE:
                event.type = Event::RESIZE;
                event.x = inEvent.resize.w;
                event.y = inEvent.resize.h;
                break;
            case SDL_VIDEOEXPOSE:
                event.type = Event::EXPOSE;
                break;
        }

        return event;
    }

    inline Event createEmptyEvent(){
        Event event;
        event.type = Event::EMPTY;
        return event;
    }

};

SDLInput::SDLInput(): mImpl(new SDLInput::Impl()){
    if(!(SDL_WasInit(SDL_INIT_EVERYTHING) & SDL_INIT_VIDEO)) throw std::runtime_error("SDLInput: can't init without window init using SDLWindow.");
}

SDLInput::~SDLInput(){

}

Event SDLInput::getNextEvent(){
    assert(mImpl);

    SDL_Event sdlEvent;
    Event lastEvent;

    do{
        if(!SDL_PollEvent(&sdlEvent)){
            //no event
            return mImpl->createEmptyEvent();
        }

        lastEvent = mImpl->translateSDLEvent(sdlEvent);
    } while(lastEvent.type == Event::EMPTY);

    assert(lastEvent.type != Event::EMPTY);
    return lastEvent;
}

Event SDLInput::waitForNextEvent(){
    assert(mImpl);

    SDL_Event sdlEvent;
    Event lastEvent;

    do{
        if(!SDL_WaitEvent(&sdlEvent)){
            //error acquired
            return mImpl->createEmptyEvent();
        }
        lastEvent = mImpl->translateSDLEvent(sdlEvent);
    } while(lastEvent.type != Event::EMPTY);

    assert(lastEvent.type != Event::EMPTY);
    return lastEvent;
}

void SDLInput::enableKeyRepeat(unsigned int inDelay, unsigned int inInterval){
    assert(mImpl);

    if(inDelay == 0) inDelay = SDL_DEFAULT_REPEAT_DELAY;
    if(inInterval == 0) inInterval = SDL_DEFAULT_REPEAT_INTERVAL;
    
    if(!SDL_EnableKeyRepeat(inDelay, inInterval)) throw std::runtime_error("SDLInput: error while setting key repeat rate.");
}

void SDLInput::disableKeyRepeat(){
    assert(mImpl);

    if(!SDL_EnableKeyRepeat(0, 0)) throw std::runtime_error("SDLInput: error while disabling key repeat.");
}

void SDLInput::dropEvents(){
    assert(mImpl);

    SDL_Event sdlEvent;
    while(SDL_PollEvent(&sdlEvent));
}

void SDLInput::warpMouse(unsigned int inX, unsigned int inY){
    assert(mImpl);

    SDL_WarpMouse(inX, inY);
}

void SDLInput::grabMouse(){
    assert(mImpl);

    SDL_WM_GrabInput(SDL_GRAB_ON);
}

void SDLInput::ungrabMouse(){
    assert(mImpl);

    SDL_WM_GrabInput(SDL_GRAB_OFF);
}

bool SDLInput::isGrabbed(){
    assert(mImpl);

    return (SDL_GRAB_ON == SDL_WM_GrabInput(SDL_GRAB_QUERY));
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
