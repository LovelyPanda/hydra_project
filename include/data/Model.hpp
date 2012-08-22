//Model.hpp

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


#ifndef MODEL_HPP__
#define MODEL_HPP__

/**
 * \class hydra::data::Model
 * \brief Class for representing a 3D model.
 * 
 * Contains all the needed data for drawing the model.
 * Model may contain data to create animation.
 *
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

#include "common/SharedPtr.hpp"
#include <vector>
#include <map>
#include <string>

namespace hydra{

namespace data{

//forward declarations
class Mesh;
struct Animation;
class Image;
struct Skeleton;
typedef common::SharedPtr<Mesh>::Type MeshPtr;
typedef common::SharedPtr<Animation>::Type AnimationPtr;
typedef common::SharedPtr<Image>::Type ImagePtr;
typedef common::SharedPtr<Skeleton>::Type SkeletonPtr;

class Model{

public:
    ///container for images
    typedef std::map<std::string, hydra::data::ImagePtr> ImageCont;

    ///container for Meshes
    typedef std::vector<hydra::data::MeshPtr> MeshCont;

    ///container for animations
    typedef std::vector<hydra::data::AnimationPtr> AnimationCont;

    ///default ctor. Builds empty model.
    Model();

    ///Constructor which builds empty model with specified name
    Model(const std::string& inName);

    ///copy-constructor
    ///\warning Copy-constructor does not copy images which are used by Meshes of this model
    ///Only pointers are copied so 2 models will use same objects after copy-construction
    ///If you are going to modify some images and don't want those modifications
    ///to affect other models, you should copy-construct images yourself. 
    ///Animations are also not copy-constructed.
    ///Meshes and bind-pose skeleton are copy-constructed.
    explicit Model(const Model& inModel);

    ///assign operator
    ///\warning assign operator does not create new images (see warning above)
    ///but it does create new Mesh objects
    Model& operator=(const Model& inModel);

    ///Get image with specified name.
    ///Returns empty pointer if no such image.
    inline const hydra::data::ImagePtr getImage(const std::string& inId) const{
        ImageCont::const_iterator iter = mImages.find(inId);
        if(iter != mImages.end()) return (*iter).second;
        else return hydra::data::ImagePtr();
    }

    ///add new Image (do nothing if image with such id is already present)
    void addImage(const std::string& inId, hydra::data::ImagePtr inImage);

    ///sorts meshes by their material's opacity
    void sortMeshesByOpacity();

    ///drops unused (empty) meshes if any
    void dropEmptyMeshes();

    ///clears all data
    void clear();

    ///adds animation an makes important conversions to frame's skeletons
    ///(it changes the object you put, not a copy)
    void addAnimation(hydra::data::AnimationPtr inAnimation);

    ///Returns pointer to animation with specified index.
    ///Returns empty pointer of no such animation.
    inline hydra::data::AnimationPtr getAnimation(size_t inIndex){
        if(inIndex < mAnims.size()) return mAnims[inIndex];
        else return hydra::data::AnimationPtr();
    }

    ///returns number of animations
    inline size_t getAnimationNum() const{
        return mAnims.size();
    }

    ///deletes animation with specified number
    inline void deleteAnimation(size_t inIndex){
        if(inIndex < mAnims.size()) mAnims.erase(mAnims.begin() + inIndex);
    }

    ///name of model
    std::string mName;

    ///Meshes
    MeshCont mMeshes;

    ///images used by materials of meshes
    ImageCont mImages;

    ///skeleton in bind position
    hydra::data::SkeletonPtr mBindSkel;

private:
    ///animations (sequences of frames)
    AnimationCont mAnims;

};

///pointer (smart) to Model object
typedef common::SharedPtr<hydra::data::Model>::Type ModelPtr;

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
