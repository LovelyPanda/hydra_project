//DummyLoader.hpp

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


#ifndef DUMMY_LOADER_HPP__
#define DUMMY_LOADER_HPP__

/**
 * \class hydra::loading::DummyLoader
 * \brief Loader's interface implemetation. Does nothing.
 * 
 * May be used where the Loader object is needed (syntactically) but
 * there is nothing to load. 
 * For example is is used with DummyManager.
 * If loader will reqest some more loadings DummyManager will save requests
 * but will not accomplish them. However due to Manager's interface he must
 * create stream and pass it to some loader. So DummyManager creates empty stream pointer
 * and pass it to DummyLoader.
 *
 * \see hydra::loading::DummyManager
 * \see hydra::loading::Loader
 *
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

#include "loading/Loader.hpp"
#include "common/SharedPtr.hpp"

namespace hydra{

namespace loading{

//forward declarations
class Manager;

template <typename T>
class DummyLoader: public hydra::loading::Loader<T>{

public:
    ///pointer to load type
    typedef typename hydra::common::SharedPtr<T>::Type TPtr;

    ///creates empty object
    DummyLoader(){

    }

    ///clears everything up
    virtual ~DummyLoader(){

    }

protected:
    ///does nothing, returns empty pointer
    virtual TPtr doLoad(std::istream& source, hydra::loading::Manager& inMan, const std::string& inType){
        return TPtr();
    }
    ///does nothing
    virtual void init(){

    }

    ///always inited
    virtual bool isInited() const{
        return true;
    }
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
