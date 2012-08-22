//QuadTree.hpp

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


#ifndef QUAD_TREE_HPP__
#define QUAD_TREE_HPP__

/**
 * \class hydra::common::QuadTree
 * \brief Class for representing quad tree (data structure).
 *
 * QuadTree is used for generating levels of details and optimization.
 * hydra::rendering::TerrainOptimizer uses it.
 * ALso hydra::rendering::TerrainPreprocessor uses it.
 * Each node contains 4 links to its childs.
 * In this implementation it also contains link to parent,
 * and 4 links to neighbours. User may store some additional data expanding template.
 * Implementation does not use pointers and dynamic memory
 * but it uses std::vector and indices.
 * Maximum quad tree resolution is 8 levels.
 * Minimum resolution is 1.
 *
 * QuadTree may be built for square matrix of size (2^n + 1).
 * 
 * \author A.V.Medvedev
 * \date 15.08.2010
 */

#include <vector>
#include <stdexcept>
#include <utility>
#include <cassert>

namespace hydra{

namespace common{
    
template <typename NODE_DATA>
class QuadTree{

public:

    /** 
     * \brief Represents node of QuadTree.
     * 
     * Contains some needed data + user data.
     *
     * \see hydra::rendering::QuadTree
     */
    struct Node{
        ///children (if 0 - no son)
        unsigned short nodes[4];

        ///neighbours (0 - no neighbour)
        unsigned short neighbours[4];

        ///parent node
        unsigned short parent;

        ///your data
        NODE_DATA data;
    };

    ///functor for use with algorithms
    struct NodeFunctor{
        ///virtual empty destructor
        virtual ~NodeFunctor(){}
        ///operator() to handle node. User should implement it.
        virtual void operator()(QuadTree<NODE_DATA>& tree, Node& inNode) = 0;
    };


    ///node position. Used as index in 'nodes' array of node.
    enum NodePos{
        SOUTH_WEST_QUAD = 0,
        NORTH_WEST_QUAD = 1,
        SOUTH_EAST_QUAD = 2,
        NORTH_EAST_QUAD = 3
    };

    ///position of neightbour. Used as index in 'neighbours' array of node.
    enum NeighbourPos{
        NORTH = 0,
        EAST = 1,
        SOUTH = 2,
        WEST = 3
    };

    ///builds full quad tree (with connected nodes).
    ///resolution is a log2(width - 1)
    ///\warning quad tree may be build only for square matrix with size of 2^n + 1
    QuadTree(unsigned int inResolution);
    
    ///returns reference to node with specified id
    inline Node& getNode(unsigned short inIndex);

    ///returns const reference to node with specified id
    inline const Node& getNode(unsigned short inIndex) const;

    ///returns id of node specified by stack of positions
    inline unsigned short getNodeID(const std::vector<int>& inStack) const;
    
    ///returns reference to root node
    inline Node& getRoot();

    ///returns const reference to root node
    inline const Node& getRoot() const;

    ///returns value of resoultion (log2(size - 1))
    inline unsigned int getResolution() const{
        return mResolution;
    }

    ///function clues neighbouring subtrees filling their 'neighbour' fields
    ///Works recursively. You should specify oprientation of subtrees:
    ///for example, if NORTH is specified, first subtree argument will be north subtree
    ///and second one is southern.
    void clueSubtrees(unsigned short inFirstTree, unsigned short inSecondTree, NeighbourPos inOrient);

    ///returns opposite direction for specified one.
    ///For example: if NORTH is specified - SOUTH will be returned.
    static inline NeighbourPos getOpposite(NeighbourPos inPos){
        int pos = inPos + 2;
        if(pos > 3) pos -= 4;
        return static_cast<NeighbourPos>(pos);
    }

    ///returns opposite quad from specified one and direction
    static inline NodePos getOpposite(NodePos inPos, NeighbourPos inDir){
        if(inDir == SOUTH || inDir == NORTH){
            int res = inPos;
            res += ((inPos % 2) == 0)? 1 : (-1);
            return static_cast<NodePos>(res);
        }
        else{
            int res = inPos;
            res += 2;
            if(res > 3) res -= 4;
            return static_cast<NodePos>(res);
        }
    }

    ///returns quad positions which are at specified direction (NeighbourPos)
    static inline std::pair<NodePos, NodePos> getQuadsByDirection(NeighbourPos inPos){
        std::pair<NodePos, NodePos> result;
        switch (inPos){
            case NORTH:
                result.first = NORTH_WEST_QUAD;
                result.second = NORTH_EAST_QUAD;
                return result;
            case SOUTH:
                result.first = SOUTH_WEST_QUAD;
                result.second = SOUTH_EAST_QUAD;
                return result;
            case WEST:
                result.first = SOUTH_WEST_QUAD;
                result.second = NORTH_WEST_QUAD;
                return result;
            case EAST:
                result.first = SOUTH_EAST_QUAD;
                result.second = NORTH_EAST_QUAD;
                return result;
            default: throw std::runtime_error("Can't return quads of specified direction.");
        }
    }

private:
    typedef std::vector<Node> QuadTreeCont;

public:
    ///quadtree iterator
    typedef typename QuadTreeCont::iterator iterator;

    ///quadtree const iterator
    typedef typename QuadTreeCont::const_iterator const_iterator;

    ///value type
    typedef NODE_DATA value_type;

    ///returns iterator to first node
    inline iterator begin(){
        return mNodes.begin();
    }

    ///returns const iterator to first node
    inline const_iterator begin() const{
        return mNodes.begin();
    }

    ///returns iterator to the node after the last one
    inline iterator end(){
        return mNodes.end();
    }

    ///returns const iterator to the node after the last one
    inline const_iterator end() const{
        return mNodes.end();
    }

private:
    QuadTreeCont mNodes;
    unsigned int mResolution;
};


template <typename NODE_DATA>
void QuadTree<NODE_DATA>::clueSubtrees(unsigned short inFirstTree, unsigned short inSecondTree, NeighbourPos inOrient){
    //first clue roots
    getNode(inFirstTree).neighbours[getOpposite(inOrient)] = inSecondTree;
    getNode(inSecondTree).neighbours[inOrient] = inFirstTree;

    //now we should clue subtrees
    //we will always have only 2 border elements
    std::pair<NodePos, NodePos> borders = getQuadsByDirection(inOrient);
    unsigned short borderNodes[2];
    //get border nodes' codes (for first subtree)
    borderNodes[0] = getNode(inFirstTree).nodes[getOpposite(borders.first, inOrient)];
    borderNodes[1] = getNode(inFirstTree).nodes[getOpposite(borders.second, inOrient)];

    //if subtrees have children, clue them recursively
    if(getNode(inSecondTree).nodes[borders.first] != 0){
        clueSubtrees(borderNodes[0], getNode(inSecondTree).nodes[borders.first], inOrient);
        clueSubtrees(borderNodes[1], getNode(inSecondTree).nodes[borders.second], inOrient);
    }
}


template <typename NODE_DATA>
QuadTree<NODE_DATA>::QuadTree(unsigned int inResolution): mResolution(inResolution){
    if(inResolution > 8) throw std::runtime_error("Can't create quad tree with such resolution. Too big.");
    else if(inResolution < 1) throw std::runtime_error("Too small resolution to build quad tree");

    size_t size = 0;
    unsigned int nextNum = 1;
    for(size_t i = 0; i < inResolution; ++i){
        size += nextNum;
        nextNum <<= 2; // 1 4 16 64 ...
    }

    mNodes.resize(size);

    //for 1 level we have root only
    if(inResolution == 1){
        mNodes[0].nodes[0] = mNodes[0].nodes[1] = mNodes[0].nodes[2] = mNodes[0].nodes[3] = 0;
        mNodes[0].parent = 0;
        mNodes[0].neighbours[0] = mNodes[0].neighbours[1] = mNodes[0].neighbours[2] = mNodes[0].neighbours[3] = 0;

        return;
    }

    size_t stride = (size - 1) / 4;

    //recursion here (tree is not very big so it's OK)
    if(inResolution == 2){
        mNodes[0].nodes[0] = 1;
        mNodes[0].nodes[1] = 2;
        mNodes[0].nodes[2] = 3;
        mNodes[0].nodes[3] = 4;
        mNodes[0].parent = 0;
        for(int i = 1; i < 5; ++i){
            mNodes[i].nodes[0] = mNodes[i].nodes[1] = mNodes[i].nodes[2] = mNodes[i].nodes[3] = 0;
            mNodes[i].parent = 0;
        }
    }
    else{
        //fill root
        mNodes[0].nodes[0] = 1;
        mNodes[0].nodes[1] = 1 + stride;
        mNodes[0].nodes[2] = 1 + 2 * stride;
        mNodes[0].nodes[3] = 1 + 3 * stride;
        mNodes[0].parent = 0;
        //create subtree
        QuadTree tree(inResolution - 1);

        //copy and fix subtrees
        for(size_t i = 0; i < 4; ++i){
            for(size_t j = 0; j < stride; ++j){
                mNodes[1 + i * stride + j] = tree.mNodes[j];
                if(mNodes[1 + i * stride + j].nodes[0] != 0) mNodes[1 + i * stride + j].nodes[0] += (1 + i * stride);
                if(mNodes[1 + i * stride + j].nodes[1] != 0) mNodes[1 + i * stride + j].nodes[1] += (1 + i * stride);
                if(mNodes[1 + i * stride + j].nodes[2] != 0) mNodes[1 + i * stride + j].nodes[2] += (1 + i * stride);
                if(mNodes[1 + i * stride + j].nodes[3] != 0) mNodes[1 + i * stride + j].nodes[3] += (1 + i * stride);
               
                if(mNodes[1 + i * stride + j].neighbours[0] != 0) mNodes[1 + i * stride + j].neighbours[0] += (1 + i * stride);
                if(mNodes[1 + i * stride + j].neighbours[1] != 0) mNodes[1 + i * stride + j].neighbours[1] += (1 + i * stride);
                if(mNodes[1 + i * stride + j].neighbours[2] != 0) mNodes[1 + i * stride + j].neighbours[2] += (1 + i * stride);
                if(mNodes[1 + i * stride + j].neighbours[3] != 0) mNodes[1 + i * stride + j].neighbours[3] += (1 + i * stride);
               
                mNodes[1 + i * stride + j].parent += (1 + i * stride);
            }
            mNodes[1 + i * stride].parent = 0;
        }
    }

    //now we should fill 'neighbours' field
    //root has no neighbours
    mNodes[0].neighbours[0] = mNodes[0].neighbours[1] = mNodes[0].neighbours[2] = mNodes[0].neighbours[3] = 0;

    clueSubtrees(1, 1 + stride, SOUTH);
    clueSubtrees(1, 1 + 2 * stride, WEST);
    clueSubtrees(1 + stride, 1 + 3 * stride, WEST);
    clueSubtrees(1 + 2 * stride, 1 + 3 * stride, SOUTH);
}

template<typename NODE_DATA>
typename QuadTree<NODE_DATA>::Node& QuadTree<NODE_DATA>::getNode(unsigned short inIndex){
    return mNodes.at(inIndex);
}

template<typename NODE_DATA>
const typename QuadTree<NODE_DATA>::Node& QuadTree<NODE_DATA>::getNode(unsigned short inIndex) const{
    return mNodes.at(inIndex);
}

template<typename NODE_DATA>
unsigned short QuadTree<NODE_DATA>::getNodeID(const std::vector<int>& inStack) const{
    unsigned short result = 0;
    for(size_t i = 0; i < inStack.size(); ++i){
        assert(inStack[i] < 4 && inStack[i] >= 0);
        result = getNode(result).nodes[inStack[i]];
    }
    return result;
}

template<typename NODE_DATA>
typename QuadTree<NODE_DATA>::Node& QuadTree<NODE_DATA>::getRoot(){
    return mNodes.at(0);
}

template<typename NODE_DATA>
const typename QuadTree<NODE_DATA>::Node& QuadTree<NODE_DATA>::getRoot() const{
    return mNodes.at(0);
}

///helper function (for internal use only)
template<typename NODE_DATA>
void doApplyToLevel(QuadTree<NODE_DATA>& tree, 
                    typename QuadTree<NODE_DATA>::Node& node, 
                    size_t currentLevel, 
                    size_t level, 
                    typename QuadTree<NODE_DATA>::NodeFunctor& func){

    if(currentLevel < level){
        if(!(node.nodes[0] && node.nodes[1] && node.nodes[2] && node.nodes[3])){
            assert(false && "Tree has not such a big level of detail.");
        }
        //call for childs
        for(int i = 0; i < 4; ++i)
            doApplyToLevel(tree, tree.getNode(node.nodes[i]), currentLevel + 1, level, func);
    }
    else if(currentLevel == level){
        //run functor
        func(tree, node);
    }
    else{
        assert(false && "Error while building index LODs.");
    }
}

///applies user's function to all nodes of specified level in quad tree
template<typename NODE_DATA>
void applyToLevel(QuadTree<NODE_DATA>& inTree, size_t inLevel, typename QuadTree<NODE_DATA>::NodeFunctor& inFunc){
    doApplyToLevel(inTree, inTree.getRoot(), 0, inLevel, inFunc);
}



} //common namespace

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
