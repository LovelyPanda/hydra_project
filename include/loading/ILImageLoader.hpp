//ILImageLoader.hpp

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


#ifndef IL_IMAGE_LOADER_HPP__
#define IL_IMAGE_LOADER_HPP__

/**
 * \class hydra::loading::ILImageLoader
 * \brief Image loader implementation using DevIL.
 * 
 * Image loader which interprets and decodes data using DevIL lib.
 * \warning You shouldn't create this object by hand
 * but use special factories to get instance.
 * You shouldn't create several instances and init them.
 * If you initialize few such objects results depends on DevIL.
 * 
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

#include "loading/Loader.hpp"
#include "common/SharedPtr.hpp"
#include "common/PimplPtr.hpp"

namespace hydra{

//forward declarations
namespace data{

class Image;
typedef hydra::common::SharedPtr<Image>::Type ImagePtr;

}

namespace loading{

//forward declaration
class Manager;

class ILImageLoader: public hydra::loading::Loader<hydra::data::Image>{

public:
    ///Default constructor. Does almost nothing. Does not provide init.
	ILImageLoader();

    ///Destructor.
	virtual ~ILImageLoader();

protected:
    ///Does main job. May throw std::runtime_error.
    virtual hydra::data::ImagePtr doLoad(std::istream& inSource, hydra::loading::Manager& inManager, const std::string& inImageType);

    ///Initializes DevIL library.
    virtual void init();

    ///Returns initialization status.
    virtual bool isInited() const;

private:
    ///Implemetation (pimpl idiom)
    struct ILImpl;
    hydra::common::PimplPtr<ILImpl>::Type mImpl;
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
