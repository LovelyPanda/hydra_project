//Singleton.hpp

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


#ifndef SINGLETON_HPP__
#define SINGLETON_HPP__

/**
 * \class hydra::common::Singleton
 * \brief Singleton template class.
 * Implementation of singleton design pattern.
 * 
 * Users can instantiate Singleton template with their classes to use them
 * as singletons. While user's class is used via Singleton interface
 * there will be only 1 instance of that class.
 * And you always can get instance via instance() static method. 
 * See singleton design pattern for more info.
 * Implementation also uses lazy initialization pattern.
 *
 * \warning: do not forget to define static variable Singleton<T>::sInstance upon
 * new instantiation of template.
 *
 * \author A.V. Medvedev
 * \date 07.06.2010
 *
 */

#include "common/SharedPtr.hpp"
#include <boost/noncopyable.hpp>

namespace hydra{

namespace common{

template <typename T>
class Singleton: private boost::noncopyable{
    
public:
    ///type of singleton's argument's pointer
    typedef typename hydra::common::SharedPtr<T>::Type TPtr;
    ///type of argument
    typedef T Type;

    ///returns object (pointer)
    static TPtr instance(){
        if(!sInstance) sInstance = TPtr(new T());
        return sInstance;
    }

    ///drops object (resets it) if any
    static void drop(){
        sInstance.reset();
    }

private:
    ///variable to store instance
    static TPtr sInstance;
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
