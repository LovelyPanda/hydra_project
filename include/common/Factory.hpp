//Factory.hpp

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


#ifndef FACTORY_HPP__
#define FACTORY_HPP__

/**
 * \class hydra::common::Factory
 * \brief Factory class. Implementation of factory design pattern.
 * 
 * BASE argument is the base class (may be abstract).
 * Users can instantiate Factory template
 * to get factories for different base classes and then register 
 * some derived classes (different at a time) at appropriate 
 * factory to have ability to get objects of derived classes
 * available by pointer of base class type. See factory design
 * pattern for more info.
 *
 * \author A.V. Medvedev
 * \date 26.04.2010
 *
 */


#include <string>
#include <map>
#include "common/SharedPtr.hpp"

namespace hydra{

namespace common{

template <class BASE>
class Factory{

public:
    ///Pointer(smart) of BASE type
    typedef typename common::SharedPtr<BASE>::Type BasePtr;
    ///Pointer(smart) of Factory<BASE> type
    typedef typename common::SharedPtr< Factory<BASE> >::Type FactoryPtr;

    ///Register new element from specified pointer.
    ///Pointer is shared so you can (and should) use same pointer for different id's.
    ///You can't reg element if id has been used before.
    bool reg(const std::string& id, BasePtr inPtr){
        if(mFactory[id]) return false; //if there already is specified key
        mFactory[id] = inPtr;
        return true;
    }

    ///get registered element with specified id
    BasePtr get(const std::string& id){
            //if there is nothing to return,
            //new pair is created with specified key and empty pointer is returned.
            return mFactory[id];
    }

    ///drop element with specified id only.
    void drop(const std::string& id){
        mFactory.erase(id);
    }

    ///drops all stuff that has been registered so far
    void drop(){
        mFactory.clear();
    }

private:
    ///container for factory data (new type)
    typedef std::map<std::string, BasePtr> FactoryContainer;
    ///container itself
    FactoryContainer mFactory;
};

} //common namespace

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
