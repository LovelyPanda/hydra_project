//MD5AnimationLoader.cpp

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

#include "loading/MD5AnimationLoader.hpp"
#include "MD5Common.hpp"
#include "data/Animation.hpp"
#include "data/Skeleton.hpp"
#include "data/Bone.hpp"
#include "loading/Manager.hpp"
#include "math/Point.hpp"
#include "math/Vector3D.hpp"
#include "math/Quat.hpp"

#include <istream>
#include <string>
#include <algorithm>
#include <cassert>

using hydra::math::Quat;
using hydra::math::Vector3D;
using hydra::math::Point;
using hydra::data::Animation;
using hydra::data::AnimationPtr;
using hydra::data::Skeleton;
using hydra::data::SkeletonPtr;
using hydra::loading::MD5AnimationLoader;
using hydra::loading::Manager;
using hydra::data::Bone;

using namespace hydra::MD5Common;

struct MD5AnimationLoader::MD5AnimImpl{

   AnimationPtr loadAnim(std::istream& inSource, Manager& inManager){
        if(!inSource.good()) fatalError("stream is in bad condition");

        //check header
        checkAndEatHeader(inSource);

        //get numFrames
        size_t numFrames = getNamedValue(inSource, "numFrames");
        //get numJoints
        size_t numJoints = getNamedValue(inSource, "numJoints");
        //get framerate
        size_t frameRate = getNamedValue(inSource, "frameRate");
        //get numAnimatedComponents
        size_t numAnimatedComponents = getNamedValue(inSource, "numAnimatedComponents");

        std::string token;

        //get hierarchy data
        findNextToken(inSource);
        std::getline(inSource, token, ' ');
        if(token != "hierarchy") fatalError("'hierarchy' wanted.");

        //eat {
        eatChar(inSource, '{');

        std::vector<Bone> joints;
        std::vector<int> jointFlags;
        std::vector<int> jointStartIndices;
        joints.reserve(numJoints);
        jointFlags.reserve(numJoints);
        jointStartIndices.reserve(numJoints);


        for(size_t i = 0; i < numJoints; ++i){
            //read next joint
            // '"' must be here
            eatChar(inSource, '"');

            hydra::data::Bone nextBone;
            //read name
            std::getline(inSource, nextBone.mName, '"');
            findNextToken(inSource);
            //read parent's id
            inSource >> nextBone.mParent;
            joints.push_back(nextBone);
            findNextToken(inSource);
            //read flags
            int flags = 0;
            inSource >> flags;
            jointFlags.push_back(flags);
            
            int startIndex = -1;
            inSource >> startIndex;
            jointStartIndices.push_back(startIndex);
        }

        //eat '}'
        eatChar(inSource, '}');

        //ignore bounds for now
        //TODO: save them somewhere (may be)
        
        //bounds data
        findNextToken(inSource);
        std::getline(inSource, token, ' ');
        if(token != "bounds") fatalError("'bounds' wanted.");

        eatChar(inSource, '{');
        
        //bounding boxes for each frame
        //for(size_t i = 0; i < numFrames; ++i){
        //    
        //}

        //just eat till '}'
        inSource.ignore(1000*numFrames, '}');

        //baseframe goes now
        findNextToken(inSource);
        std::getline(inSource, token, ' ');
        if(token != "baseframe") fatalError("'baseframe' wanted");

        eatChar(inSource, '{');

        for(size_t i = 0; i < numJoints; ++i){
            eatChar(inSource, '(');
            float x, y, z;
            //read position
            inSource >> x >> y >> z;
            joints[i].mPos = Vector3D(x, y, z);

            eatChar(inSource, ')');

            eatChar(inSource, '(');
            //read orientation
            inSource >> x >> y >> z;
            joints[i].mOrient = buildUnitQuat(x, y, z);
            eatChar(inSource, ')');
        }

        eatChar(inSource, '}');

        std::vector< std::vector<float> > frameData;
        frameData.resize(numFrames);

        //read frame data
        for(size_t i = 0; i < numFrames; ++i){
            findNextToken(inSource);
            std::getline(inSource, token, ' ');
            if(token != "frame") fatalError("'frame' wanted");
            
            int frameIndex = -1;
            findNextToken(inSource);
            inSource >> frameIndex;

            eatChar(inSource, '{');

            frameData[frameIndex].resize(numAnimatedComponents);
            for(size_t j = 0; j < numAnimatedComponents; ++j){
                findNextToken(inSource);
                inSource >> frameData[frameIndex][j];
            }

            eatChar(inSource, '}');
        }

        //all the data read
        //now we should build frames
        AnimationPtr anim(new Animation());
        anim->mFrames.resize(numFrames);
        //for each frame
        for(size_t i = 0; i < numFrames; ++i){
            anim->mFrames[i].mBones.resize(numJoints);
            
            //for each joint
            for(size_t j = 0; j < numJoints; ++j){
                //copy bone
                hydra::data::Bone currentBone = joints[j];

                int startIndex = jointStartIndices[j];
                    
                if (jointFlags[j] & 1){
                    currentBone.mPos.setX(frameData[i][startIndex]);
                    ++startIndex;
                }
                if(jointFlags[j] & 2){
                    currentBone.mPos.setY(frameData[i][startIndex]);
                    ++startIndex;
                }
                if(jointFlags[j] & 4){
                    currentBone.mPos.setZ(frameData[i][startIndex]);
                    ++startIndex;
                }

                Vector3D newQuat;
                if(jointFlags[j] & 8){
                    newQuat.setX(frameData[i][startIndex]);
                    ++startIndex;
                }
                if(jointFlags[j] & 16){
                    newQuat.setY(frameData[i][startIndex]);
                    ++startIndex;
                }
                if(jointFlags[j] & 32){
                    newQuat.setZ(frameData[i][startIndex]);
                    ++startIndex;
                }

                currentBone.mOrient = buildUnitQuat(newQuat.x(), newQuat.y(), newQuat.z());

                //assuming parent bone is alredy handled
                int parentIndex = currentBone.mParent;

                //if has parent
                if(parentIndex >= 0){
                    const hydra::data::Bone& parentBone = anim->mFrames[i].mBones[parentIndex];
                    //rotate position
                    parentBone.mOrient.rotate(currentBone.mPos);
                    currentBone.mPos += parentBone.mPos;
                    currentBone.mOrient = parentBone.mOrient * currentBone.mOrient;
                    currentBone.mOrient.normalize();
                }
                
                anim->mFrames[i].mBones[j] = currentBone;

            }
        }
        anim->mFramerate = static_cast<float>(frameRate);

        return anim;
    }

};

MD5AnimationLoader::MD5AnimationLoader(): mImpl(new MD5AnimationLoader::MD5AnimImpl()){

}

MD5AnimationLoader::~MD5AnimationLoader(){

}

void MD5AnimationLoader::init(){

}

bool MD5AnimationLoader::isInited() const{
    return true;
}

AnimationPtr MD5AnimationLoader::doLoad(std::istream& inSource, Manager& inManager, const std::string& inDataType){
    assert(mImpl);
    //get model's type from input string
    std::string sourceDataType = inDataType;
    //transform input string to lower case
    std::transform(sourceDataType.begin(), sourceDataType.end(), 
                   sourceDataType.begin(), tolower);

    if(inDataType == "md5anim") return mImpl->loadAnim(inSource, inManager);
    else throw std::runtime_error("MD5AnimationLoader: error, can't load data of specified type");
}

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
