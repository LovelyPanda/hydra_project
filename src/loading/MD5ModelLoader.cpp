//MD5ModelLoader.cpp

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

#include "loading/MD5ModelLoader.hpp"
#include "MD5Common.hpp"
#include "data/Model.hpp"
#include "data/Mesh.hpp"
#include "data/Material.hpp"
#include "loading/Manager.hpp"
#include "math/Vector3D.hpp"
#include "math/Quat.hpp"
#include "data/Bone.hpp"
#include "data/Skeleton.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <cassert>
#include <stdexcept>

//'using namespace' is ugly. Don't touch those usings.
using hydra::loading::MD5ModelLoader;
using hydra::data::Model;
using hydra::data::ModelPtr;
using hydra::data::Mesh;
using hydra::data::MeshPtr;
using hydra::data::Material;
using hydra::data::MaterialPtr;
using hydra::loading::Manager;
using hydra::data::Bone;
using hydra::data::Skeleton;
using hydra::data::SkeletonPtr;
using hydra::math::Vector3D;
using hydra::math::Quat;

using namespace hydra::MD5Common;

struct MD5ModelLoader::MD5Impl{

    struct MD5Weight{
        int joint;
        float weight;
        Vector3D pos;
    };


    ModelPtr loadMesh(std::istream& inSource, Manager& inManager){
        if(!inSource.good()) fatalError("stream is in bad condition. Can't read data.");
        
        //first we should read the header data to check if
        //it is the MD5 we are loading
        checkAndEatHeader(inSource);
        
        size_t numJoints; 
        {
            int tempNumJoints = getNamedValue(inSource, "numJoints");
            if(tempNumJoints < 0) fatalError("'numJoins' has wrong value.");
            numJoints = static_cast<size_t>(tempNumJoints); 
        }

        size_t numMeshes;
        {
            int tempNumMeshes = getNamedValue(inSource, "numMeshes");
            if(tempNumMeshes < 0) fatalError("'numMeshes' has wrong value.");
            numMeshes = static_cast<size_t>(tempNumMeshes);
        }

        ModelPtr model(new Model());
        model->mMeshes.reserve(numMeshes);
        model->mBindSkel = SkeletonPtr(new Skeleton());
        model->mBindSkel->mBones.reserve(numJoints);

        //now we can read joint data
        std::string token;
        findNextToken(inSource);
        std::getline(inSource, token, ' ');
        if(token != "joints") fatalError("'joints' wanted.");
        //eat {
        eatChar(inSource, '{');
        //{ is eatten, we can load joints now

        for(size_t i = 0; i < numJoints; ++i){
            // '"' must be here
            eatChar(inSource, '"');
            Bone nextBone;
            //read name
            std::getline(inSource, nextBone.mName, '"');
            findNextToken(inSource);
            //read parent's id
            inSource >> nextBone.mParent;

            //it must be '('
            eatChar(inSource, '(');

            findNextToken(inSource);
            //read position vector data
            float vec_x, vec_y, vec_z;
            inSource >> vec_x >> vec_y >> vec_z;
            nextBone.mPos = Vector3D(vec_x, vec_y, vec_z);
            
            //')' must be here
            eatChar(inSource, ')');

            //we read unit quaternion data now (orientation)
            //'(' must be here
            eatChar(inSource, '(');
            //reuse same variables :)
            inSource >> vec_x >> vec_y >> vec_z;
            nextBone.mOrient = buildUnitQuat(vec_x, vec_y, vec_z);
            
            //')' must be here
            eatChar(inSource, ')');

            //push back next bone
            model->mBindSkel->mBones.push_back(nextBone);
        }
        //'}' must be here
        eatChar(inSource, '}');

        //now we must read meshes

        for(size_t i = 0; i < numMeshes; ++i){
            findNextToken(inSource);
            std::getline(inSource, token, ' ');
            if(token != "mesh") fatalError("'mesh' wanted.");
            //eat {
            eatChar(inSource, '{');

            MeshPtr newMesh(new Mesh());
            //we can load mesh's content now
            //first 'shader'
            //for now we just save shader's string to mesh's material's name
            //user must interpret it as he like/can.
            //TODO: think about it!
            newMesh->mMaterial = MaterialPtr(new Material());
            newMesh->mMaterial->mName = getNamedString(inSource, "shader");

            //numverts
            size_t numVerts = getNamedValue(inSource, "numverts");
            newMesh->mVertices.resize(numVerts);
            newMesh->mBoneWeights.resize(numVerts);
            newMesh->mBones.resize(numVerts);


            //first value is start, second value is number of weights
            std::vector<std::pair<int, int> > MD5WeightIndices;
            MD5WeightIndices.resize(numVerts);

            //read vertices
            for(size_t i = 0; i < numVerts; ++i){
                findNextToken(inSource);
                //'vert' must be here
                std::getline(inSource, token, ' ');
                if(token != "vert") fatalError("'vert' wanted.");
                
                findNextToken(inSource);
                //read vertex' index
                int vertIndex;
                inSource >> vertIndex;
                if(vertIndex < 0 || vertIndex >= static_cast<int>(numVerts)) 
                    fatalError("wrong vertex' index found!");

                eatChar(inSource, '(');

                //read texture coords
                inSource >> newMesh->mVertices[vertIndex].mTexCoord.x >> newMesh->mVertices[vertIndex].mTexCoord.y;
                //flip V coordinate
                newMesh->mVertices[vertIndex].mTexCoord.x = 1.0f - newMesh->mVertices[vertIndex].mTexCoord.x; 

                //eat ')'             
                eatChar(inSource, ')');

                //get weights
                findNextToken(inSource);
                inSource >> MD5WeightIndices[vertIndex].first >> MD5WeightIndices[vertIndex].second;
                if(MD5WeightIndices[vertIndex].second > Mesh::MAX_BONES_PER_VERTEX) 
                    fatalError("weight number is too big. Such quantity of bones per vertex is not supported.");
            } //end read vertices

            //read triangles (indices)
            size_t numTriangles = getNamedValue(inSource, "numtris");
            newMesh->mIndices.resize(3*numTriangles);
            
            //read each triangle
            for(size_t i = 0; i < numTriangles; ++i){
                //'tri' must be here
                findNextToken(inSource);
                std::getline(inSource, token, ' ');
                if(token != "tri") fatalError("'tri' wanted.");

                int triIndex;
                inSource >> triIndex;
                if(triIndex < 0 || triIndex >= static_cast<int>(numTriangles)) 
                    fatalError("wrong triangle's index found!");
                findNextToken(inSource);
                //read indices
                inSource >> newMesh->mIndices[3*triIndex + 2] >> newMesh->mIndices[3*triIndex + 1] >> newMesh->mIndices[3*triIndex]; 
            }//end read triangles

            size_t numWeights = getNamedValue(inSource, "numweights");

            //temporary container for weights
            std::vector<MD5Weight> weights;
            weights.resize(numWeights);

            //read weights
            for(size_t i = 0; i < numWeights; ++i){
                //'weight' must be here
                findNextToken(inSource);
                std::getline(inSource, token, ' ');
                if(token != "weight") fatalError("'weight' wanted.");

                int weightIndex;
                inSource >> weightIndex;
                if(weightIndex < 0 || weightIndex >= static_cast<int>(numWeights)) 
                    fatalError("wrong weight's index found!");
                findNextToken(inSource);

                //read weight's data
                inSource >> weights[weightIndex].joint >> weights[weightIndex].weight;

                //'(' must be here
                eatChar(inSource, '(');

                float vec_x, vec_y, vec_z;
                //read position vector
                findNextToken(inSource);
                inSource >> vec_x >> vec_y >> vec_z;
                weights[weightIndex].pos = Vector3D(vec_x, vec_y, vec_z);
                
                eatChar(inSource, ')');
            }//end read weights

            //we should convert weights to our format
            //we should also generate vertex' position (coordinate)
            for(size_t i = 0; i < numVerts; ++i){
                Vector3D vertexPos;
                for(int j = 0; j < MD5WeightIndices[i].second; ++j){
                    MD5Weight &weight = weights[MD5WeightIndices[i].first + j];
                    newMesh->mBoneWeights[i][j] = weight.weight;
                    newMesh->mBones[i][j] = weight.joint;

                    Vector3D tempPos = weight.pos;
                    //rotate using quaternion
                    model->mBindSkel->mBones[weight.joint].mOrient.rotate(tempPos);

                    vertexPos += ((model->mBindSkel->mBones[weight.joint].mPos + tempPos) * weight.weight);
                }
                newMesh->mVertices[i].mCoord.x = vertexPos.x();
                newMesh->mVertices[i].mCoord.y = vertexPos.y();
                newMesh->mVertices[i].mCoord.z = vertexPos.z();
            }

            newMesh->mMode = Mesh::TRIANGLES;
            //add loaded mesh to model
            model->mMeshes.push_back(newMesh);

            //'}' is here
            eatChar(inSource, '}');
        }//end read meshes
       
        //generate normals
        for(size_t i = 0; i < model->mMeshes.size(); ++i)
            model->mMeshes[i]->generateNormals();

        return model;
    }

};


MD5ModelLoader::MD5ModelLoader(): mImpl(new MD5ModelLoader::MD5Impl()){

}

MD5ModelLoader::~MD5ModelLoader(){

}

void MD5ModelLoader::init(){

}

bool MD5ModelLoader::isInited() const{
    return true;
}

ModelPtr MD5ModelLoader::doLoad(std::istream& inSource, Manager& inManager, const std::string& inModelType){
    assert(mImpl);
    //get model's type from input string
    std::string sourceDataType = inModelType;
    //transform input string to lower case
    std::transform(sourceDataType.begin(), sourceDataType.end(), 
                   sourceDataType.begin(), tolower);

    if(inModelType == "md5mesh") return mImpl->loadMesh(inSource, inManager);
    else throw std::runtime_error("MD5ModelLoader: error, can't load data of specified type");
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
