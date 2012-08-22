//LoadingMain.hpp

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

#ifndef LOADING_MAIN_HPP__
#define LOADING_MAIN_HPP__

/**
 * \file LoadingMain.hpp
 * \brief Interface for Loading Library users.
 * 
 * Few functions for work with library declared here.
 * You should also include headers for data types you want to use.
 *
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

#include "loading/FileManager.hpp"
#include "loading/ZipFileManager.hpp"
#include "common/SharedPtr.hpp"
#include "loading/Cache.hpp"

/**
 * \namespace hydra::loading
 * \brief Contains all the types and functions 
 * to load different data from different sources
 *
 * Uses data namespace and loads different entities of that namespace
 * such as Image or Model.
 */

namespace hydra{
namespace loading{

///\brief Template function for loading different objects from files
///
/// One of the main functions (for users). Loads different kinds of stuff from file
/// (depends on template parameter). Can throw std::runtime_error.
/// Client may supply Cache object (with pointer). While not supplied, caching doesn't work.
/// \warning initialization of factories must be performed before use.
/// \see hydra::loading::initFactories
template <typename T>
typename hydra::common::SharedPtr<T>::Type loadFromFile(const std::string& inPath, hydra::loading::CachePtr inCache = hydra::loading::CachePtr()){
    hydra::loading::FileManager fMan(inCache);
    //can throw std::runtime_error
    return fMan.load<T>(inPath);
}

///\brief Template function for loading different objects from zipped files
///
/// One of the main functions (for users). Loads different kinds of stuff from zipped file
/// (depends on template parameter). Can throw std::runtime_error.
/// Client may supply Cache object (with pointer). While not supplied, caching doesn't work.
/// \warning initialization of factories must be performed before use.
/// \see hydra::loading::initFactories
template <typename T>
typename hydra::common::SharedPtr<T>::Type loadFromZipFile(const std::string& inPathToZipArchive, const std::string& inPathInArchive, hydra::loading::CachePtr inCache = hydra::loading::CachePtr()){
    hydra::loading::ZipFileManager zFileMan(inPathToZipArchive, inCache);
    //can throw std::runtime_error
    return zFileMan.load<T>(inPathInArchive);
}

///\brief Initialize loader factories. 
///
/// Function which register all the known loaders at factories.
/// Must be called before loading any kind of objects.
void initFactories();

///\brief Drops all the loaders from factories.
///
/// Drops all registered loaders at factories. Can be called after all load operations to clean memory.
/// You must call initFactories again for loading routines if you droped factories using this function.
///\see loading::initFactories
void dropFactories();


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
