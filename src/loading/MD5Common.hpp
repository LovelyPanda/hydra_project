//MD5Common.hpp

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

#ifndef MD5_COMMON_HPP__
#define MD5_COMMON_HPP__

/**
 * Common handy functions for MD5 loading liy here.
 */


#include "math/Quat.hpp"

#include <cmath>
#include <string>
#include <stdexcept>
#include <istream>
#include <cctype>

namespace hydra{

namespace MD5Common{

const int MD5_SUPPORTED_VERSION = 10;

inline void fatalError(const std::string& inErrorString){
    const std::string firstPart = "MD5Loader: ";
    throw std::runtime_error(firstPart + inErrorString);
}

//eats and ignore all spacers and comments
inline void findNextToken(std::istream& inStream){
    char c = '\0';
    while(inStream.good()){
        inStream.get(c);
        //ignore comments
        if(c == '/'){
            inStream.ignore(10000, '\n');
        }
        else if(isspace(c)) continue;
        else break;
    }
    if(inStream.good()) inStream.putback(c);
}

inline void eatChar(std::istream& inStream, const char inChar){
    findNextToken(inStream);
    char last_char = '\0';
    inStream.get(last_char);
    if(last_char != inChar) fatalError(std::string("wrong format: '") +
                                       inChar + std::string("' wanted."));
}

//get named integer positive value
inline int getNamedValue(std::istream& inStream, const std::string& inName){
    findNextToken(inStream);
    std::string token;
    std::getline(inStream, token, ' ');
    if(token != inName) fatalError(inName + " variable wanted.");
    int value = -1;
    findNextToken(inStream);
    inStream >> value;
    if(value < 0) fatalError( inName + " has wrong value.");
    return value;
}

inline std::string getNamedString(std::istream& inStream, const std::string& inName){
    findNextToken(inStream);
    std::string token;
    std::getline(inStream, token, ' ');
    if(token != inName) fatalError(inName + " variable wanted.");

    // '"' must be here
    eatChar(inStream, '"');
    //read name
    std::string value;
    std::getline(inStream, value, '"');
    return value;
}



inline math::Quat buildUnitQuat(float x, float y, float z){
    float t = 1.0f - (x * x) - (y * y) - (z * z);
    float w;

    if (t < 0.001f){
        w = 0.0f;
    }
    else{
        w = -sqrtf(t);
    }
    return math::Quat(x, y, z, w);
}

inline void checkAndEatHeader(std::istream& inStream){
        int version = getNamedValue(inStream, "MD5Version");
 
        if(version != MD5_SUPPORTED_VERSION){
            fatalError("MD5 data has wrong version.");
        }

        //get 'commandline' string
        //we don't use it but it must be present
        findNextToken(inStream);
        std::string token;
        std::getline(inStream, token, ' ');
        if(token != "commandline") fatalError("MD5 data must contain 'commandline' variable in its header.");
        findNextToken(inStream);
        //find " character
        inStream.ignore(50, '"');
        //ignore commanline data
        inStream.ignore(10000, '"');
}

} //MD5Common namespace

}//hydra

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
