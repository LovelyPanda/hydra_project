//MeshOptimizer.cpp

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

#include "rendering/MeshOptimizer.hpp"
#include "data/Mesh.hpp"

#include "tri_stripper.h"

#include <boost/foreach.hpp>
#include <cassert>

using hydra::data::Mesh;
using hydra::data::MeshPtr;
using hydra::rendering::MeshOptimizer;

using triangle_stripper::tri_stripper;
using triangle_stripper::primitive_vector;
using triangle_stripper::primitive_group;

struct MeshOptimizer::MeshOptImpl{
    unsigned int mCacheSize;
};

MeshOptimizer::MeshOptimizer(): mImpl(new MeshOptimizer::MeshOptImpl()){

}

MeshOptimizer::MeshOptimizer(const MeshOptimizer& inMeshOpt): mImpl(new MeshOptimizer::MeshOptImpl(*(inMeshOpt.mImpl))){

}

MeshOptimizer::~MeshOptimizer(){

}

MeshOptimizer& MeshOptimizer::operator=(const MeshOptimizer& rhv){
    if(this == &rhv) return *this;
    *mImpl = *(rhv.mImpl);
    return *this;
}

void MeshOptimizer::setCacheSize(unsigned int inSize){
    mImpl->mCacheSize = inSize;
}


void MeshOptimizer::optimizeMesh(MeshPtr inMesh) const{
    //do nothing if given data is crappy
    if(!inMesh || inMesh->mMode != Mesh::TRIANGLES || inMesh->getIndexNum() < 3 || inMesh->getIndexNum()%3 != 0) return;

    //create stripper
    tri_stripper stripper(inMesh->mIndices);
    
    stripper.SetCacheSize(mImpl->mCacheSize);
    //stripper.SetPushCacheHits(true);

    primitive_vector* optimized = new primitive_vector();
    
    //optimize
    stripper.Strip(optimized);

    //we got several submeshes here in primitive_vector;
    //some of them must be triangle strips but some og them may be triangle lists;

    inMesh->mIndices.clear();
    Mesh::IndexCont& indices = inMesh->mIndices;

    //value to store size of last linked strip
    size_t lastStripSize = 0;

    //we should link all strips into 1 tristrip using degenerate triangles
    BOOST_FOREACH(const primitive_group& nextSubMesh, *optimized){
        if(nextSubMesh.Type == triangle_stripper::TRIANGLE_STRIP){
            if(indices.size() == 0){ //just add first one
                indices.insert(indices.end(), nextSubMesh.Indices.begin(), nextSubMesh.Indices.end());
                lastStripSize = nextSubMesh.Indices.size();
            }
            else{
                //add degenerate triangles
                indices.push_back(indices.back());
                //we should add one more if size was odd to save face-order
                if(lastStripSize % 2 != 0) indices.push_back(indices.back());

                indices.push_back(nextSubMesh.Indices.front());
                
                //insert next strip
                indices.insert(indices.end(), nextSubMesh.Indices.begin(), nextSubMesh.Indices.end());

                lastStripSize = nextSubMesh.Indices.size();
            }
        }
        else{ //triangle list found
            //add all triangles to strip 
            //(produce lots of degenerate triangles)
            const size_t nextSubMeshSize = nextSubMesh.Indices.size();
            assert(nextSubMeshSize >= 3 && nextSubMeshSize % 3 == 0);

            if(indices.size() == 0){
                indices.push_back(nextSubMesh.Indices[0]);
                indices.push_back(nextSubMesh.Indices[1]);
                indices.push_back(nextSubMesh.Indices[2]);
            }
            else{
                indices.push_back(indices.back());
                if(lastStripSize % 2 != 0) indices.push_back(indices.back());

                indices.push_back(nextSubMesh.Indices[0]);
                indices.push_back(nextSubMesh.Indices[0]);
                indices.push_back(nextSubMesh.Indices[1]);
                indices.push_back(nextSubMesh.Indices[2]);

                lastStripSize = 3;
            }
            //THIS CODE MAY CONTAIN BUGS IN FACE-ORDER!!!

            for(size_t i = 1; 3*i < nextSubMeshSize; ++i){
                indices.push_back(indices.back());

                indices.push_back(nextSubMesh.Indices[3*i + 0]);
                indices.push_back(nextSubMesh.Indices[3*i + 0]);
                if(i % 2 == 0){
                    indices.push_back(nextSubMesh.Indices[3*i + 1]);
                    indices.push_back(nextSubMesh.Indices[3*i + 2]);
                }
                else{
                    indices.push_back(nextSubMesh.Indices[3*i + 2]);
                    indices.push_back(nextSubMesh.Indices[3*i + 1]);
                }
            }
        }
    }
    //now we should add triangles lists


    inMesh->mMode = Mesh::TRIANGLE_STRIP;

    delete optimized;
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
