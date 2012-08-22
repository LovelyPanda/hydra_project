//TerrainPreprocessor.cpp

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

#include "rendering/TerrainPreprocessor.hpp"
#include "rendering/TerrainOptimizer.hpp"
#include "common/QuadTree.hpp"
#include "common/LoadStatus.hpp"
#include "data/TerrainChunk.hpp"
#include "data/TerrainFragment.hpp"
#include "data/HeightMap.hpp"
#include "math/AABB.hpp"
#include "data/Vertex.hpp"
#include "data/Mesh.hpp"
#include "math/Vector3D.hpp"

#include <cassert>
#include <cmath>

#include <algorithm>
#include <limits>
#include <boost/foreach.hpp>

using hydra::rendering::TerrainPreprocessor;
using hydra::rendering::TerrainOptimizer;
using hydra::rendering::TerrainOptimizerPtr;
using hydra::data::TerrainFragment;
using hydra::data::TerrainFragmentPtr;
using hydra::common::QuadTree;
using hydra::data::TerrainChunk;
using hydra::data::TerrainChunkPtr;
using hydra::common::Conditional;
using hydra::data::HeightMap;
using hydra::data::Vertex;
using hydra::data::Mesh;
using hydra::math::AABB;
using hydra::math::Vector3D;

typedef std::vector<unsigned int> LongIndexCont;

struct ExtraChunkData{
    TerrainFragment::ChunkData chunkData;
    LongIndexCont longIndices;
};

typedef hydra::common::QuadTree<ExtraChunkData> QTree;
typedef hydra::common::SharedPtr<QTree>::Type QTreePtr;

//functor to build triangulation for nodes in quadtree
struct TriangulationBuilder: public QTree::NodeFunctor{
    
    virtual void operator()(QTree& tree, QTree::Node& node){
        assert(optimizer);

        TerrainChunkPtr newChunk(new TerrainChunk());
        std::vector<int> stack;

        //if not a root
        if(&node != &tree.getRoot()){
            //now we should find out for what chunk we are generating
            //triangulation. Having a stack of positions we can generate
            //proper triangulation.
            unsigned short parentNodeId = node.parent;
            unsigned short previousNodeId = 0;
            for(size_t i = 0; i < 4; ++i){
                //if(node.neighbours[i] == 0) continue;
                ////get neightbour's neightbour's id (this)
                //previousNodeId = tree.getNode(node.neighbours[i]).neighbours[QTree::getOpposite(static_cast<QTree::NeighbourPos>(i))];
                if(&node == &tree.getNode(tree.getNode(parentNodeId).nodes[i])){                
                    previousNodeId = tree.getNode(parentNodeId).nodes[i];
                    break;
                }
            }
            assert(previousNodeId != 0);
            assert((node.neighbours[0] == tree.getNode(previousNodeId).neighbours[0]) &&
                   (node.neighbours[1] == tree.getNode(previousNodeId).neighbours[1]) &&
                   (node.neighbours[2] == tree.getNode(previousNodeId).neighbours[2]) &&
                   (node.neighbours[3] == tree.getNode(previousNodeId).neighbours[3]));

            while(previousNodeId != 0){
                for(int i = 0; i < 4; ++i)
                    if(previousNodeId == tree.getNode(parentNodeId).nodes[i]){
                        stack.push_back(i);
                        break;
                    }
                previousNodeId = parentNodeId;
                parentNodeId = tree.getNode(parentNodeId).parent;
            }
            assert(!stack.empty());

            std::reverse(stack.begin(), stack.end());

            node.data.longIndices = optimizer->getTriangulation(stack);
        }
        else{
            node.data.longIndices = optimizer->getTriangulation();
        }

        node.data.chunkData.level = stack.size();
        node.data.chunkData.ptr = newChunk;
        node.data.chunkData.status = TerrainFragment::RAM;

        //check whether we got empty triangulation
        if(node.data.longIndices.size() == 0){
            //we should mark level as not present
            levelIsNotPresent = true;
        }
    }

    bool levelIsNotPresent;
    TerrainOptimizerPtr optimizer;
};

//functor for generating a skirt triangulation
struct SkirtTriangulationBuilder: public QTree::NodeFunctor{

    virtual void operator()(QTree&, QTree::Node& node){
        if(node.data.longIndices.empty()) return;

        //we traverse all the indices and mark enabled border vertices
        std::vector<bool> borderVertexFlags;
        borderVertexFlags.resize(4 * chunkSize - 4, false);

        //find bottom left corner
        unsigned int bottomLeftCorner = *std::min_element(node.data.longIndices.begin(), node.data.longIndices.end());
        
        
        unsigned int topLeftCorner = bottomLeftCorner + (chunkSize - 1) * fragmentSize;
        unsigned int leftSideFirstOne = bottomLeftCorner + fragmentSize; //don't ask me wtf is it...

        //traverse all indices
        for(unsigned int i = 0; i < node.data.longIndices.size(); ++i){
            //find indices at the border of chunk
            if(((node.data.longIndices[i] % fragmentSize) % (chunkSize - 1)) == 0 ||
               ((node.data.longIndices[i] / fragmentSize) % (chunkSize - 1)) == 0){
                
                //mark
                //if same line as bottom left corner
                
                if(bottomLeftCorner + chunkSize > node.data.longIndices[i]){
                    borderVertexFlags[node.data.longIndices[i] - bottomLeftCorner] = true;
                }
                
                //same line as top left corner
                else if(topLeftCorner <= node.data.longIndices[i]){
                    borderVertexFlags[borderVertexFlags.size() - chunkSize + node.data.longIndices[i] - topLeftCorner] = true;
                }
                //sides
                else{
                    unsigned int delta = node.data.longIndices[i] - leftSideFirstOne;
                    if(delta % fragmentSize == 0){
                        //left
                        borderVertexFlags[chunkSize + 2 * delta / fragmentSize] = true;
                    }
                    else{
                        //right
                        borderVertexFlags[chunkSize + 1 + 2 * (delta + 1 - chunkSize) / fragmentSize] = true;
                    }
                }
            }
        }

        //now we have marked vertices
        //we can generate triangulation for our skirts
        
        //first generate clean indices
        //and also toss them in counter clockwise order
        unsigned int numOfEnabled = std::count(borderVertexFlags.begin(), borderVertexFlags.end(), true);
        std::vector<unsigned int> enabledVertices;
        enabledVertices.reserve(numOfEnabled);

        //traverse first line
        for(unsigned int i = 0; i < chunkSize; ++i)
            if(borderVertexFlags[i])
                enabledVertices.push_back(bottomLeftCorner + i);
    
        //right side
        for(unsigned int i = chunkSize + 1; i < (borderVertexFlags.size() - chunkSize); i += 2){
            if(borderVertexFlags[i])
                enabledVertices.push_back(leftSideFirstOne + chunkSize - 1 + fragmentSize * (i - chunkSize - 1) / 2);
        }

        //top line
        for(unsigned int i = borderVertexFlags.size() - 1; i >= borderVertexFlags.size() - chunkSize; --i){
            if(borderVertexFlags[i])
                enabledVertices.push_back(topLeftCorner + (i - (borderVertexFlags.size() - chunkSize)));
        }

        //left side
        for(unsigned int i = borderVertexFlags.size() - chunkSize - 2; i >= chunkSize; i -= 2){
            if(borderVertexFlags[i])
                enabledVertices.push_back(leftSideFirstOne + fragmentSize * (i - chunkSize) / 2);
        }
        assert(enabledVertices.size() == numOfEnabled);


        //now we have clean indices in right order
        //just traverse chunk's border and triangulate
        for(unsigned int i = 0, previousIndex = enabledVertices.back(); 
                i < enabledVertices.size(); 
                previousIndex = enabledVertices[i],++i){
            node.data.longIndices.push_back(previousIndex);
            node.data.longIndices.push_back(previousIndex + fragmentSize * fragmentSize);
            node.data.longIndices.push_back(enabledVertices[i]);

            node.data.longIndices.push_back(previousIndex + fragmentSize * fragmentSize);
            node.data.longIndices.push_back(enabledVertices[i] + fragmentSize * fragmentSize);
            node.data.longIndices.push_back(enabledVertices[i]);
        }
    }

    unsigned int chunkSize;
    unsigned int fragmentSize;
};

//functor for marking vertices as enabled in special set
struct VertexMarker: public QTree::NodeFunctor{
    VertexMarker(): number(0){

    }

    virtual void operator()(QTree&, QTree::Node& node){
        LongIndexCont& indices = node.data.longIndices;
        for(unsigned int index = 0; index < indices.size(); ++index){
            if(!bitmap[indices[index]]){
                ++number;
                bitmap[indices[index]] = true;
            }
        }
    }

    unsigned int number;
    std::vector<bool> bitmap;
};

//functor to transform indices using map
struct IndexTransformer: public QTree::NodeFunctor{
    IndexTransformer(const std::vector<unsigned int>& inTransformationMap): mTransformationMap(inTransformationMap){

    }

    virtual void operator()(QTree&, QTree::Node& node){
        assert(!mTransformationMap.empty());
        for(size_t i = 0; i < node.data.longIndices.size(); ++i){
            node.data.longIndices[i] = mTransformationMap[node.data.longIndices[i]];
        }
    }

    const std::vector<unsigned int>& mTransformationMap;
};

//functor to set vertex level value to all nodes of level
struct VertexLevelAndErrorSetter: public QTree::NodeFunctor{
    virtual void operator()(QTree&, QTree::Node& node){
       node.data.chunkData.vertices = level;
       //node.data.chunkData.ptr->maxError = maxError;
       node.data.chunkData.maxError = maxError;
    }

    unsigned int level;
    float maxError;
};

//functor to get index's LOD
struct IndexGetter: public QTree::NodeFunctor{
    IndexGetter(Mesh::IndexCont& inIndices): indices(inIndices){

    }

    virtual void operator()(QTree&, QTree::Node& node){
        indices.insert(indices.end(), node.data.longIndices.begin(), node.data.longIndices.end());
    }

    Mesh::IndexCont& indices;
};

//functor to calculate AABB for every chunk
struct AABBCalculator: public TerrainFragment::QuadTreeOfChunks::NodeFunctor{
    virtual void operator()(TerrainFragment::QuadTreeOfChunks&, TerrainFragment::QuadTreeOfChunks::Node& node){
        assert(node.data.ptr);
        //build mesh object
        mesh.mIndices.resize(node.data.ptr->indices.size());
        for(size_t i = 0; i < mesh.mIndices.size(); ++i)
            mesh.mIndices[i] = node.data.ptr->indices[i];
        
        node.data.aabb = mesh.calcAABB();
    }

    Mesh mesh;
    TerrainFragmentPtr fragment;
};

struct TerrainPreprocessor::Impl{
    Impl(TerrainPreprocessor::Properties inProps): properties(inProps){
        
    }

    TerrainPreprocessor::Properties properties;
    TerrainOptimizerPtr optimizer;
};

TerrainPreprocessor::TerrainPreprocessor(TerrainPreprocessor::Properties inProperties): mImpl(new TerrainPreprocessor::Impl(inProperties)){

}

TerrainPreprocessor::~TerrainPreprocessor(){

}


Vector3D calcNormalForTriangle(Vector3D vert1, Vector3D vert2, Vector3D vert3){
    Vector3D vec1(vert2.x() - vert1.x(), vert2.y() - vert1.y(), vert2.z() - vert1.z());
    Vector3D vec2(vert3.x() - vert1.x(), vert3.y() - vert1.y(), vert3.z() - vert1.z());

    //find cross product
    return vec1.cross(vec2);
}

//helper function to generate compressed vertices from heightmap (row-major with lower left origin)
static TerrainFragment::VertexCont generateVertices(const HeightMap& inHeightMap, bool inGenSkirts){
    TerrainFragment::VertexCont vertices;
    if(!inGenSkirts){
        vertices.resize(inHeightMap.getSize() * inHeightMap.getSize());
    }
    else{
        vertices.resize(2 * inHeightMap.getSize() * inHeightMap.getSize());
    }

    for(unsigned int i = 0; i < inHeightMap.getSize(); ++i){ //line
        for(unsigned int j = 0; j < inHeightMap.getSize(); ++j){
            TerrainFragment::CompressedVertex newVertex;
            newVertex.x = static_cast<unsigned short>(i);
            newVertex.y = (inHeightMap[i * inHeightMap.getSize() + j]);
            newVertex.z = static_cast<unsigned short>(j);

            //normal?? no
            //we will generate normals later using Mesh utility functions
            //well, actually, yes!
            //TODO

            //dirty
            //traverse all neighbouring triangles and calculate normal
            Vector3D normal(0.0f, 0.0f, 0.0f);
			//we are assuming that i and j may not be very big
            Vector3D thisVertex(static_cast<float>(i), newVertex.y, static_cast<float>(j));
        
            //first get normal contribution on the left (for index i - 1)
            if(i != 0){

                Vector3D leftVertex(static_cast<float>(i - 1), inHeightMap[(i - 1) * inHeightMap.getSize() + j], static_cast<float>(j));

                //on the bottom (second index j - 1)
                if(j != 0){
                    Vector3D bottomVertex(static_cast<float>(i), inHeightMap[(i) * inHeightMap.getSize() + j - 1], static_cast<float>(j - 1));
                    Vector3D bottomLeftVertex(static_cast<float>(i - 1), inHeightMap[(i - 1) * inHeightMap.getSize() + j - 1], static_cast<float>(j - 1));
                    
                    //2 trangles
                    normal += calcNormalForTriangle(bottomLeftVertex, bottomVertex, thisVertex);
                    normal += calcNormalForTriangle(bottomLeftVertex, thisVertex, leftVertex);
                }

                //on the top
                if(j != (inHeightMap.getSize() - 1)){
                    Vector3D topVertex(static_cast<float>(i), inHeightMap[(i) * inHeightMap.getSize() + j + 1], static_cast<float>(j + 1));
                    
                    //1 trangle
                    normal += calcNormalForTriangle(leftVertex, thisVertex, topVertex);
                }
            }

            //get normal component  on the right
            if(i != (inHeightMap.getSize() - 1)){
                Vector3D rightVertex(static_cast<float>(i + 1), inHeightMap[(i + 1) * inHeightMap.getSize() + j], static_cast<float>(j));

                //on the bottom (second index j + 1)
                if(j != 0){
                    Vector3D bottomVertex(static_cast<float>(i), inHeightMap[(i) * inHeightMap.getSize() + j - 1], static_cast<float>(j - 1));
                    
                    //1 trangle
                    normal += calcNormalForTriangle(bottomVertex, rightVertex, thisVertex);
                }

                //on the top
                if(j != (inHeightMap.getSize() - 1)){
                    Vector3D topVertex(static_cast<float>(i), inHeightMap[(i) * inHeightMap.getSize() + j + 1], static_cast<float>(j + 1));
                    Vector3D topRightVertex(static_cast<float>(i + 1), inHeightMap[(i + 1) * inHeightMap.getSize() + j + 1], static_cast<float>(j + 1));

                    //2 trangles
                    normal += calcNormalForTriangle(rightVertex, topRightVertex, thisVertex);
                    normal += calcNormalForTriangle(topVertex, thisVertex, topRightVertex);
                }
            }

            normal.normalize();

            newVertex.normalX = normal.x();
            newVertex.normalZ = normal.z();

            //insert new vertex into vertex container
            vertices[i * inHeightMap.getSize() + j] = newVertex;
            
            //for skirts we generate vertices twice
            if(inGenSkirts){
                newVertex.y -= 5.0f; //TODO

                vertices[(inHeightMap.getSize() * inHeightMap.getSize()) + (i * inHeightMap.getSize() + j)] = newVertex;
            }
        }
    }



    return vertices;
}

//utility function to copy N levels from one quadtree to another
static void copyQuadTreeLevels(const QTree& source, QTree& dest, unsigned int numLevels, unsigned int sourceNodeId = 0, unsigned int destNodeId = 0){
    assert(numLevels <= source.getResolution());
    assert(numLevels <= dest.getResolution());
    assert(numLevels != 0);
    
    const QTree::Node& sourceNode = source.getNode(sourceNodeId);
    QTree::Node& destNode = dest.getNode(destNodeId);

    //copy data
    destNode.data = sourceNode.data;

    unsigned int numLevelsLeft = numLevels - 1;
    if(numLevelsLeft == 0) return;

    //copy childs
    for(unsigned int i = 0; i < 4; ++i){
        assert(sourceNode.nodes[i] != 0);
        assert(destNode.nodes[i] != 0);

        //recursive call
        copyQuadTreeLevels(source, dest, numLevelsLeft, sourceNode.nodes[i], destNode.nodes[i]);
    }
}

TerrainFragmentPtr TerrainPreprocessor::process(const HeightMap& inHeightMap){
    assert(mImpl);   

    //first time create
    if(!mImpl->optimizer){
        mImpl->optimizer = TerrainOptimizerPtr(new TerrainOptimizer(inHeightMap));
    }
    else{
        mImpl->optimizer->rebuild(inHeightMap);
    }

    //now we should build levels of details
    TerrainFragmentPtr result(new TerrainFragment());

    QTreePtr tree = QTreePtr(new QTree(mImpl->properties.numOfLODs));

    TerrainFragment::VertexCont allVertices = generateVertices(inHeightMap, mImpl->properties.generateSkirts);

    TriangulationBuilder triangBuilder;
    triangBuilder.optimizer = mImpl->optimizer;

    float error = mImpl->properties.maxError;
    unsigned int currentLOD = 0;

    //now we should build all the levels of details for indices
    //
    //some of index levels may be empty because optimizer is not able
    //to build triangulation when specified max error is to large and
    //all the quads of level were disabled
    //we skip such levels
    for(size_t i = 0; i < mImpl->properties.numOfLODs; ++i){
        triangBuilder.levelIsNotPresent = false;

        triangBuilder.optimizer->generateLOD(error);
        applyToLevel(*tree, currentLOD, triangBuilder);

        //we should add skirts (if we need them)
        if(mImpl->properties.generateSkirts && (!triangBuilder.levelIsNotPresent)){ //TODO: ??
            SkirtTriangulationBuilder skirtBuilder;

            skirtBuilder.fragmentSize = inHeightMap.getSize();
            skirtBuilder.chunkSize = skirtBuilder.fragmentSize;

            for(unsigned int i = 0; i < currentLOD; ++i)
                skirtBuilder.chunkSize = (skirtBuilder.chunkSize - 1) / 2 + 1;

            applyToLevel(*tree, currentLOD, skirtBuilder);
        }

        //if present increment current LOD
        if(!triangBuilder.levelIsNotPresent){
            ++currentLOD;
        }

        error *= mImpl->properties.LODErrorFactor;
    }
    unsigned int numOfLODs = currentLOD;

    //we should recreate tree if some levels are empty
    if(numOfLODs != mImpl->properties.numOfLODs){
        QTreePtr newTree = QTreePtr(new QTree(numOfLODs));

        //we must copy data from tree to the new one
        copyQuadTreeLevels(*tree, *newTree, numOfLODs);

        tree = newTree;
    }

    //now we should build levels of details for vertices
    //we should resort vertices of original vertex buffer
    //in order of usage (first - vertices of level0 [they are used by other
    //levels too], then vertices of level1 [they are used by level1, level2 ...]
    //and so on).
    //To do that we first build binary map to find out what vertices are used
    //in current level. Having such a bitmap we can build map of transformation
    //(whose key is previous position of vertex and value is new position of vertex)
    //For each new level we do all of that but we start not from the beginning
    //of vertex array but after the last used vertex of previous LOD.

    //map of transformation
    std::vector<unsigned int> mapOfTransformation;
    mapOfTransformation.resize(allVertices.size(), allVertices.size() + 1);

    //functor to apply transformations
    IndexTransformer transform(mapOfTransformation);

    //functor to mark vertices wich are used in specified level
    //and build bitmap
    VertexMarker vertMarker;
    vertMarker.bitmap.resize(allVertices.size(), false);
    vertMarker.number = 0;

    std::vector<unsigned int> numbersOfUsedVertices;
    numbersOfUsedVertices.reserve(numOfLODs);

    //build transformation map for first level

    //first build bitmap of usage
    applyToLevel(*tree, 0, vertMarker);
    numbersOfUsedVertices.push_back(vertMarker.number);
    //build map
    for(size_t i = 0, usedVerticesIndex = 0, otherVerticesIndex = vertMarker.number; i < mapOfTransformation.size(); ++i){
        assert(usedVerticesIndex <= numbersOfUsedVertices.back());
        assert(otherVerticesIndex <= allVertices.size());

        if(vertMarker.bitmap[i]) mapOfTransformation[i] = usedVerticesIndex++;
        else mapOfTransformation[i] = otherVerticesIndex++;
    }

    //build transformation map and other needed data
    for(size_t lodLevel = 1; lodLevel < numOfLODs; ++lodLevel){
        
        for(size_t i = 0; i < vertMarker.bitmap.size(); ++i)
            vertMarker.bitmap[i] = false;
        vertMarker.number = 0;
        //build bitmap of usage for transformed indices
        applyToLevel(*tree, lodLevel, vertMarker);

        if(vertMarker.number == 0) continue;
        
        unsigned int previousVerticesNum = numbersOfUsedVertices.back();
        unsigned int newVerticesNum = 0;
        for(size_t i = 0; i < vertMarker.bitmap.size(); ++i){
            if(vertMarker.bitmap[i] && mapOfTransformation[i] >= previousVerticesNum){
                ++newVerticesNum;
            }
        }
        if(newVerticesNum == 0) continue;
        numbersOfUsedVertices.push_back(newVerticesNum + previousVerticesNum);

        //build a map of transformation using bitmap
        for(size_t i = 0, usedVerticesIndex = previousVerticesNum, otherVerticesIndex = numbersOfUsedVertices.back(); i < mapOfTransformation.size(); ++i){
            assert(usedVerticesIndex <= numbersOfUsedVertices.back());
            assert(otherVerticesIndex <= mapOfTransformation.size());
            
            //we don't touch any old data
            if(mapOfTransformation[i] >= previousVerticesNum){
                if(vertMarker.bitmap[i]) mapOfTransformation[i] = usedVerticesIndex++;
                else mapOfTransformation[i] = otherVerticesIndex++;
            }
        }
    }

    //now we should transform all the indices
    for(size_t lodLevel = 0; lodLevel < numOfLODs; ++lodLevel){
        applyToLevel(*tree, lodLevel, transform);
    }

    //resort vertices accordingly to the map
    //not used vertices are dropped
    {
        TerrainFragment::VertexCont newVertices(numbersOfUsedVertices.back());
        for(size_t i = 0; i < mapOfTransformation.size(); ++i){
            if(mapOfTransformation[i] < numbersOfUsedVertices.back())
            //if(mapOfTransformation[i] < allVertices.size()) 
               newVertices[mapOfTransformation[i]] = allVertices[i];
        }
        allVertices = newVertices;
    }
    //now we should calculate levels we have to merge.
    //we merge only those levels which are too similar 
    //(difference of number of vertices is not big enough)
    
    //it will contain numbers of vertices per each level
    std::vector<unsigned int> vertexLevels;
    for(int lodLevel = (numbersOfUsedVertices.size() - 1), lastAddedNum = 0; lodLevel >= 0; --lodLevel){
        //check difference
        if((lastAddedNum * mImpl->properties.vertexLODFactor > numbersOfUsedVertices[lodLevel] ||
                (lastAddedNum == 0)) && (numbersOfUsedVertices[lodLevel] != 0)){
            //add new level
            vertexLevels.push_back(numbersOfUsedVertices[lodLevel]);
            lastAddedNum = vertexLevels.back();
        }
    }
    //levels are built in reverse order
    std::reverse(vertexLevels.begin(), vertexLevels.end());

    VertexLevelAndErrorSetter vertexLevelAndErrorSetter;
    error = mImpl->properties.maxError;

    for(size_t level = 0, currentVertexLevel = 0; level < numOfLODs; ++level){
        if(vertexLevels[currentVertexLevel] < numbersOfUsedVertices[level]){
            ++currentVertexLevel;
        }
        vertexLevelAndErrorSetter.level = currentVertexLevel;
        vertexLevelAndErrorSetter.maxError = error;

        //set appropriate levels and errors to all the chunks of this level
        applyToLevel(*tree, level, vertexLevelAndErrorSetter);

        error *= mImpl->properties.LODErrorFactor;
    }

    //calculate AABB
    //calculate normals using the most detalised LOD
   
    //split vertices into levels and save them
    for(size_t vertexLOD = 0, lastUsedVerticesNum = 0; vertexLOD < vertexLevels.size(); ++vertexLOD){
        result->vertexLODs.push_back(TerrainFragment::VertexCont());
        result->vertexLODsStatuses.push_back(TerrainFragment::RAM);
        
        assert(vertexLevels[vertexLOD] != 0);
        assert(vertexLevels[vertexLOD] > lastUsedVerticesNum);
        if(vertexLevels[vertexLOD] > lastUsedVerticesNum){
            result->vertexLODs.back().resize(vertexLevels[vertexLOD] - lastUsedVerticesNum);
            std::copy(&allVertices[lastUsedVerticesNum], &allVertices[vertexLevels[vertexLOD]], result->vertexLODs.back().begin());
        }
        lastUsedVerticesNum = vertexLevels[vertexLOD];
    }

    //convert temporary tree to result one
    result->tree = TerrainFragment::QuadTreeOfChunksPtr(new TerrainFragment::QuadTreeOfChunks(tree->getResolution()));
    TerrainFragment::QuadTreeOfChunks::iterator resultIter = result->tree->begin();
    for(QTree::const_iterator iter = tree->begin(); iter != tree->end() || resultIter != result->tree->end(); ++iter, ++resultIter){
        iter->data.chunkData.ptr->indices.resize(iter->data.longIndices.size());
        for(size_t i = 0; i < iter->data.longIndices.size(); ++i){
            if(iter->data.longIndices[i] >= std::numeric_limits<TerrainChunk::index_t>::max()){ //here is some magic
                throw std::runtime_error("Error while preprocessing. Can't optimize terrain fragment because it has too many vertices. Try decrease error metric.");
            }
            iter->data.chunkData.ptr->indices[i] = static_cast<TerrainChunk::index_t>(iter->data.longIndices[i]);
        }
        resultIter->data = iter->data.chunkData;
    }

    //calculate AABB
    AABBCalculator aabbCalc;
    aabbCalc.mesh.mMode = Mesh::TRIANGLE_LIST;
    aabbCalc.mesh.mVertices.resize(allVertices.size());
    
    //copy vertices
    for(size_t i = 0; i < allVertices.size(); ++i){
        aabbCalc.mesh.mVertices[i].mCoord.x = allVertices[i].x;
        aabbCalc.mesh.mVertices[i].mCoord.y = allVertices[i].y;
        aabbCalc.mesh.mVertices[i].mCoord.z = allVertices[i].z;
    }

    aabbCalc.fragment = result;

    for(unsigned int i = 0; i < numOfLODs; ++i)
        applyToLevel(*result->tree, i, aabbCalc);
    
    return result; 
}

TerrainPreprocessor::Properties TerrainPreprocessor::getProperties() const{
    assert(mImpl);

    return mImpl->properties;
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
