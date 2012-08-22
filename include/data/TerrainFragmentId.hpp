//TerrainFragmentId.hpp

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

#ifndef TERRAIN_FRAGMENT_ID_HPP__
#define TERRAIN_FRAGMENT_ID_HPP__

/**
 * \class hydra::data::TerrainFragmentId
 * \brief Identificator for TerrainFragment object.
 *
 * This identificator identifies concrete TerrainFragment object
 * from terrain surface. It stores x and y coordinate of this fragment.
 * Do not confuse this id with TerrainChunkId which full identificator for
 * TerrainChunk object (not a fragment).
 *
 * \see hydra::data::TerrainFragment
 * \see hydra::data::TerrainChunkId
 *
 * \author A.V.Medvedev
 * \date 15.09.2010
 */

namespace hydra{

namespace data{

class TerrainFragmentId{

public:

    ///empty ctor. Builds empty (zero) fragment id.
    inline TerrainFragmentId(): mX(0), mY(0){

    }
    
    ///contructs fragment id from specified x and y coords.
    inline TerrainFragmentId(int inX, int inY): mX(inX), mY(inY){

    }

    ///returns x coordinate of fragment
    inline int getX() const{
        return mX;
    }
    
    ///returns y coordinate of fragment
    inline int getY() const{
        return mY;
    }

    ///increase X coordinate
    inline void increaseX(){
        ++mX;
    }

    ///increase Y coordinate
    inline void increaseY(){
        ++mY;
    }

    ///decrease X coordinate
    inline void decreaseX(){
        --mX;
    }

    ///decrease Y coordinate
    inline void decreaseY(){
        --mY;
    }

    ///operator < for sorting and other such things
    inline bool operator<(const TerrainFragmentId& inId) const{
        if(mX != inId.mX){
            return (mX < inId.mX);
        }
        else return (mY < inId.mY);
    }

    ///operator ==
    inline bool operator==(const TerrainFragmentId& inId) const{
        return (mX == inId.mX && mY == inId.mY);       
    }

    ///operator !=
    inline bool operator!=(const TerrainFragmentId& inId) const{
        return !operator==(inId);
    }

private:
    int mX, mY;
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
