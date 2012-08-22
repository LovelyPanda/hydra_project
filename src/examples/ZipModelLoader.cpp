//ZipModelLoader.cpp

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
 * Simple sample console program.
 * Loads specified file using loading library from Hydra project.
 * Prints information, warnings, errors and benchmarks.
 */

//main loading header
#include "loading/LoadingMain.hpp"
#include "rendering/MeshOptimizer.hpp"
#include "data/Model.hpp"

#include <boost/foreach.hpp>
#include <iostream>
#include <string>
#include <stdexcept>
#include <ctime>

using namespace hydra::loading;
using namespace hydra::data;

void printHelp(){
    std::cout << "\n\t========= HELP MESSAGE ===========\n\
        Simple sample console program.\n\
        Loads specified file (model) using loading library from Hydra project.\n\
        Then it optimizes meshs of loaded model using MeshOptimizer\n\
        Prints information, warnings, errors and benchmarks.\n\
        \n\
        You should specify zip archive as first argument and file to load (inside archive) as second one." << std::endl;
}

int main(int argv, char** args){
    std::cout << "=========== started ===========" << std::endl;

    //init factories for loading stuff
    initFactories();
    
    ModelPtr model;
    ModelPtr model_copy;

    CachePtr cache(new Cache<hydra::data::CacheableTypes>());

    clock_t clockBeforeLoading = clock();

    try{
        //load model
        if(argv > 2){
            model = loadFromZipFile<Model>(args[1], args[2], cache);
            model_copy = loadFromZipFile<Model>(args[1], args[2], cache);
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
    std::cout << "successfully loaded model" << std::endl;
    std::cout << "time elapsed: " << static_cast<float>(clock() - clockBeforeLoading)/CLOCKS_PER_SEC << " s" << std::endl;
    clock_t clockBeforeOptimization = clock();
    std::cout << "optimization goes now..." << std::endl;

    hydra::rendering::MeshOptimizer meshOpt;
    BOOST_FOREACH(MeshPtr nextMesh, model->mMeshes){
        meshOpt.optimizeMesh(nextMesh);
    }

    std::cout << "time elapsed: " << static_cast<float>(clock() - clockBeforeOptimization)/CLOCKS_PER_SEC << " s" << std::endl;

    //we don't need loaders any more
    dropFactories();
    //drop cache
    cache.reset();

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
