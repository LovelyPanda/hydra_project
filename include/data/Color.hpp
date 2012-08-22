//Color.hpp

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


#ifndef COLOR_HPP__
#define COLOR_HPP__

/**
 * \class hydra::data::Color
 * \brief Class for representing RGB color
 *
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

namespace hydra{

namespace data{

class Color{

public:
    ///Default constructor. Creates white color.
    inline Color():mRed(1.0f), mGreen(1.0f), mBlue(1.0f), mAlpha(1.0f){
    
    }
    
    ///Creates color with specified values.
    explicit inline Color(float inRed, float inGreen, float inBlue, float inAlpha = 1.0f): mRed(inRed), mGreen(inGreen), mBlue(inBlue), mAlpha(inAlpha){
    
    }
    
    ///Returns red component of color.
    inline float r() const{
        return mRed;
    }

    ///Returns green component of color.
    inline float g()  const{
        return mGreen;
    }

    ///Returns blue component of color.
    inline float b() const{
        return mBlue;
    }

    ///Returns alpha component of color
    inline float a() const{
        return mAlpha;
    }

    ///Returns red component of color as unsigned char.
    inline unsigned char ru() const{ // Россия, вперед! :)
        return static_cast<unsigned char>(mRed * 255);
    }

    ///Returns green component of color as unsigned char.
    inline unsigned char gu() const{
        return static_cast<unsigned char>(mGreen * 255);
    }

    ///Returns blue component of color as unsigned char.
    inline unsigned char bu() const{
        return static_cast<unsigned char>(mBlue * 255);
    }

    ///Returns alpha component of color as unsigned char
    inline unsigned char au() const{
        return static_cast<unsigned char>(mAlpha * 255);
    }

private:
    ///red component
    float mRed;
    ///green component
    float mGreen;
    ///blue component
    float mBlue;
    ///alpha component
    float mAlpha;
};

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
