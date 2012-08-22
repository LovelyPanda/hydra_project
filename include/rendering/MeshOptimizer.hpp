//MeshOptimizer.hpp

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


#ifndef MESH_OPTIMIZER_HPP__
#define MESH_OPTIMIZER_HPP__

/**
 * \class hydra::rendering::MeshOptimizer
 * \brief Class for representing an entity which optimizes mesh for video-card usage
 *
 * MeshOptimizer optimizes mesh which contains irredundant data (there are
 * no equal vertices in vertex container). It does not change vertices but
 * changes indices. For now optimizer convert mesh index from triangle list to triangle strip.
 * So you should check mesh's mode (Mesh::mMode) after optimization and before rendering.
 * Video-card's vertex cache usage is also optimized.
 *
 * \warning Do not try to optimize meshes which contain redundant data as you may
 * get poor results. You can call Mesh::deleteRedundantVertices() to drop redundancy.
 * If mesh was loaded using some Loaders (loading::Loader) you don't need to
 * do that (loaders have already made that work for you).
 *
 * \author A.V.Medvedev
 * \date 4.07.2010
 */

#include "common/SharedPtr.hpp"
#include "common/PimplPtr.hpp"

namespace hydra{

namespace data{

class Mesh;
typedef common::SharedPtr<Mesh>::Type MeshPtr;

}

namespace rendering{

class MeshOptimizer{

public:
    ///builds empty object
    MeshOptimizer();

    ///copy-constructs same object
    explicit MeshOptimizer(const MeshOptimizer& inMeshOpt);

    ///destructor
    virtual ~MeshOptimizer();

    ///assign operator
    MeshOptimizer& operator=(const MeshOptimizer& inMeshOpt);

    ///function optimizes given mesh for video-card usage
    ///your mesh will be modified
    void optimizeMesh(hydra::data::MeshPtr inMesh) const;

    ///sets size of vcache to optimize for
    void setCacheSize(unsigned int inSize);

private:
    ///implementation
    struct MeshOptImpl;
    ///pimpl idiom
    common::PimplPtr<MeshOptImpl>::Type mImpl;

};

} //rendering namespace

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
