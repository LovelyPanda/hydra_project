//Loader.hpp

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

#ifndef LOADER_HPP__
#define LOADER_HPP__

/**
 * \class hydra::loading::Loader
 * \brief Template class for representing abstract loader.
 *
 * Instantiations of this class must be derived to
 * implement real loaders.
 *
 * \author A.V. Medvedev
 * \date 26.04.2010
 */

#include "common/SharedPtr.hpp"
#include <istream>
#include <string>

namespace hydra{

namespace loading{

class Manager;

template <typename T>
class Loader{

public:

    ///pointer (smart) to T
    typedef typename hydra::common::SharedPtr<T>::Type TPtr; 

    ///destructor
    virtual ~Loader(){}

    ///Loading function. Must be called by client's code to load some stuff.
    TPtr load(std::istream& source, hydra::loading::Manager& inManager, const std::string& inType = "DEFAULT_TYPE"){
        if(!isInited()) init(); 
        return doLoad(source, inManager, inType);
    }

protected:
    ///Initialization procedure. Must be implemented by Loaders.
    ///This is done for some optimization and stability. Now you can create lots of
    ///Loaders objects
    virtual void init() = 0;

    ///Function which shows whether Loader has already been inited or not.
    virtual bool isInited() const = 0;

    ///Function which does real work of loading needed data. Must be implemented by Loaders.
    virtual TPtr doLoad(std::istream& source, hydra::loading::Manager& inManager, const std::string& inType) = 0;

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
