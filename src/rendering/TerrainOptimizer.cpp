//TerrainOptimizer.cpp

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

#include "rendering/TerrainOptimizer.hpp"
#include "common/QuadTree.hpp"
#include "math/Point.hpp"
#include "data/Vertex.hpp"

#include <stdexcept>
#include <algorithm>

#include <cassert>
#include <cmath>

using hydra::rendering::TerrainOptimizer;
using hydra::data::HeightMap;
using hydra::common::QuadTree;

unsigned int log2(unsigned int argument){
	return static_cast<unsigned int>(log(static_cast<double>(argument)) / log(2.0f));
}


///additional data to store in tree nodes
struct NodeData{
    //we store 4 errors for all optional vertices of quad
    float vertex_errors[4]; 

    //we store 4 errors for all optional quads (sons)
    float quad_errors[4];

    //used for 'flags' parameter
    enum EnableMask{
        NONE = 0,
        NORTH_VERTEX_MASK = 0x01,
        EAST_VERTEX_MASK = 0x02,
        SOUTH_VERTEX_MASK = 0x04,
        WEST_VERTEX_MASK = 0x08,
        NORTH_WEST_QUAD_MASK = 0x10,
        NORTH_EAST_QUAD_MASK = 0x20,
        SOUTH_WEST_QUAD_MASK = 0x40,
        SOUTH_EAST_QUAD_MASK = 0x80
    };

    //flags (enable/disable) of vertices and child quads
    unsigned char flags;
};

enum VertexPos{
    SOUTH_WEST_VERTEX = 0,
    NORTH_WEST_VERTEX = 1,
    SOUTH_EAST_VERTEX = 2,
    NORTH_EAST_VERTEX = 3,
    NORTH_VERTEX = 4,
    EAST_VERTEX = 5,
    SOUTH_VERTEX = 6,
    WEST_VERTEX = 7,
    CENTER_VERTEX = 8
};

typedef QuadTree<NodeData> RQuadTree; //restricted quad tree

struct TerrainOptimizer::Impl{

    Impl(unsigned int inSize): tree(new RQuadTree(inSize)){

    }

    ///==================================UTILITIES===================================

    ///simply calculates y component of distance betweeen line and point
    inline float errorBetweenLineAndPoint(float inPointHeight, float inVert1, float inVert2) const{
        float error = ((inVert2 + inVert1) / 2.0f) - inPointHeight;
        //abs without function calls
        if(error < 0.0f) error = -error;

        return error;
    }

    //calculates vertex position which is bottom left corner of specified subquad
    inline VertexPos getBottomLeftCorner(RQuadTree::NodePos which) const{
        switch(which){
            //west
            case RQuadTree::NORTH_WEST_QUAD: return WEST_VERTEX;
            //center
            case RQuadTree::NORTH_EAST_QUAD: return CENTER_VERTEX;
            //same
            case RQuadTree::SOUTH_WEST_QUAD: return SOUTH_WEST_VERTEX;
            //south
            case RQuadTree::SOUTH_EAST_QUAD: return SOUTH_VERTEX;
            default: throw std::runtime_error("Error while optimization.");
        }
    }


    inline unsigned int calcPointPos(unsigned int prevBottomLeftCorner, unsigned int currSize, VertexPos which, unsigned int totalSize) const{
        unsigned int firstComp = 0, secondComp = 0;
        
        //calc first component (vertical displacement)
        if(which == WEST_VERTEX || which == CENTER_VERTEX || which == EAST_VERTEX) firstComp = totalSize * ((currSize - 1)/2);
        if(which == NORTH_WEST_VERTEX || which == NORTH_VERTEX || which == NORTH_EAST_VERTEX) firstComp = totalSize * (currSize - 1);

        //calc second component (horizontal displacement)
        if(which == SOUTH_VERTEX || which == CENTER_VERTEX || which == NORTH_VERTEX) secondComp = (currSize - 1) / 2;
        if(which == EAST_VERTEX || which == SOUTH_EAST_VERTEX || which == NORTH_EAST_VERTEX) secondComp = (currSize - 1);
        
        return prevBottomLeftCorner + firstComp + secondComp;
    }


    ///maximum error
    inline float maxErrorOfQuad(const NodeData& data) const{
        return std::max(
                std::max( 
                    std::max(data.vertex_errors[0], data.vertex_errors[1]),
                    std::max(data.vertex_errors[2], data.vertex_errors[3])
                ), 
                std::max(
                    std::max(data.quad_errors[0], data.quad_errors[1]), 
                    std::max(data.quad_errors[2], data.quad_errors[3])
                )
               );
    }
    
    inline NodeData::EnableMask fromVertexToMask(VertexPos inVertex) const{
        switch (inVertex){
            case NORTH_VERTEX: return NodeData::NORTH_VERTEX_MASK;
            case WEST_VERTEX: return NodeData::WEST_VERTEX_MASK;
            case EAST_VERTEX: return NodeData::EAST_VERTEX_MASK;
            case SOUTH_VERTEX: return NodeData::SOUTH_VERTEX_MASK;
            default: return NodeData::NONE;
        }
    }

    inline NodeData::EnableMask fromQuadToMask(RQuadTree::NodePos inQuad) const{
        switch (inQuad){
            case RQuadTree::NORTH_WEST_QUAD: return NodeData::NORTH_WEST_QUAD_MASK;
            case RQuadTree::NORTH_EAST_QUAD: return NodeData::NORTH_EAST_QUAD_MASK;
            case RQuadTree::SOUTH_WEST_QUAD: return NodeData::SOUTH_WEST_QUAD_MASK;
            case RQuadTree::SOUTH_EAST_QUAD: return NodeData::SOUTH_EAST_QUAD_MASK;
            default: return NodeData::NONE;
        }
    }

    //next quad direction in clock-wise order
    inline RQuadTree::NodePos nextCW(RQuadTree::NodePos inPos) const{
        switch(inPos){
            case RQuadTree::NORTH_WEST_QUAD: return RQuadTree::NORTH_EAST_QUAD;
            case RQuadTree::SOUTH_WEST_QUAD: return RQuadTree::NORTH_WEST_QUAD;
            case RQuadTree::NORTH_EAST_QUAD: return RQuadTree::SOUTH_EAST_QUAD;
            case RQuadTree::SOUTH_EAST_QUAD: return RQuadTree::SOUTH_WEST_QUAD;
            default: throw std::runtime_error("Can't get next clockwise quad. Wrong input value.");
        }
    }

    //next quad direction in counter clock-wise order
    inline RQuadTree::NodePos nextCCW(RQuadTree::NodePos inPos) const{
        switch(inPos){
            case RQuadTree::NORTH_WEST_QUAD: return RQuadTree::SOUTH_WEST_QUAD;
            case RQuadTree::SOUTH_WEST_QUAD: return RQuadTree::SOUTH_EAST_QUAD;
            case RQuadTree::NORTH_EAST_QUAD: return RQuadTree::NORTH_WEST_QUAD;
            case RQuadTree::SOUTH_EAST_QUAD: return RQuadTree::NORTH_EAST_QUAD;
            default: throw std::runtime_error("Can't get next counter clockwise quad. Wrong input value.");
        }
    }

    //returns optional point location between quads
    inline VertexPos getVertexBetweenQuads(RQuadTree::NodePos inQuad1, RQuadTree::NodePos inQuad2) const{
        switch(inQuad1 + inQuad2){
            case (RQuadTree::NORTH_WEST_QUAD + RQuadTree::SOUTH_WEST_QUAD): return WEST_VERTEX;
            case (RQuadTree::SOUTH_WEST_QUAD + RQuadTree::SOUTH_EAST_QUAD): return SOUTH_VERTEX;
            case (RQuadTree::NORTH_WEST_QUAD + RQuadTree::NORTH_EAST_QUAD): return NORTH_VERTEX;
            case (RQuadTree::NORTH_EAST_QUAD + RQuadTree::SOUTH_EAST_QUAD): return EAST_VERTEX;
            default: throw std::runtime_error("Specified quads do not have shared edge.");
        }
    }

    inline VertexPos getOpposite(VertexPos inVertexPos) const{
        switch(inVertexPos){
            case NORTH_VERTEX: return SOUTH_VERTEX;
            case SOUTH_VERTEX: return NORTH_VERTEX;
            case EAST_VERTEX: return WEST_VERTEX;
            case WEST_VERTEX: return EAST_VERTEX;
            default: throw std::runtime_error("Can't get opposite vertex for specified one.");
        }
    }

    //returns total size of height map used to build tree
    inline unsigned int getTotalSize() const{
        assert(tree);
        unsigned int res = tree->getResolution();
        res = 1 << (res);
        return (res + 1);

    }

    ///================================END OF UTILITIES=================================

    //calculates maximum errors for hole tree (vertex errors and quad errors)
    //drops 'enable flags'
    void calculateMaxErrors(const HeightMap& inHeightmap){
        assert(tree);

        clearEnableFlags();
        recursiveCalculateMaxErrors(tree->getRoot(), inHeightmap, 0, inHeightmap.getSize());
    }

    //recursive function to calculate errors based on original heightmap and save them into our restricted quad tree
    void recursiveCalculateMaxErrors(RQuadTree::Node& node, const HeightMap& heightmap, unsigned int bottomLeftCorner, unsigned int size){

        //calculate vertex errors
        {
            //save corner heights
            float upperLeftHeight = heightmap[calcPointPos(bottomLeftCorner, size, NORTH_WEST_VERTEX, heightmap.getSize())];
            float bottomLeftHeight = heightmap[calcPointPos(bottomLeftCorner, size, SOUTH_WEST_VERTEX, heightmap.getSize())];
            float upperRightHeight = heightmap[calcPointPos(bottomLeftCorner, size, NORTH_EAST_VERTEX, heightmap.getSize())];
            float bottomRightHeight = heightmap[calcPointPos(bottomLeftCorner, size, SOUTH_EAST_VERTEX, heightmap.getSize())];

            node.data.vertex_errors[static_cast<VertexPos>(NORTH_VERTEX - 4)] = errorBetweenLineAndPoint(heightmap[calcPointPos(bottomLeftCorner, size, NORTH_VERTEX, heightmap.getSize())], upperLeftHeight, upperRightHeight);
            node.data.vertex_errors[static_cast<VertexPos>(EAST_VERTEX - 4)] = errorBetweenLineAndPoint(heightmap[calcPointPos(bottomLeftCorner, size, EAST_VERTEX, heightmap.getSize())], upperRightHeight, bottomRightHeight);
            node.data.vertex_errors[static_cast<VertexPos>(SOUTH_VERTEX - 4)] = errorBetweenLineAndPoint(heightmap[calcPointPos(bottomLeftCorner, size, SOUTH_VERTEX, heightmap.getSize())], bottomRightHeight, bottomLeftHeight);
            node.data.vertex_errors[static_cast<VertexPos>(WEST_VERTEX - 4)] = errorBetweenLineAndPoint(heightmap[calcPointPos(bottomLeftCorner, size, WEST_VERTEX, heightmap.getSize())], bottomLeftHeight, upperLeftHeight);

        }

        //calculate quad error
        for(int i = 0; i < 4; ++i){
            if(node.nodes[i] != 0){
                //save bottom left corner of subquad
                unsigned int newBottomLeftCorner = 
                    calcPointPos(bottomLeftCorner, size, 
                            getBottomLeftCorner(static_cast<RQuadTree::NodePos>(i)), 
                            heightmap.getSize());
                recursiveCalculateMaxErrors(tree->getNode(node.nodes[i]), 
                                    heightmap, 
                                    newBottomLeftCorner, 
                                    (size - 1)/2 + 1 );
                //error is a maximum error between all inner optional elements of quad and error of central point
                
                float centerHeight = heightmap[calcPointPos(newBottomLeftCorner, (size - 1)/2 + 1, CENTER_VERTEX, heightmap.getSize())];
                float lineHeight1 = heightmap[newBottomLeftCorner]; //same corner
                float lineHeight2 = heightmap[calcPointPos(newBottomLeftCorner, (size - 1)/2 + 1, NORTH_EAST_VERTEX, heightmap.getSize())];
                float centerError = errorBetweenLineAndPoint(centerHeight, lineHeight1, lineHeight2);
                node.data.quad_errors[i] = std::max(maxErrorOfQuad(tree->getNode(node.nodes[i]).data), centerError);
            }
            else{ //quad has no sons
                node.data.quad_errors[i] = 0;
            }
        }
    }

    void clearEnableFlags(){
        assert(tree);

        recursiveClearEnableFlags(tree->getRoot());
    }

    void recursiveClearEnableFlags(RQuadTree::Node& inNode){
        inNode.data.flags = NodeData::NONE;

        for(int i = 0; i < 4; ++i){
            if(inNode.nodes[i] != 0) recursiveClearEnableFlags(tree->getNode(inNode.nodes[i]));
        }
    }

    void calculateEnableFlags(float inMaxError){
        assert(tree);

        recursiveCalculateEnableFlags(0, inMaxError);
    }

    //recursively enables chain from root to current quad
    void enableQuad(unsigned short inNodeId){
        unsigned short nodeId = inNodeId;
        while(nodeId != 0){
            RQuadTree::Node& parent = tree->getNode(tree->getNode(nodeId).parent);
        
            //find out what position current node takes
            unsigned int pos;
            for(pos = 0; pos < 4; ++pos){
                if(parent.nodes[pos] == nodeId){
                    break;
                }
            }

            //check whether current node is enabled
            if(!(parent.data.flags & fromQuadToMask(static_cast<RQuadTree::NodePos>(pos)))){
                //enable it
                parent.data.flags |= fromQuadToMask(static_cast<RQuadTree::NodePos>(pos));
            
                //enable vertices
                VertexPos vertex = getVertexBetweenQuads(static_cast<RQuadTree::NodePos>(pos), 
                                                        nextCCW(static_cast<RQuadTree::NodePos>(pos)));
                if(!(parent.data.flags & fromVertexToMask(vertex))){
                    parent.data.flags |= fromVertexToMask(vertex);
                    notifyNeighbours(tree->getNode(nodeId).parent, vertex);
                }
                vertex = getVertexBetweenQuads(static_cast<RQuadTree::NodePos>(pos), 
                                                        nextCW(static_cast<RQuadTree::NodePos>(pos)));
                if(!(parent.data.flags & fromVertexToMask(vertex))){
                    parent.data.flags |= fromVertexToMask(vertex);
                    notifyNeighbours(tree->getNode(nodeId).parent, vertex);
                }
                nodeId = tree->getNode(nodeId).parent;
            }
            else{
                //if we found already enabled quad, exit
                return;
            }
        }
    }

    //recursively notifies all the neighbours of enabled vertex
    //all the dependencies will be resolved (current vertex must be enabled before)
    //handle NORTH, WEST, EAST, SOUTH and CENTER vertices
    void notifyNeighbours(unsigned short inNodeId, VertexPos inEnabledVertex){
        //first we enable current node (if it was not enabled yet)
        if(inNodeId != 0){
            enableQuad(inNodeId);
        
            //for non-center vertex we should enable neighbour quad and his vertex
            if(inEnabledVertex != CENTER_VERTEX){
                //check neighbour quad
                if(tree->getNode(inNodeId).neighbours[inEnabledVertex - 4] != 0){
                    RQuadTree::Node& neighbour = tree->getNode(tree->getNode(inNodeId).neighbours[inEnabledVertex - 4]);
                    //if not enabled, enable it
                    if(!(neighbour.data.flags & fromVertexToMask(getOpposite(inEnabledVertex)))){
                        neighbour.data.flags |= fromVertexToMask(getOpposite(inEnabledVertex));
                        //notify
                        notifyNeighbours(tree->getNode(inNodeId).neighbours[inEnabledVertex - 4], getOpposite(inEnabledVertex));
                    }
                }
            }
        }
    }

    void recursiveCalculateEnableFlags(unsigned short inNodeId, float inMaxError){
           
        RQuadTree::Node& inNode = tree->getNode(inNodeId); 
        //calc vertices
        for(int i = 0; i < 4; ++i){
            if(inNode.data.flags & fromVertexToMask(static_cast<VertexPos>(i + 4))) continue;
            if(inNode.data.vertex_errors[i] > inMaxError){
                inNode.data.flags |= fromVertexToMask(static_cast<VertexPos>(i + 4));
                
                notifyNeighbours(inNodeId, static_cast<VertexPos>(i + 4));
            }
        }

        //calc subquads
        for(int i = 0; i < 4; ++i){
            if(inNode.nodes[i] == 0) continue;
            if(inNode.data.flags & fromQuadToMask(static_cast<RQuadTree::NodePos>(i))){
                recursiveCalculateEnableFlags(inNode.nodes[i], inMaxError);
                continue;
            }

            if(inNode.data.quad_errors[i] > inMaxError){
                //inNode.data.flags |= fromQuadToMask(static_cast<RQuadTree::NodePos>(i));
                enableQuad(inNode.nodes[i]);


                notifyNeighbours(inNode.nodes[i], CENTER_VERTEX);
                
                //recursive call for enabled quad
                recursiveCalculateEnableFlags(inNode.nodes[i], inMaxError);
            }
        }
    }

    //recursive function to triangulate specified node in one pass 
    void recursiveTriangulateInOnePass(std::vector<unsigned int>& inoutList, RQuadTree::Node& inNode, RQuadTree::NodePos inStartCorner, unsigned int inSouthWestCornerCoord, unsigned int inCurrentSize) const{
       
        RQuadTree::NodePos nextQuadPos = inStartCorner;
        RQuadTree::NodePos nextQuadStart = inStartCorner;


        if(inNode.nodes[nextQuadPos] != 0 && (inNode.data.flags & fromQuadToMask(nextQuadPos))){
            recursiveTriangulateInTwoPasses(inoutList, tree->getNode(inNode.nodes[nextQuadPos]), nextQuadStart, calcPointPos(inSouthWestCornerCoord, inCurrentSize, getBottomLeftCorner(nextQuadPos), getTotalSize()), (inCurrentSize - 1) / 2 + 1);
        }
        else{
            //add triangles
            if((inNode.nodes[nextCCW(nextQuadPos)] != 0 && (inNode.data.flags & fromQuadToMask(nextCCW(nextQuadPos)))) ||
                    (inNode.data.flags & fromVertexToMask(getVertexBetweenQuads(nextQuadPos, nextCCW(nextQuadPos))))){
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, getVertexBetweenQuads(nextQuadPos, nextCCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
            }
            else{
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextCCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
            }
        }

        //second quad
        nextQuadPos = nextCCW(nextQuadPos);
        nextQuadStart = nextCW(nextQuadStart);

        if(inNode.nodes[nextQuadPos] != 0 && (inNode.data.flags & fromQuadToMask(nextQuadPos))){
            recursiveTriangulateInOnePass(inoutList, tree->getNode(inNode.nodes[nextQuadPos]), nextQuadStart, calcPointPos(inSouthWestCornerCoord, inCurrentSize, getBottomLeftCorner(nextQuadPos), getTotalSize()), (inCurrentSize - 1) / 2 + 1);
        }
        else{
            //add triangles
            //if previous optinal vertex enabled or previous quad enabled
            if((inNode.data.flags & fromVertexToMask(getVertexBetweenQuads(nextQuadPos, nextCW(nextQuadPos)))) ||
                    (inNode.nodes[nextCW(nextQuadPos)] != 0 && (inNode.data.flags & fromQuadToMask(nextCW(nextQuadPos))))){
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, getVertexBetweenQuads(nextQuadPos, nextCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
            }
            
            if((inNode.nodes[nextCCW(nextQuadPos)] != 0 && (inNode.data.flags & fromQuadToMask(nextCCW(nextQuadPos)))) ||
                    (inNode.data.flags & fromVertexToMask(getVertexBetweenQuads(nextQuadPos, nextCCW(nextQuadPos))))){
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, getVertexBetweenQuads(nextQuadPos, nextCCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
            }
            else{
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextCCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
            }
        }

        //third quad
        nextQuadPos = nextCCW(nextQuadPos);
        if(inNode.nodes[nextQuadPos] != 0 && (inNode.data.flags & fromQuadToMask(nextQuadPos))){
            recursiveTriangulateInOnePass(inoutList, tree->getNode(inNode.nodes[nextQuadPos]), inStartCorner, calcPointPos(inSouthWestCornerCoord, inCurrentSize, getBottomLeftCorner(nextQuadPos), getTotalSize()), (inCurrentSize - 1) / 2 + 1);
        }
        else{
            //add triangles
            //if previous optinal vertex enabled
            if((inNode.data.flags & fromVertexToMask(getVertexBetweenQuads(nextQuadPos, nextCW(nextQuadPos)))) ||
                    (inNode.nodes[nextCW(nextQuadPos)] != 0 && (inNode.data.flags & fromQuadToMask(nextCW(nextQuadPos))))){
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, getVertexBetweenQuads(nextCW(nextQuadPos), nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
            }

            if((inNode.nodes[nextCCW(nextQuadPos)] != 0 && (inNode.data.flags & fromQuadToMask(nextCCW(nextQuadPos)))) ||
                    (inNode.data.flags & fromVertexToMask(getVertexBetweenQuads(nextQuadPos, nextCCW(nextQuadPos))))){
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, getVertexBetweenQuads(nextQuadPos, nextCCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
            }
            else{
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextCCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
            }
        }

        //forth quad
        nextQuadPos = nextCCW(nextQuadPos);
        nextQuadStart = nextCCW(inStartCorner);
        if(inNode.nodes[nextQuadPos] != 0 && (inNode.data.flags & fromQuadToMask(nextQuadPos))){
            recursiveTriangulateInOnePass(inoutList, tree->getNode(inNode.nodes[nextQuadPos]), nextQuadStart, calcPointPos(inSouthWestCornerCoord, inCurrentSize, getBottomLeftCorner(nextQuadPos), getTotalSize()), (inCurrentSize - 1) / 2 + 1);
        }
        else{
            //add triangles
            //if previous optinal vertex enabled
            if((inNode.data.flags & fromVertexToMask(getVertexBetweenQuads(nextQuadPos, nextCW(nextQuadPos)))) ||
                    (inNode.nodes[nextCW(nextQuadPos)] != 0 && (inNode.data.flags & fromQuadToMask(nextCW(nextQuadPos))))){
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, getVertexBetweenQuads(nextCW(nextQuadPos), nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
            }

            if((inNode.nodes[nextCCW(nextQuadPos)] != 0 && (inNode.data.flags & fromQuadToMask(nextCCW(nextQuadPos)))) ||
                    (inNode.data.flags & fromVertexToMask(getVertexBetweenQuads(nextQuadPos, nextCCW(nextQuadPos))))){
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, getVertexBetweenQuads(nextQuadPos, nextCCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
            }
            else{
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextCCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
            }
        }

        //last fifth quad (second pass of first one)
        nextQuadStart = nextCCW(nextQuadStart);
        nextQuadPos = nextCCW(nextQuadPos);

        if(inNode.nodes[inStartCorner] != 0 && (inNode.data.flags & fromQuadToMask(inStartCorner))){
            recursiveTriangulateInTwoPasses(inoutList, tree->getNode(inNode.nodes[inStartCorner]), nextQuadStart, calcPointPos(inSouthWestCornerCoord, inCurrentSize, getBottomLeftCorner(inStartCorner), getTotalSize()), (inCurrentSize - 1) / 2 + 1);
        }
        else{
            //add triangles
            //if previous optinal vertex enabled
            if((inNode.data.flags & fromVertexToMask(getVertexBetweenQuads(nextQuadPos, nextCW(nextQuadPos)))) ||
                    (inNode.nodes[nextCW(nextQuadPos)] != 0 && (inNode.data.flags & fromQuadToMask(nextCW(nextQuadPos))))){
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, getVertexBetweenQuads(nextCW(nextQuadPos), nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
            }

            //do nothing
        }
    }

    //recursive function to triangulate specified node in two passes
    //(should be called twice for every node as it triangulates only half of quad)
    //see [1] for more info
    void recursiveTriangulateInTwoPasses(std::vector<unsigned int>& inoutList, RQuadTree::Node& inNode, RQuadTree::NodePos inStartCorner, unsigned int inSouthWestCornerCoord, unsigned int inCurrentSize) const{
        
        RQuadTree::NodePos nextQuadPos = inStartCorner;
        RQuadTree::NodePos nextQuadStart = inStartCorner;

        //first quad
        if(inNode.nodes[nextQuadPos] != 0 && (inNode.data.flags & fromQuadToMask(nextQuadPos))){
            recursiveTriangulateInTwoPasses(inoutList, tree->getNode(inNode.nodes[nextQuadPos]), nextQuadStart, calcPointPos(inSouthWestCornerCoord, inCurrentSize, getBottomLeftCorner(nextQuadPos), getTotalSize()), (inCurrentSize - 1) / 2 + 1);
        }
        else{
            //add triangles
            if((inNode.nodes[nextCCW(nextQuadPos)] != 0 && (inNode.data.flags & fromQuadToMask(nextCCW(nextQuadPos)))) ||
                    (inNode.data.flags & fromVertexToMask(getVertexBetweenQuads(nextQuadPos, nextCCW(nextQuadPos))))){
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, getVertexBetweenQuads(nextQuadPos, nextCCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
            }
            else{
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextCCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
            }
        }

        //second quad
        nextQuadPos = nextCCW(nextQuadPos);
        nextQuadStart = nextCW(nextQuadStart);
        
        if(inNode.nodes[nextQuadPos] != 0 && (inNode.data.flags & fromQuadToMask(nextQuadPos))){
            recursiveTriangulateInOnePass(inoutList, tree->getNode(inNode.nodes[nextQuadPos]), nextQuadStart, calcPointPos(inSouthWestCornerCoord, inCurrentSize, getBottomLeftCorner(nextQuadPos), getTotalSize()), (inCurrentSize - 1) / 2 + 1);
        }
        else{
            //add triangles
            //if previous optinal vertex enabled
            if((inNode.data.flags & fromVertexToMask(getVertexBetweenQuads(nextQuadPos, nextCW(nextQuadPos)))) ||
                    (inNode.nodes[nextCW(nextQuadPos)] != 0 && (inNode.data.flags & fromQuadToMask(nextCW(nextQuadPos))))){
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, getVertexBetweenQuads(nextCW(nextQuadPos), nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
            }

            if((inNode.nodes[nextCCW(nextQuadPos)] != 0 && (inNode.data.flags & fromQuadToMask(nextCCW(nextQuadPos)))) ||
                    (inNode.data.flags & fromVertexToMask(getVertexBetweenQuads(nextQuadPos, nextCCW(nextQuadPos))))){
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, getVertexBetweenQuads(nextQuadPos, nextCCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
            }
            else{
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextCCW(nextQuadPos)), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
            }
        }

        //last third quad
        nextQuadPos = nextCCW(nextQuadPos);
        if(inNode.nodes[nextQuadPos] != 0 && (inNode.data.flags & fromQuadToMask(nextQuadPos))){
            recursiveTriangulateInTwoPasses(inoutList, tree->getNode(inNode.nodes[nextQuadPos]), inStartCorner, calcPointPos(inSouthWestCornerCoord, inCurrentSize, getBottomLeftCorner(nextQuadPos), getTotalSize()), (inCurrentSize - 1) / 2 + 1);
        }
        else{
            //add triangles
            //if previous optinal vertex enabled
            if((inNode.data.flags & fromVertexToMask(getVertexBetweenQuads(nextQuadPos, nextCW(nextQuadPos)))) ||
                    (inNode.nodes[nextCW(nextQuadPos)] != 0 && (inNode.data.flags & fromQuadToMask(nextCW(nextQuadPos))))){
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, CENTER_VERTEX, getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, getVertexBetweenQuads(nextCW(nextQuadPos), nextQuadPos), getTotalSize()));
                inoutList.push_back(calcPointPos(inSouthWestCornerCoord, inCurrentSize, static_cast<VertexPos>(nextQuadPos), getTotalSize()));
            }

            //do nothing
        }
    }



    ///builds triangle list
    std::vector<unsigned int> triangulate() const{
        assert(tree);

        std::vector<unsigned int> list;
        recursiveTriangulateInOnePass(list, tree->getRoot(), RQuadTree::SOUTH_WEST_QUAD, 0, getTotalSize());

        return list;
    }

 
    hydra::common::SharedPtr<RQuadTree>::Type tree;
};

TerrainOptimizer::TerrainOptimizer(const HeightMap& inHeights): mImpl(new TerrainOptimizer::Impl(log2(inHeights.getSize() - 1))){
    //empty tree is built here
    //we should fill tree (full) with error data using height map
    mImpl->calculateMaxErrors(inHeights);

}

TerrainOptimizer::~TerrainOptimizer(){

}

void TerrainOptimizer::rebuild(const HeightMap& inHeights){
    assert(mImpl);
    assert(mImpl->tree);
    
    if(log2(inHeights.getSize() - 1) != mImpl->tree->getResolution()){
        mImpl->tree = hydra::common::SharedPtr<RQuadTree>::Type(new RQuadTree(log2(inHeights.getSize() - 1)));
    }

    //recalculate errors
    mImpl->calculateMaxErrors(inHeights);
}

void TerrainOptimizer::generateLOD(float inMaxError){
    assert(mImpl);
    
    //first we build tree which contains all the included quads and vertices
    //we use same tree
    mImpl->calculateEnableFlags(inMaxError);
}

std::vector<unsigned int> TerrainOptimizer::getTriangulation() const{
    assert(mImpl);

    //then we should triangulate those data and build one single triangle strip
    //we return result to user
    return mImpl->triangulate();
}

std::vector<unsigned int> TerrainOptimizer::getTriangulation(const std::vector<int>& inPositionStack) const{
    assert(mImpl);
    assert(mImpl->tree);

    if(inPositionStack.size() > mImpl->tree->getResolution()) throw std::runtime_error("Input stack has too much elements. Tree's resolution is less than this value. Triangulation can't be built.");

    const unsigned int totalSize = mImpl->getTotalSize();

    unsigned int currentSize = totalSize;
    unsigned int bottomLeftCornerCoord = 0;
    std::vector<unsigned int> list;
    unsigned short nodeIndex = 0;

    //we triangulate only the chunk we need
    for(size_t i = 0; i < inPositionStack.size(); ++i){
        RQuadTree::NodePos nextNodePos = static_cast<RQuadTree::NodePos>(inPositionStack[i]);

        if(!(mImpl->tree->getNode(nodeIndex).data.flags & mImpl->fromQuadToMask(nextNodePos))) return list; //return empty set of indices
        //throw std::runtime_error("Specified quad is not enabled. Triangulation can't be built. Rebuild LOD with lower error value");
        
        nodeIndex = mImpl->tree->getNode(nodeIndex).nodes[nextNodePos];

        bottomLeftCornerCoord = mImpl->calcPointPos(bottomLeftCornerCoord, currentSize, mImpl->getBottomLeftCorner(nextNodePos), totalSize);     
    
        currentSize = (currentSize - 1) / 2 + 1;
    }

    mImpl->recursiveTriangulateInOnePass(list, mImpl->tree->getNode(nodeIndex), RQuadTree::SOUTH_WEST_QUAD, bottomLeftCornerCoord, currentSize);

    return list;
}

std::vector<hydra::data::Vertex> TerrainOptimizer::generateVertices(const HeightMap& inHeights, float inVertexStride){
    std::vector<hydra::data::Vertex> result;
    result.resize(inHeights.getSize() * inHeights.getSize());

    math::Point startPos;
    startPos.x = - inVertexStride * (inHeights.getSize() - 1) / 2;
    startPos.z = inVertexStride * (inHeights.getSize() - 1) / 2; 
    startPos.y = 0;
    for(unsigned int i = 0; i < inHeights.getSize(); ++i){
        for(unsigned int j = 0; j < inHeights.getSize(); ++j){
            hydra::data::Vertex newVertex;
            newVertex.mCoord.x = startPos.x + j * inVertexStride;
            newVertex.mCoord.y = startPos.y + (inHeights[i * inHeights.getSize() + j]);
            newVertex.mCoord.z = startPos.z - i * inVertexStride;

            newVertex.mTexCoord.x = static_cast<float>(j) / inHeights.getSize();
            newVertex.mTexCoord.y = static_cast<float>(i) / inHeights.getSize();

            //normal??
            //TODO

            result[i * inHeights.getSize() + j] = newVertex;
        }
    }

    return result;
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
