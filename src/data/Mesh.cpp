//Mesh.cpp

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

#include "data/Mesh.hpp"
#include "data/Material.hpp"
#include "data/Vertex.hpp"
#include "common/SharedPtr.hpp"
#include "math/Vector3D.hpp"
#include "math/Point.hpp"
#include "math/AABB.hpp"

#include <vector>
#include <map>
#include <algorithm>
#include <boost/foreach.hpp>

using hydra::data::Mesh;
using hydra::data::Vertex;
using hydra::math::Vector3D;
using hydra::math::AABB;
using hydra::math::Point;

void Mesh::addIndex(const Mesh::index_t inIndex){
    mIndices.push_back(inIndex);
}

void Mesh::addVertex(const Vertex& inVertex){
    mVertices.push_back(inVertex);
}

void Mesh::deleteRedundantVertices(){
    //if empty do nothing
    if(!(getVertexNum() && getIndexNum())) return;

    typedef std::vector<Mesh::index_t*> IndicesList;
    typedef std::map<Vertex, IndicesList> VertexMap;
    VertexMap vertexMap;

    //fill the map
    //same vertices will be merged; unused will be droped
    for(size_t i = 0; i < mIndices.size(); ++i){
        vertexMap[mVertices[mIndices[i]]].push_back(&(mIndices[i]));
    }

    mVertices.clear();
    mVertices.reserve(vertexMap.size());

    typedef VertexMap::value_type vertexMapPair;
    BOOST_FOREACH(const vertexMapPair& nextPair, vertexMap){
        mVertices.push_back(nextPair.first);
        BOOST_FOREACH(index_t* nextIndexPtr, nextPair.second)
            *nextIndexPtr = mVertices.size() - 1;
    }
}

void Mesh::generateNormals(){
    if(mMode != Mesh::TRIANGLE_STRIP && mMode != Mesh::TRIANGLE_LIST) return;

    //set all normals to zero
    for(size_t i = 0; i < mVertices.size(); ++i)
        mVertices[i].mNormal = Vector3D(0.0f, 0.0f, 0.0f);

    //build normals from triangles
    size_t numTriangles;
    if(mMode == Mesh::TRIANGLE_LIST) numTriangles = mIndices.size() / 3;
    else if(mMode == Mesh::TRIANGLE_STRIP) numTriangles = mIndices.size() - 2;
    else return;

    for(size_t i = 0; i < numTriangles; ++i){

        Vertex* vert1;
        Vertex* vert2;
        Vertex* vert3;

        if(mMode == Mesh::TRIANGLE_LIST){
            vert1 = &mVertices[mIndices[3*i]];
            vert2 = &mVertices[mIndices[3*i + 1]];
            vert3 = &mVertices[mIndices[3*i + 2]];
        }
        else if(mMode == Mesh::TRIANGLE_STRIP){
            vert1 = &mVertices[mIndices[i + 0]];
            vert2 = &mVertices[mIndices[i + 1]];
            vert3 = &mVertices[mIndices[i + 2]];

            //if the triangle num id even we should swap orientation
            if(i % 2 != 0){
                Vertex* temp = vert1;
                vert1 = vert3;
                vert3 = temp;
            }
        }
        else return;

        Vector3D vec1(vert2->mCoord.x - vert1->mCoord.x, vert2->mCoord.y - vert1->mCoord.y, vert2->mCoord.z - vert1->mCoord.z);
        Vector3D vec2(vert3->mCoord.x - vert1->mCoord.x, vert3->mCoord.y - vert1->mCoord.y, vert3->mCoord.z - vert1->mCoord.z);

        //find cross product
        Vector3D contribution = vec1.cross(vec2);
        
        vert1->mNormal += contribution;
        vert2->mNormal += contribution;
        vert3->mNormal += contribution;
    }

    //normalization
    for(size_t i = 0; i < mVertices.size(); ++i)
        mVertices[i].mNormal.normalize();
}

AABB Mesh::calcAABB() const{
    
    //if empty return empty aabb
    if(!(getVertexNum() && getIndexNum())) return AABB();

    //traverse all vertices and find minimums and maximums
    float minX, maxX, minY, maxY, minZ, maxZ;

    //set initial values
    const Point& firstVertexCoord = mVertices[mIndices[0]].mCoord;

    maxX = minX = firstVertexCoord.x;
    maxY = minY = firstVertexCoord.y;
    maxZ = minZ = firstVertexCoord.z;

    //traverse all vertices and find maximums/minimums
    BOOST_FOREACH(Mesh::index_t index, mIndices){
        const Point& nextCoord = mVertices[index].mCoord;

        maxX = std::max(maxX, nextCoord.x);
        maxY = std::max(maxY, nextCoord.y);
        maxZ = std::max(maxZ, nextCoord.z);

        minX = std::min(minX, nextCoord.x);
        minY = std::min(minY, nextCoord.y);
        minZ = std::min(minZ, nextCoord.z);
    }

    return AABB(Vector3D(minX, minY, minZ), Vector3D(maxX - minX, maxY - minY, maxZ - minZ));
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
