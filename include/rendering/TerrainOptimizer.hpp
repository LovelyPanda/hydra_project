//TerrainOptimizer.hpp

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


#ifndef TERRAIN_OPTIMIZER_HPP__
#define TERRAIN_OPTIMIZER_HPP__

/**
 * \class hydra::rendering::TerrainOptimizer;
 * \brief Class for optimizing terrain surface.
 *
 * It is capable for optimizing the landscape with specified
 * maximum error value.
 * TerrainOptimizer may be used for generating levels of details
 * for landscape.
 * Current implementation uses quad tree to optimize landscape.
 * Triangulation generates triangle list wich may be optimized using
 * hydra::rendering::MeshOptimizer.
 *
 * \author A.V.Medvedev
 * \date 18.08.2010
 */

#include <vector>
#include "common/PimplPtr.hpp"
#include "common/SharedPtr.hpp"
#include <boost/noncopyable.hpp>
#include "data/HeightMap.hpp"

namespace hydra{

namespace data{

struct Vertex;

}

namespace rendering{

class TerrainOptimizer: public boost::noncopyable{

public:
    ///builds optimizer from height map data
    TerrainOptimizer(const hydra::data::HeightMap& inHeights);

    ~TerrainOptimizer();

    ///Rebuild optimizer to work with new height map data
    ///If new height map's size is same, tree will not be recreated
    ///which is very good optimization
    void rebuild(const hydra::data::HeightMap& inHeights);

    ///generates LOD with specified max error [in object space; height].
    ///returns nothing.
    ///This must be called before building triangulation.
    ///After generating LOD you may use same data to build several triangulations.
    void generateLOD(float inMaxError);

    ///Builds triangulation and returns it (for whole chunk).
    ///You may generate vertices using generateVertices utility function.
    ///\warning You must generateLOD before building triangulation.
    std::vector<unsigned int> getTriangulation() const;

    ///builds triangulation and returns it (for specified chunk).
    ///You may generate vertices using generateVertices utility function.
    ///Stack is encoded using QuadTree::NodePos's.
    ///\warning You must generateLOD before building triangulation.
    std::vector<unsigned int> getTriangulation(const std::vector<int>& inPositionStack) const;

    ///utility function to generate vertices from heightmap data
    static std::vector<hydra::data::Vertex> generateVertices(const hydra::data::HeightMap& inHeights, float inVertexStride = 1.0f);

private:
    //pimpl
    struct Impl;
    hydra::common::PimplPtr<TerrainOptimizer::Impl>::Type mImpl;
};

///pointer to terrain optimizer
typedef hydra::common::SharedPtr<TerrainOptimizer>::Type TerrainOptimizerPtr;

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
