//LoadingMain.cpp

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

#include "loading/LoadingMain.hpp"
#include "loading/DummyLoader.hpp"
#include "loading/DummyManager.hpp"

#ifdef BUILD_LOADING_WITH_IL
    #include "loading/ILImageLoader.hpp"
#endif

#include "loading/OBJModelLoader.hpp"
#include "loading/MD5ModelLoader.hpp"
#include "loading/MD5AnimationLoader.hpp"

#ifdef BUILD_LOADING_WITH_OGG
    #include "loading/OGGSoundTrackLoader.hpp"
#endif

#include "common/Factory.hpp"
#include "common/Singleton.hpp"
#include "loading/Loader.hpp"
#include "data/Image.hpp"
#include "data/Model.hpp"
#include "data/Animation.hpp"

namespace hydra{

namespace common{

template <typename T>
typename hydra::common::SharedPtr<T>::Type Singleton<T>::sInstance = typename hydra::common::SharedPtr<T>::Type();

}

namespace loading{

using hydra::common::SharedPtr;
using hydra::common::Singleton;
using hydra::common::Factory;
using hydra::loading::Loader;
using hydra::data::Image;
using hydra::data::Model;
using hydra::data::SoundTrack;
using hydra::data::Animation;


///Image loader
typedef Loader<Image> ImageLoader;
///Model loader
typedef Loader<Model> ModelLoader;
///Animation loader
typedef Loader<Animation> AnimationLoader;
///SoundTrack loader
typedef Loader<SoundTrack> SoundTrackLoader;

///pointer (smart) to loading::ImageLoader
typedef SharedPtr<ImageLoader>::Type ImageLoaderPtr;
///pointer (smart) to loading::ModelLoader
typedef SharedPtr<ModelLoader>::Type ModelLoaderPtr;
///pointer (smart) to loading::AnimationLoader
typedef SharedPtr<AnimationLoader>::Type AnimationLoaderPtr;
///pointer (smart) to loading::SoundTrackLoader
typedef SharedPtr<SoundTrackLoader>::Type SoundTrackLoaderPtr;


///Singleton Factory of image loaders
typedef Singleton<Factory<ImageLoader> > ImageLoaderFactory;
///Singleton Factory of model loaders
typedef Singleton<Factory<ModelLoader> > ModelLoaderFactory;
///Singleton Factory of animation loaders
typedef Singleton<Factory<AnimationLoader> > AnimationLoaderFactory;
///Singleton Factory of sound track loaders
typedef Singleton<Factory<SoundTrackLoader> > SoundTrackLoaderFactory;

}//loading namespace

} //hydra


void hydra::loading::initFactories(){

    //registering loaders for image factory
    hydra::loading::ImageLoaderFactory::TPtr imageFact = hydra::loading::ImageLoaderFactory::instance();
   
#ifdef BUILD_LOADING_WITH_IL
    //registering DevIL image loaders
    //we use same pointer for all the different supported image formats
    hydra::loading::ImageLoaderFactory::Type::BasePtr DevIL(new ILImageLoader());
    imageFact->reg("bmp", DevIL);
    imageFact->reg("png", DevIL);
    imageFact->reg("jpg", DevIL);
    imageFact->reg("jpeg", DevIL);
    imageFact->reg("gif", DevIL);
    imageFact->reg("tga", DevIL);
    imageFact->reg("dds", DevIL);
    imageFact->reg("gif", DevIL);
    imageFact->reg("ico", DevIL);
    imageFact->reg("mdl", DevIL);
    imageFact->reg("mng", DevIL);
    imageFact->reg("pcd", DevIL);
    imageFact->reg("pcx", DevIL);
    imageFact->reg("pic", DevIL);
    imageFact->reg("pbm", DevIL);
    imageFact->reg("pgm", DevIL);
    imageFact->reg("ppm", DevIL);
    imageFact->reg("psd", DevIL);
    imageFact->reg("pxr", DevIL);
    imageFact->reg("raw", DevIL);
    imageFact->reg("rgb", DevIL);
    imageFact->reg("bw", DevIL);
    imageFact->reg("rgba", DevIL);
    imageFact->reg("sgi", DevIL);
    imageFact->reg("tif", DevIL);
    imageFact->reg("tiff", DevIL);
    imageFact->reg("wal", DevIL);
    imageFact->reg("xpm", DevIL);
#endif

    //registering loaders for model factory
    hydra::loading::ModelLoaderFactory::TPtr modelFact = hydra::loading::ModelLoaderFactory::instance();
    //registering OBJ model loader
    hydra::loading::ModelLoaderFactory::Type::BasePtr OBJLoader(new OBJModelLoader());
    modelFact->reg("obj", OBJLoader);
    modelFact->reg("mtl", OBJLoader);

    //reg MD5 loader
    hydra::loading::ModelLoaderFactory::Type::BasePtr MD5Loader(new MD5ModelLoader());
    hydra::loading::AnimationLoaderFactory::TPtr animFact = hydra::loading::AnimationLoaderFactory::instance();
    hydra::loading::AnimationLoaderFactory::Type::BasePtr MD5AnimLoader(new MD5AnimationLoader());
    modelFact->reg("md5mesh", MD5Loader);
    animFact->reg("md5anim", MD5AnimLoader);


    hydra::loading::SoundTrackLoaderFactory::TPtr soundFact = hydra::loading::SoundTrackLoaderFactory::instance();

#ifdef BUILD_LOADING_WITH_OGG
    //reg OGG loader
    hydra::loading::SoundTrackLoaderFactory::Type::BasePtr OGGLoader(new hydra::loading::OGGSoundTrackLoader());
    
    soundFact->reg("ogg", OGGLoader);
#endif

    //dummy loaders
    hydra::loading::ModelLoaderFactory::Type::BasePtr DummyModelL(new DummyLoader<hydra::data::Model>());
    hydra::loading::ImageLoaderFactory::Type::BasePtr DummyImageL(new DummyLoader<hydra::data::Image>());
    hydra::loading::AnimationLoaderFactory::Type::BasePtr DummyAnimationL(new DummyLoader<hydra::data::Animation>());
    hydra::loading::SoundTrackLoaderFactory::Type::BasePtr DummySoundL(new DummyLoader<hydra::data::SoundTrack>());
    DummyManager dummyMan;
    modelFact->reg(dummyMan.DUMMY_TYPE_ID, DummyModelL);
    imageFact->reg(dummyMan.DUMMY_TYPE_ID, DummyImageL);
    animFact->reg(dummyMan.DUMMY_TYPE_ID, DummyAnimationL);
    soundFact->reg(dummyMan.DUMMY_TYPE_ID, DummySoundL);


    //more loaders or even more factories here...

}

void hydra::loading::dropFactories(){
    hydra::loading::ImageLoaderFactory::instance()->drop();
    hydra::loading::ModelLoaderFactory::instance()->drop();
    hydra::loading::AnimationLoaderFactory::instance()->drop();
    hydra::loading::ImageLoaderFactory::drop();
    hydra::loading::ModelLoaderFactory::drop();
    hydra::loading::AnimationLoaderFactory::drop();
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
