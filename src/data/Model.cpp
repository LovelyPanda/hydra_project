//Model.cpp

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

#include "data/Model.hpp"
#include "data/Mesh.hpp"
#include "data/Material.hpp"
#include "data/Image.hpp"
#include "data/Bone.hpp"
#include "data/Skeleton.hpp"
#include "data/Animation.hpp"
#include "math/Vector3D.hpp"
#include "math/Quat.hpp"

#include <algorithm>
#include <boost/foreach.hpp>

using hydra::data::Model;
using hydra::data::ModelPtr;
using hydra::data::Mesh;
using hydra::data::MeshPtr;
using hydra::data::Image;
using hydra::data::ImagePtr;
using hydra::data::Skeleton;
using hydra::data::SkeletonPtr;
using hydra::data::Animation;
using hydra::data::AnimationPtr;
using hydra::math::Vector3D;
using hydra::math::Quat;

Model::Model(){
 
}

Model::Model(const std::string& inName): mName(inName){

}

Model::Model(const Model& inModel): mName(inModel.mName), mImages(inModel.mImages), mAnims(inModel.mAnims){
    //copy Meshes
    BOOST_FOREACH(const MeshPtr& nextMesh, inModel.mMeshes)
        mMeshes.push_back(MeshPtr(new Mesh(*nextMesh)));

    if(inModel.mBindSkel){
        mBindSkel = SkeletonPtr(new Skeleton(*inModel.mBindSkel));
    }

}

Model& Model::operator=(const Model& inModel){
    if(&inModel == this) return (*this);

    mName = inModel.mName;
    //create a copy of Meshes
    BOOST_FOREACH(const MeshPtr& nextMesh, inModel.mMeshes)
        mMeshes.push_back(MeshPtr(new Mesh(*nextMesh)));
    //do not copy images, only pointers!
    mImages = inModel.mImages;
    mAnims = inModel.mAnims;
    if(inModel.mBindSkel){
        mBindSkel = SkeletonPtr(new Skeleton(*inModel.mBindSkel));
    }
    return (*this);
}

void Model::addImage(const std::string& inId, ImagePtr inImage){
    //don't add image if it is already present
    if(!mImages[inId]) mImages[inId] = inImage;
}

static bool compareMeshesByOpacity(const MeshPtr lhv, const MeshPtr rhv){
    if(!lhv || !lhv->mMaterial) return true;
    if(!rhv || !rhv->mMaterial) return false;
    return compareByOpacity(*(lhv->mMaterial), *(rhv->mMaterial));
}

void Model::sortMeshesByOpacity(){
    std::sort(mMeshes.begin(), mMeshes.end(), compareMeshesByOpacity);
}

void Model::dropEmptyMeshes(){
    //temporary model
    Model model;
    
    //fill container
    BOOST_FOREACH(MeshPtr nextMesh, mMeshes){
        //if mesh is not empty
        if(nextMesh->getIndexNum() > 0){
            model.mMeshes.push_back(nextMesh);
            //if texture is present preserve it
            if(nextMesh->mMaterial->mTexture != Material::ImageId()){
                model.addImage(nextMesh->mMaterial->mTexture, 
                        getImage(nextMesh->mMaterial->mTexture));
            }
            if(nextMesh->mMaterial->mBump != Material::ImageId()){
                model.addImage(nextMesh->mMaterial->mBump,
                        getImage(nextMesh->mMaterial->mBump));
            }
            if(nextMesh->mMaterial->mRefl != Material::ImageId()){
                model.addImage(nextMesh->mMaterial->mRefl,
                        getImage(nextMesh->mMaterial->mRefl));
            }
        }
     }
     //save non-empty meshes
     mMeshes = model.mMeshes;
     mImages = model.mImages;
}

void Model::clear(){
    mName = "";
    mMeshes.clear();
    mImages.clear();
    mBindSkel.reset();
    mAnims.clear();
}

void Model::addAnimation(AnimationPtr inAnim){
    mAnims.push_back(inAnim);
    BOOST_FOREACH(Skeleton& nextSkel, mAnims.back()->mFrames){
        for(size_t i = 0; i < nextSkel.mBones.size(); ++i){
            nextSkel.mBones[i].mOrient = nextSkel.mBones[i].mOrient * mBindSkel->mBones[i].mOrient.getInverse();
            nextSkel.mBones[i].mOrient.normalize();
            Vector3D temp = mBindSkel->mBones[i].mPos;
            nextSkel.mBones[i].mOrient.rotate(temp);
            nextSkel.mBones[i].mPos -= temp;
         }
    }
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
