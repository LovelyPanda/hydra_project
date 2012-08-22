//SoundPlayer.cpp

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

/**
 * Simple console program.
 * Loads specified file using loading library from Hydra project.
 * Prints information, warnings, errors and benchmarks.
 * And plays sound.
 */

//main loading header
#include "loading/LoadingMain.hpp"
#include "data/SoundTrack.hpp"

//to play sound
#include "sound/OpenALSound.hpp"

//timer
#include "common/PerformanceTimer.hpp"

#include <iostream>
#include <string>
#include <stdexcept>

using namespace hydra::loading;
using namespace hydra::data;
using namespace hydra::sound;
using namespace hydra::common;

void printHelp(){
    std::cout << "\n\t========= HELP MESSAGE ===========\n\
        Simple sample console program.\n\
        Loads specified file (sound track) using loading library from Hydra project.\n\
        Prints information, warnings, errors and benchmarks.\n\
        And plays sound.\n\
        \n\
        You should specify file to load as first argument." << std::endl;
}

int main(int argv, char** args){
    std::cout << "=========== started ===========" << std::endl;

    //init factories for loading stuff
    initFactories();
    
    SoundTrackPtr soundTrack;

    PerformanceTimer timer;
    timer.start();

    try{
        //load model
        if(argv > 1){
            soundTrack = loadFromFile<SoundTrack>(args[1], CachePtr()); //loading without caching
        }
        else{
            printHelp();
            std::cout << "Press ENTER to exit..." << std::endl;
            std::cin.get();
            return 1;
        }
    }
    catch(const std::runtime_error& err){
        std::cerr << err.what() << std::endl;
        printHelp();
        std::cout << "Press ENTER to exit..." << std::endl;
        std::cin.get(); 
        return 1;
    }
    std::cout << "successfully loaded sound track\nits size is: " << soundTrack->getSize() << " bytes" << std::endl;

    std::cout << "time elapsed: " << timer.getSeconds() << " s" << std::endl;
    std::cout << "sound system init started" << std::endl;

    OpenALSound sound;
    std::cout << "sound track loading to sound system...";

    unsigned int trackName = sound.loadTrack(*soundTrack);
    std::cout << " done!" << std::endl;

    sound.playTrack(trackName, true);

    std::cout << "playing..." << std::endl;
    std::cout << "Press ENTER to exit..." << std::endl;
    std::cin.get(); 
    
    //we don't need loaders any more
    dropFactories();

    std::cout << "=========== finished ===========" << std::endl;
    return 0;
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
