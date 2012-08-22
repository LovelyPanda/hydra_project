//TerrainPreprocessor.hpp

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


#ifndef TERRAIN_PREPROCESSOR_HPP__
#define TERRAIN_PREPROCESSOR_HPP__

/**
 * \class hydra::rendering::TerrainPreprocessor
 * \brief Class for preprocessing terrain data.
 *
 * Preprocessor creates levels of details for specified terrain data.
 * It may be used as offline tool. The result data must be saved somewhere
 * for future use in this case.
 *
 * \see hydra::data::TerrainFragment
 * \see hydra::data::TerrainOptimizer
 *
 * \author A.V.Medvedev
 * \date 05.09.2010
 */

#include <boost/noncopyable.hpp>
#include "common/SharedPtr.hpp"
#include "common/PimplPtr.hpp"
#include "data/HeightMap.hpp"
#include "data/TerrainFragment.hpp"

namespace hydra{

namespace rendering{

class TerrainPreprocessor: private boost::noncopyable{

public:
    /**
     * \brief Properties for Terrain preprocessor
     *
     * User may configure preprocessor for his needs.
     */
    struct Properties{
        ///Number of levels of details.
        ///Preprocessor will try to build lods. If it is
        ///not possible for given maxError and errorFactor values
        ///there will be less levels in result fragment.
        unsigned char numOfLODs;

        ///Maximum error value for LOD.
        ///0 level will have maxError value.
        ///No LOD will have error value more than this one.
        float maxError;

        ///For each next level of detail current error value
        ///will be multiplied by this factor.
        ///So, minimum error value will be 
        ///minError = maxError * LODErrorFactor ^ (numOfLODs - 1)
        float LODErrorFactor;

        ///Preprocessor will create new level of detail for vertices 
        ///if and only if its number of vertices would differ at least
        ///in vertexLODFactor times. So if n-th level of detail 
        ///(vertex lod) consist of x different vertices, (n+1)-th level
        ///would have more than x / vertexLODFactor vertices and 
        ///(n-1)-th level would have less than x * vertexLODFactor
        ///vertices.
        ///So, vertexLODFactor's value should be in interval (0, 1.0)
        float vertexLODFactor;

        ///Preprocessor may generate skirts for each chunk.
        ///Skirt is a simple set of triangles along the chunk's border.
        ///It is intended to cover the cracks between chunks with different
        ///levels during rendering. See 'Chunked LOD' for more info.
        bool generateSkirts;
    };

    ///creates optimizer with specified properties
    TerrainPreprocessor(TerrainPreprocessor::Properties inProps);

    ///destroys optimizer
    ~TerrainPreprocessor();

    ///processes heightmap and generates TerrainFragment.
    ///May throw std::runtime_error.
    hydra::data::TerrainFragmentPtr process(const hydra::data::HeightMap& inHeightMap);

    ///returns properties
    TerrainPreprocessor::Properties getProperties() const;

private:
    //pimpl
    struct Impl;
    hydra::common::PimplPtr<Impl>::Type mImpl;

};

///pointer to TerrainPreprocessor
typedef hydra::common::SharedPtr<TerrainPreprocessor>::Type TerrainPreprocessorPtr;

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
