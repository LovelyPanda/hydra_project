//HeightMap.hpp

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


#ifndef HEIGHT_MAP_HPP__
#define HEIGHT_MAP_HPP__

/**
 * \class hydra::data::HeightMap
 * \brief Abstract class for representing a height map.
 *
 * This interface is capable for read-only work
 * with heightmap data. If you want to write
 * or you want to make some implementation-specific
 * stuff you should look to implementation details
 * (particularly to HeightMapGen).
 *
 * \see hydra::data::HeightMapGen
 * 
 * \author A.V.Medvedev
 * \date 16.10.2010
 */

/**
 * \class hydra::data::HeightMapGen
 * \brief Generic map of heights (implementation of HeightMap interface).
 *
 * This template stores heights which are supplied by user.
 * Type of heights is generic. HeightMapGen has special functions
 * to retrieave data. Some of those functions decode data and
 * some of them return data unchanged.
 * This class implements HeightMap interface so it may be used
 * as its implementation. In this case user may not know what
 * type of source is used so he is able to work with any type uniformly.
 *
 * Contains encoded heights and data to decode them.
 * Data may be in any order (row-major, column-major, from bottom to top and vice versa)
 * but you should make reads assuming that same order.
 * 
 * If underlying implementation uses integers,
 * they are decoded using maxHeight and minHeight values during read
 * (result will be in interval [minHeight, maxHeight]).
 * Floats are not decoded but returned 'as is'.
 *
 * Also contains size of heightmap (width or height).
 * \warning This implementation may store only square height maps 
 * (width == height)
 * \warning This implementation supports only float, short, char, int (signed and unsigned).
 * \warning MaxHeight and MinHeight does matter only for integer types (not for floats).
 * \warning Data which is passed to HeightMap's constructor will be released in destructor,
 * so you should not touch it upon creation.
 *
 * \see hydra::data::HeightMap
 * \author A.V.Medvedev
 * \date 23.08.2010
 */

#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include <boost/noncopyable.hpp>
#include "common/SharedPtr.hpp"

namespace hydra{

namespace data{

    /*
class HeightMap: private boost::noncopyable{

public:
    ///empty virtual destructor
    virtual ~HeightMap(){

    }

    ///read height value with specified index
    virtual float get(unsigned int inIndex) const = 0;
    
    ///read height value with specified index
    inline float operator[](unsigned int inIndex) const{
        return get(inIndex);
    }
    
    ///get width of heightmap (height supposed to be same)
    virtual unsigned int getSize() const = 0;
};
*/
///smart pointer to heightmap
//typedef hydra::common::SharedPtr<HeightMap>::Type HeightMapPtr;

template <typename T>
class HeightMapGen/*: public HeightMap*/{

public:
    ///ctor. Data which is passed will be released in destructor.
    inline HeightMapGen(unsigned int inSize, T* inData): mData(inData), mSize(inSize){
        
    }

    ///copy-constructs heightmap (creates full copy of data)
    inline HeightMapGen(const HeightMapGen& inHeightMap){
        mSize = inHeightMap.getSize();
        mData = new T[mSize * mSize];
        memcpy(static_cast<void*>(mData), static_cast<const void*>(inHeightMap.mData), sizeof(T) * mSize * mSize);

        mMaxHeight = inHeightMap.mMaxHeight;
        mMinHeight = inHeightMap.mMinHeight;
    }

    ///destructor
    /*virtual*/ ~HeightMapGen(){
        if(mData) delete[] mData;
        mData = 0;
    }

    ///assign operator. Resizes data pool if needed and copies data.
    const HeightMapGen& operator=(const HeightMapGen& rvalue){
        if(&rvalue != this){
            if(rvalue.mSize != mSize){
                if(mData) delete [] mData;
                mData = new T [rvalue.mSize * rvalue.mSize];
            }
            memcpy(static_cast<void*>(mData), static_cast<const void*>(rvalue.mData), sizeof(T) * rvalue.mSize * rvalue.mSize);
            mSize = rvalue.mSize;
            mMaxHeight = rvalue.mMaxHeight;
            mMinHeight = rvalue.mMinHeight;
        }
    }

    ///Sets max height value. This should be called before any non-raw reads from object (for integer instantiations).
    void setMaxHeight(float inMaxHeight){
        mMaxHeight = inMaxHeight;
    }
    
    ///Sets min height value. This should be called before any non-raw reads from object (for integer instantiations).
    void setMinHeight(float inMinHeight){
        mMinHeight = inMinHeight;
    }

    ///Returns width of square height map (but not a length of data array).
    /*virtual*/ unsigned int getSize() const{
        return mSize;
    }

    ///Returns raw value, stored in specified position.
    inline T getRaw(unsigned int inIndex) const{
        assert(mData);
        assert(inIndex < (mSize * mSize));
        return mData[inIndex];
    }

    ///Returns converted value from one in specified position.
    ///For integer instatiations (short, char, int ...) you should 
    ///set MaxHeight and MinHeight values before calling this.
    ///Result will be in interval [MinHeight, MaxHeight].
    /*virtual*/inline float get(unsigned int inIndex) const{
        assert(mData);
        assert(inIndex < (mSize * mSize));

        //get value
        T value = mData[inIndex];
        
        //decode value
        if(!std::numeric_limits<T>::is_integer){
            //for non-integer types just return value
            return static_cast<float>(value);
        }
        else{
            //integer types should be converted to float
            //using max and min values
            return decode(value);
        }
    }

    inline float operator[](unsigned int inIndex) const{
        return get(inIndex);
    }


private:
    ///decodes integer value and converts to float in specified interval [MinHeight, MaxHeight].
    inline float decode(T inValue) const{
        assert(mMinHeight < mMaxHeight);
        unsigned int totalNumbersNum = 
                static_cast<unsigned int>((std::numeric_limits<T>::min() < 0)?(- std::numeric_limits<T>::min()): std::numeric_limits<T>::min()) +
                std::numeric_limits<T>::max();
        float floatValue = static_cast<float>(inValue) / totalNumbersNum;
        float result = static_cast<float>(mMinHeight + floatValue * abs(mMaxHeight - mMinHeight));
        return result;
    }

    ///maximum height value
    float mMaxHeight;

    ///minimum height value
    float mMinHeight;

    ///data
    T* mData;

    ///size (width)
    unsigned int mSize;
};

typedef HeightMapGen<unsigned char> HeightMap;
typedef hydra::common::SharedPtr<HeightMap>::Type HeightMapPtr;

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
