//Mesh.hpp

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


#ifndef MESH_HPP__
#define MESH_HPP__

/**
 * \class hydra::data::Mesh
 * \brief Class represents generic mesh. Simple container for polygons.
 *
 * Mesh contains vertices and indices to link vertices. It also contains
 * primitive assemble mode to understand how indices are used. 
 * It may contain pointer to material object.
 * It also may contain data which is need to create skeleton-based animation:
 * indices of bones (joints) for each vertex and respective weights.
 *
 * \author A.V.Medvedev
 * \date 10.06.2010
 */

#include <vector>
#include <boost/array.hpp>

#include "data/Vertex.hpp"
#include "common/SharedPtr.hpp"
#include "math/Vector3D.hpp"
#include "math/AABB.hpp"

namespace hydra{

namespace data{

class Material;
typedef common::SharedPtr<Material>::Type MaterialPtr;

class Mesh{

public:

    ///enumeration which specifies assemble mode of primitives
    ///which are kept in vertex arrays
    enum PrimitiveAssembleMode{
        NONE = 0,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
        TRIANGLE_LIST,
        TRIANGLES = TRIANGLE_LIST,
        QUADS,
        QUAD_STRIP
    };

    ///\brief Maximum number of bones per vertex.
    ///
    ///For software implementation of animation you
    ///may set high value, but for shader implementation
    ///number of bones per vertex is limited.
    static const int MAX_BONES_PER_VERTEX = 9;

    ///type of index (basic type) needed to build polygons
    typedef unsigned int index_t;

    ///\brief Face container type.
    ///
    ///Faces (polygons) contain vertex indices
    typedef std::vector<index_t> IndexCont;
   
    ///container for vertices (type)
    typedef std::vector<data::Vertex> VertexCont;

    ///linear container for indices of a single vertex's bones
    typedef boost::array<int, MAX_BONES_PER_VERTEX> BoneIndices;

    ///container for bone' indices
    typedef std::vector<BoneIndices> BoneCont;

    ///container for weights of a single vertex's bones
    typedef boost::array<float, MAX_BONES_PER_VERTEX> BoneWeights;

    ///container for weights
    typedef std::vector<BoneWeights> BoneWeightCont;

    ///adds new Face to mesh. It is guarantied that last added face 
    ///is added to the end of container (becomes last one).
    void addIndex(const index_t inIndex);

    ///adds new vertex coordinate to mesh. It is guarantied that last added vertex
    /// is added to the end of container (becomes last one).
    void addVertex(const data::Vertex& inVertex);

    ///returns number of vertices in mesh
    inline size_t getVertexNum() const{
        return mVertices.size();
    }

    ///returns number of faces in mesh
    inline size_t getIndexNum() const{
        return mIndices.size();
    }

    ///\brief Finds and deletes redundant vertices of mesh.
    ///
    /// Changes number of vertices and also changes some values of indices.
    /// \warning Use this function only for static mesh! 
    /// Do not use this function with mesh which contains animation data 
    /// (bones and weights) as it will be corrupted!
    void deleteRedundantVertices();

    ///generates normals from mesh data
    void generateNormals();

    ///generates axis aligned bounding box for mesh
    hydra::math::AABB calcAABB() const;

    ///assemble mode of primitives of this mesh
    PrimitiveAssembleMode mMode;

    ///all the indices to build polygons of mesh
    IndexCont mIndices;

    ///all the vertices of mesh
    VertexCont mVertices;

    ///material of this mesh
    hydra::data::MaterialPtr mMaterial;

    ///Bone indices
    BoneCont mBones;

    ///Bone weights
    BoneWeightCont mBoneWeights;
};

///pointer (smart) to Mesh object
typedef common::SharedPtr<hydra::data::Mesh>::Type MeshPtr;

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
