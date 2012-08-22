//Image.hpp

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


#ifndef IMAGE_HPP__
#define IMAGE_HPP__

/**
 * \class hydra::data::Image
 * \brief Class for representing an image.
 * 
 * This data is usually created using image loader and used for creating
 * some kinds of texture objects.
 * It incapsulates:
 *  - pixels (bytes)
 *  - format of bytes (mode)
 *  - size (width and height in pixels)
 * 
 * The pimpl idiom is used to separate interface from implementation.
 *
 * \warning Pixel data which you put (by constructor or setData() method) 
 * will be deleted by Image' implementation so you shouldn't 
 * delete it yourself. Data which you gain from getData() method
 * also shouldn't be deleted, as deletion will corrupt Image object.
 *  
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

#include "common/SharedPtr.hpp"
#include "common/PimplPtr.hpp"
#include <string>

namespace hydra{

namespace data{

class Image{

public:

    ///format of image data
    enum Mode{
        NONE = 0,
        RGBA,
        RGB
    };

    ///creates empty Image object
    Image();

    ///creates Image object
    Image(unsigned char* inData, Image::Mode inMode, 
        unsigned int inWidth, unsigned int inHeight, 
        const std::string& inName = "");

    ///copy-constructor
    explicit Image(const Image& inImage);

    ///destructor
    ~Image();

    ///sets new data. Old data (if any) is deleted.
    void setData(unsigned char* inData);
    
    ///\brief Returns image's data pointer
    ///\warning Do not delete Image object 
    ///or set new data if someone still uses pointer returned from here.
    const unsigned char* getData() const;

    ///returns Image's width
    unsigned int getWidth() const;

    ///returns Image's height
    unsigned int getHeight() const;

    ///returns Image's name (it may be its path)
    const std::string& getName() const;

    ///sets Image's name
    void setName(const std::string& inName);
    
    ///sets new width
    void setWidth(unsigned int inWidth);

    ///sets new height
    void setHeight(unsigned int inHeight);

    ///returns pixel format
    Image::Mode getMode() const;

    ///sets pixel format
    void setMode(Image::Mode inMode);

private:
    ///pimpl idiom
    struct ImageImpl;
    common::PimplPtr<ImageImpl>::Type mImpl;
};

///Image pointer (smart).
typedef common::SharedPtr<hydra::data::Image>::Type ImagePtr;

} //data namespace

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
