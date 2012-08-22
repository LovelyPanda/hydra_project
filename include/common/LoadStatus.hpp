//LoadStatus.hpp

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


#ifndef LOAD_STATUS_HPP__
#define LOAD_STATUS_HPP__

/**
 * \struct hydra::common::Conditional
 * \brief Simple template for storing pointer to object and status of loading process.
 *
 * While instatiated this template may store pointer to
 * some object and its status (whether it loaded, unloaded, loading is in progress, ...).
 * It may be usefull for some loadable/unloadable data.
 *
 * \author A.V.Medvedev
 * \date 05.09.2010
 */

/**
 * \enum hydra::common::LoadStatus
 *
 * \brief Enum represents load status of some object.
 *
 * Status may be 'loaded', 'unloaded', 'in progress', etc.
 */

#include "common/SharedPtr.hpp"

namespace hydra{

namespace common{

enum LoadStatus{
    UNLOADED = 0,
    LOADED,
    IN_PROGRESS
};

template <typename T>
struct Conditional{

    ///pointer to object (may be empty if object is not loaded)
    typename hydra::common::SharedPtr<T>::Type ptr;

    ///current status
    LoadStatus status;
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
