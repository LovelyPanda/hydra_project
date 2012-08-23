//for_doxygen.hpp

/*
 *   Copyright 2010 Alexander Medvedev
 *
 *   This file is part of Hydra project.
 *   See <http://hydraproject.org.ua> for more info.
 *
 *   Hydra is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Hydra is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with Hydra.  If not, see <http://www.gnu.org/licenses/>.
 */

//this is not a source file
//it is used for doxygen docs generation
//You can put documentation parts here if you don't know where to place them.

/**
 * \mainpage This is a hydra project documentation.
 *
 * The project aims to be an Open Source C++ toolkit for game developers.
 * It is not some kind of game engine or strict framework where you are
 * limited by architecture of library you use and/or must inherit some huge classes
 * to get simple functionality. 
 * Instead Hydra is a highly-scallable, easy to use and common set of 
 * libraries for game developers. You can use only those of Hydra libraries (or classes)
 * you need (we aim to have low cohesion between libraries and classes - hydra can live
 * without some of its heads!).
 *
 * Now Hydra has 7 parts. Each has its own namespace and some of
 * them are built to separate shared/static lib. All of the namespaces are inside hydra namespace.
 * This parts are:
 *  - hydra::common (Just a common classes which can be used anywhere, like smart pointers);
 *  - hydra::math (There is no need to explain it);
 *  - hydra::loading (Library for loading different data from files or other sources);
 *  - hydra::data (Library for keeping data in apropriate generic format without dependency from
 *  the source of data);
 *  - hydra::rendering (Library contains different classes and functions which are related to video image rendering);
 *  - hydra::system (Library for wrapping platform-dependent routines like window creation and input handling);
 *  - hydra::sound (Library contains classes and functions which are related to sound rendering);
 *
 * Some of main goals of Hydra project are:
 *  - High scalability and extendability;
 *  - Low cohesion between classes and packages;
 *  - Interface simplicity;
 *  - High code quality;
 *  - Good documentation;
 *  - Full advantage of object-oriented and generic programming using C++;
 *  - Cross-platform (different operating systems and different API's);
 *  - Free Software which you can use in your commercial projects;
 *
 * These goals are not reached so far but we are working in that direction.
 *
 * See <a href="http://hydraproject.org.ua">http://hydraproject.org.ua</a> for more info.
 * 
 * This documentation has been generated from sources with use of Doxygen.
 *
 */

/**
 * \namespace hydra::common
 * \brief Contains common classes, functions and templates
 * which may be used by any other parts.
 *
 * Namespace contains different generic classes like shared smart-pointer (SharedPtr) or 
 * implementation of factory pattern (Factory).
 *
 * Now this part of Hydra project is fully in headers.
 * So no library is built. However this can be changed in future
 * and we would have one more library in that case.
 *
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

/**
 * \namespace hydra::math
 * \brief Contains classes and functions for representing mathematical entities
 * and for work with them which can be used for game developers.
 *
 * Members of this namespace are build as dynamic library (math). The most common 
 * representatives of this namespace are vectors (Vector3D), quternions (Quat).
 *
 *
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

/**
 * \namespace hydra::rendering
 * \brief Contains classes and functions which are directly or indirectly
 * connected with rendering routines.
 *
 * \todo Add new functionality.
 *
 * \author A.V.Medvedev
 * \date 26.04.2010
 */

/**
 * \namespace hydra::data
 * \brief Contains classes which represent different entities of data storing
 * and aggregation.
 *
 * This namespace contains such classes as Image, Model, Material, etc. 
 * Classes of this namespace contains different kinds of data and represent 
 * different entites which are containers and aggregates of data.
 * However those classes are not common-purpose templates as such templates should lie
 * in common namespace.
 *
 * \author A.V.Medvedev
 * \date 06.06.2010
 */

/**
 * \namespace hydra::system
 * \brief Contains classes which represent some system-dependent entities
 * like windows or input systems.
 *
 * This namespace contains such classes as Window, Input.
 *
 * \author A.V.Medvedev
 * \date 01.08.2010
 */

/**
 * \namespace hydra::sound
 * \brief Contains classes which represent entities dealing with
 * sound rendering.
 *
 * This namespace contains such classes as Sound, etc.
 *
 * \author A.V.Medvedev
 * \date 01.08.2010
 */


/**
 * \file Loader.hpp
 */

/**
 * \file Manager.hpp
 */

/**
 * \file ILImageLoader.hpp
 */

/**
 * \file Camera.hpp
 */

/**
 * \file Color.hpp
 */

/**
 * \file FileManager.hpp
 */

/**
 * \file Image.hpp
 */

/**
 * \file Material.hpp
 */

/**
 * \file Model.hpp
 */

/**
 * \file OBJModelLoader.hpp
 */

/**
 * \file Vector3D.hpp
 */

/**
 * \file Quat.hpp
 */

/**
 * \file Vertex.hpp
 */

/**
 * \file Point.hpp
 */

/**
 * \file Factory.hpp
 */

/**
 * \file Singleton.hpp
 */

/**
 * \file SharedPtr.hpp
 */

/**
 * \file Cache.hpp
 */

/**
 * \file CachePool.hpp
 */

/**
 * \file Mesh.hpp
 */

/**
 * \file MeshOptimizer.hpp
 */

/**
 * \file PimplPtr.hpp
 */

/**
 * \file MD5ModelLoader.hpp
 */

/**
 * \file MD5AnimationLoader.hpp
 */

/**
 * \file DummyManager.hpp
 */

/**
 * \file DummyLoader.hpp
 */

/**
 * \file Animation.hpp
 */

/**
 * \file Bone.hpp
 */

/**
 * \file Skeleton.hpp
 */

/**
 * \file Matrix.hpp
 */

/**
 * \file CacheableTypes.hpp
 */

/**
 * \file Timer.hpp
 */

/**
 * \file PerformanceTimer.hpp
 */

/**
 * \file Renderer.hpp
 */

/**
 * \file OpenGLRenderer.hpp
 */

/**
 * \file Window.hpp
 */

/**
 * \file SDLWindow.hpp
 */

/**
 * \file Input.hpp
 */

/**
 * \file SDLInput.hpp
 */

/**
 * \file Win32Window.hpp
 */

/**
 * \file Win32Input.hpp
 */

/**
 * \file Sound.hpp
 */

/**
 * \file OpenALSound.hpp
 */

/**
 * \file Event.hpp
 */

/**
 * \file Application.hpp
 */

/**
 * \file SoundTrack.hpp
 */

/**
 * \file OGGSoundTrackLoader.hpp
 */

/**
 * \file HeightMap.hpp
 */

/**
 * \file TerrainOptimizer.hpp
 */

/**
 * \file TerrainLODManager.hpp
 */

/**
 * \file QuadTree.hpp
 */

/**
 * \file ZipFileManager.hpp
 */

/**
 * \file TerrainPreprocessor.hpp
 */

/**
 * \file TerrainChunk.hpp
 */

/**
 * \file TerrainChunkId.hpp
 */

/**
 * \file TerrainFragment.hpp
 */

/**
 * \file TerrainFragmentId.hpp
 */

/**
 * \file LoadStatus.hpp
 */

/**
 * \file ChunkedTerrain.hpp
 */

/**
 * \file AABB.hpp
 */

/**
 * \file CGContext.hpp
 */

/**
 * \file CGParameter.hpp
 */

/**
 * \file CGProgram.hpp
 */

/**
 * \file CGProfile.hpp
 */

/**
 * \file CGError.hpp
 */

