//OBJModelLoader.cpp

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

#include "loading/OBJModelLoader.hpp"
#include "data/Model.hpp"
#include "data/Image.hpp"
#include "data/Mesh.hpp"
#include "data/Material.hpp"
#include "loading/Manager.hpp"
#include "math/Point.hpp"
#include "data/Vertex.hpp"
#include "math/Vector3D.hpp"

#include <string>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <cctype>

#include <boost/foreach.hpp>

//'using namespace' is ugly. Don't touch those usings.
using hydra::loading::OBJModelLoader;
using hydra::data::Model;
using hydra::data::ModelPtr;
using hydra::data::Mesh;
using hydra::data::MeshPtr;
using hydra::data::Material;
using hydra::data::MaterialPtr;
using hydra::data::Color;
using hydra::data::Image;
using hydra::data::ImagePtr;
using hydra::loading::Manager;
using hydra::math::Point;
using hydra::data::Vertex;
using hydra::math::Vector3D;

struct OBJModelLoader::OBJImpl{
   
    //eats and ignore all spacers and comments
    inline void findNextToken(std::istream& inStream){
        char c = '\0';
        while(inStream.good()){
            inStream.get(c);
            //ignore comments
            if(c == '#'){
                inStream.ignore(10000, '\n');
            }
            else if(isspace(c)) continue;
            else break;
        }
        if(inStream.good()) inStream.putback(c);
    }

    inline void eatSpaces(std::istream& inStream){
        char c = '\0';
        while(inStream.good()){
            inStream.get(c);
            if(c != ' ' && c != '\t'){
                inStream.putback(c);
                break;
            }
        }
    }

    inline std::string getNextToken(std::istream& inStream){
        const int BUFFER_SIZE = 20;
        char buff[BUFFER_SIZE];
        std::string result;
        int index = 0;
        while(inStream.good()){
            inStream.get(buff[index]);

            //if separator found exit
            if(isspace(buff[index])){
                result.append(buff, index);
                inStream.putback(buff[index]);
                break;
            }

            ++index;
            if(index >= BUFFER_SIZE){
                result.append(buff, BUFFER_SIZE);
                index = 0;
            }
        }

        return result;
    }

    //eat current line (till \n)
    inline void nextLine(std::istream& inStream){
        inStream.ignore(10000, '\n');
        findNextToken(inStream);
    }

    //splits polygon with specified number of vertices into triangles
    //returns indices of polygon's vertices to be used in new triangle faces
    //(it returns 3 * (numOfVertices - 2) indices)
    std::vector<unsigned int> splitFace(unsigned int numOfVertices){
        if(numOfVertices < 3) return std::vector<unsigned int>();

        std::vector<unsigned int> indices;
        indices.reserve(3*(numOfVertices - 2));

        //TODO: think about implementation of 3dsmax-like algorithm
        //where the biggest triangle is not splitted
    

        for(size_t i = 0; i < (numOfVertices - 2); ++i){
            unsigned int i1, i2, i3;
            if(i % 2 == 0){
                i1 = i/2;
                i2 = i/2 + 1;
                i3 = (numOfVertices - i/2 - 1);
            }
            else{
                i1 = i/2 + 1;
                i2 = (numOfVertices - i/2 - 2);
                i3 = (numOfVertices - i/2 - 1);
            }
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }

        return indices;
    }

    //Load materials from MTL source stream. Returns Model object with materials
    //TODO: think about splitting this function.
    ModelPtr loadMtl(std::istream& source, Manager& inManager){
        if(!source.good())
            throw std::runtime_error("OBJModelLoader: stream is in bad condition. Can't read data.");

        //do not eat spaces (we will eat them manually)
        source.unsetf(std::istream::skipws);

        ModelPtr model(new Model());
        MaterialPtr lastMaterial;
        
        findNextToken(source);
        while(source.good()){
            std::string command = getNextToken(source);
            eatSpaces(source);
            if(!source.good()) break;
            
            if(command == "newmtl"){ //new material command
                std::string matName = getNextToken(source); //get next word(name)
                //if already taken data for some material
                if(lastMaterial){
                    //add new mesh
                    model->mMeshes.push_back(MeshPtr(new Mesh()));
                    //set current material to new mesh
                    model->mMeshes.back()->mMaterial = lastMaterial;
                    lastMaterial.reset();
                }
                lastMaterial = MaterialPtr(new Material());
                lastMaterial->mName = matName;
            }
            else if(!lastMaterial){ //no newmtl directive before first commands
                std::cerr << "OBJModelLoader: ERROR: newmtl directive should stand before any other directives" << std::endl;
            }
            else if(command[0] == 'K' && command.size() == 2){ //Ka, Kd or Ks
                float r = 0.0f, g = -1.0, b;
                source >> r;
                eatSpaces(source);
                if(isspace(source.peek())) g = b = r;
                else{
                    source >> g;
                    eatSpaces(source);
                    source >> b;
                }
                Color newColor(r, g, b);
                switch(command[1]){
                    case 'a': //Ka
                        lastMaterial->mAmbient = newColor;
                        break;
                    case 'd': //Kd
                        lastMaterial->mDiffuse = newColor;
                        break;
                    case 's': //Ks
                        lastMaterial->mSpecular = newColor;
                        break;
                    case 'e': //Ke
                        lastMaterial->mEmissive = newColor;
                    default:
                        //error
                        //throw std::runtime_error("OBJModelLoader: unknown directive in mtl file");
                        break;
                }
            }
            //TODO: delete copy-paste code (textures must be handled in one common way)
            else if(command.find("map_K") != std::string::npos){ //map_Kd, map_Ka, map_Ks
                //we ignore difference between these textures and set them as the only texture
                //if current material alredy has texture ignore new one
                if(lastMaterial->mTexture != Material::ImageId()){
                    nextLine(source);
                    continue;
                }
                std::string imagePath = getNextToken(source); //relative path (not absolute)
                //TODO: texture arguments can be here!
                std::string fullImageId = inManager.getFullId(imagePath);
                //if image is new (hasn't been loaded so far)
                if(!model->getImage(fullImageId)) model->addImage(fullImageId, inManager.load<Image>(imagePath));
                lastMaterial->mTexture = fullImageId;
            }

            else if(command.find("bump") != std::string::npos){ //bump map (bump or map_bump)
                //if current material already has bump map ignore new one
                if(lastMaterial->mBump != Material::ImageId()){
                    nextLine(source);
                    continue;
                }
                std::string imagePath = getNextToken(source); //relative path (not absolute)
                //TODO: texture arguments can be here!
                std::string fullImageId = inManager.getFullId(imagePath);
                //if image is new (hasn't been loaded so far)
                if(!model->getImage(fullImageId)) model->addImage(fullImageId, inManager.load<Image>(imagePath));
                lastMaterial->mBump = fullImageId;
            }

            else if(command.find("refl") != std::string::npos){ //reflection map (refl or map_refl)
                //if current material already has bump map ignore new one
                if(lastMaterial->mRefl != Material::ImageId()){
                    nextLine(source);
                    continue;
                }
                std::string imagePath = getNextToken(source); //relative path (not absolute)
                //TODO: texture arguments can be here!
                std::string fullImageId = inManager.getFullId(imagePath);
                //if image is new (hasn't been loaded so far)
                if(!model->getImage(fullImageId)) model->addImage(fullImageId, inManager.load<Image>(imagePath));
                lastMaterial->mRefl = fullImageId;
            }

            else if(command == "d"){ //dissolve factor
                float dissolveFactor = 100.0f;
                std::string nextToken = getNextToken(source);
                //we ignore '-halo' argument, but we should peek it
                if(nextToken != "-halo"){
                    std::istringstream tokenStream(nextToken);
                    tokenStream >> dissolveFactor;
                }
                else{
                    eatSpaces(source);
                    source >> dissolveFactor;
                }
                lastMaterial->setOpacity(dissolveFactor);
            }

            else if(command == "Ns"){ //specular exponent
                float specExponent;
                source >> specExponent;
                lastMaterial->mSpecularExponent = specExponent;
            }
            
            else if(command == "Ni"){ //optical density (index of refraction)
                float indexOfRefraction;
                source >> indexOfRefraction;
                lastMaterial->mOpticalDensity = indexOfRefraction;
            }

            else if(command == "illum"){ //illumination model
                //float illumNum;
                //source >> illumNum;
                //we ignore it for now
            }
            else if(command == "Tf"){ //transmission filter
                float r = 0.0f, g, b;
                source >> r;
                eatSpaces(source);
                if(isspace(source.peek())) g = b = r;
                else{
                    source >> g;
                    eatSpaces(source);
                    source >> b;
                }
                
                Color newColor(r, g, b);
                lastMaterial->mTransmissionFilter = newColor;
            }
            else if(command == "Tr"){ //I don't know what is 'Tr' exactly
                                      //Some people say it is some kind of alpha
                                      //who knows...
                                      //Ignore it by now.
            }

            //TODO: add some new functionality from MTL file spec.
            else{
                std::cerr << "OBJModelLoader: unknown special word in mtl file - \"" << command << "\". Skipping line..." << std::endl;
            }

            //go to next line
            nextLine(source);
        
        }

        //add last material before return
        if(lastMaterial){
            model->mMeshes.push_back(MeshPtr(new Mesh()));
            model->mMeshes.back()->mMaterial = lastMaterial;
            lastMaterial.reset();
        }
        return model;
    }

    //simple function to get right mesh from model by material name
    MeshPtr getMeshByMaterialName(ModelPtr inModel, const std::string& inMaterialName){
        BOOST_FOREACH(MeshPtr nextMesh, inModel->mMeshes){
            if(nextMesh && nextMesh->mMaterial){
                if(nextMesh->mMaterial->mName == inMaterialName) return nextMesh;
            }
        }
        return MeshPtr();
    }

    //TODO: function is large. Think about splitting it.
    //load OBJ file and run loadings of its MTL files
    ModelPtr loadObj(std::istream& source, Manager& inManager){
        if(!source.good())
            throw std::runtime_error("OBJModelLoader: stream is in bad condition. Can't read data.");

        //do not eat spaces (we will eat them by manually)
        source.unsetf(std::istream::skipws);

        ModelPtr model(new Model());
        MeshPtr workingMesh;
        bool wereNormals = false;

        //temporary containers
        std::vector<Vector3D> temp_normals;
        std::vector<Point> temp_coords, temp_texCoords;

        findNextToken(source);

        while(source.good()){
            std::string command = getNextToken(source);

            if(!source.good()) break;

            eatSpaces(source);
            if(command == "v"){ //vertex
                Point newPoint;
                source >> newPoint.x;
                eatSpaces(source);
                source >> newPoint.y;
                eatSpaces(source);
                source >> newPoint.z;
                temp_coords.push_back(newPoint);
            }
            else if(command == "vt"){ //texture coords
                Point newPoint;
                source >> newPoint.x;
                eatSpaces(source);
                source >> newPoint.y;
                temp_texCoords.push_back(newPoint);
            }
            else if(command == "vn"){ //normal vectors
                float normal[3];
                source >> normal[0];
                eatSpaces(source);
                source >> normal[1];
                eatSpaces(source);
                source >> normal[2];
                temp_normals.push_back(Vector3D(normal));
                wereNormals = true;
            }
            else if(command == "f"){ //face
                //vectors to contain indices for different vertices of polygonal face
                //the numeration starts with 1!
                //(must be converted before passing to model object)

                if(!workingMesh){
                    workingMesh = MeshPtr(new Mesh());
                    workingMesh->mMaterial = MaterialPtr(new Material());
                    workingMesh->mMaterial->mName = "OBJLoader_default_material";
                    model->mMeshes.push_back(workingMesh);
                }
                std::vector<int> coords;
                std::vector<int> texCoords;
                std::vector<int> normals;
                //small optimization
                coords.reserve(3);
                texCoords.reserve(3);
                normals.reserve(3);
                //face may not have texture coordinates or normal vectors
                bool areThereTexCoords = false;
                bool areThereNormals = false;

                //eat all triplets (or duplets or singles)
                while(source.good()){
                    eatSpaces(source);
                    if(isspace(source.peek()) || source.peek() == '#') break;
                    int vert = 0;
                    source >> vert;
                    coords.push_back(vert);
                    eatSpaces(source);
                    if(isspace(source.peek()) || source.peek() == '#') break;
                    
                    if(source.peek() == '/'){ //if there is more than vertex index
                        source.get();
                        if(source.peek() != '/'){ //if there is texture coord index
                            int texCoord = 0;
                            //eatSpaces(source);
                            source >> texCoord;
                            texCoords.push_back(texCoord);
                            areThereTexCoords = true;
                        }
                        if(source.peek() == '/'){ //if there is normal index
                            source.get();
                            int normal = 0;
                            //eatSpaces(source);
                            source >> normal;
                            normals.push_back(normal);
                            areThereNormals = true;
                        }
                    }
                } //end of while

                //number of normals, vertex and texture coordinates must be same
                //so we use only 1 size variable
                //however we'll check number of texture coords and normals
                size_t size = coords.size();
                //change relative (negative) indices to absolute
                size_t numVertices = temp_coords.size();
                size_t numTexCoords = temp_texCoords.size();
                size_t numNormals = temp_normals.size();

                //check and fix coords
                for(size_t i = 0; i < size; ++i){
                    if(coords[i] > 0){
                        --coords[i];//convert enumeration
                    }
                    else if(coords[i] < 0){
                        coords[i] = numVertices + coords[i];//convert
                    }
                    else { //error while parsing (or zero index in file)
                        //we should cut the end of vector
                        coords.resize(i);
                        size = i;
                    }
                }

                if(size < 3){ //error
                    std::cerr << "OBJModelLoader: can't add face with " << size << " vertices." << std::endl;
                    nextLine(source);
                    continue;
                }
                
                //check number of texture coords (should be 'size' or 0)
                //and convert relative indices to absolute
                if(areThereTexCoords){
                    if(texCoords.size() != size){ //error
                        std::cerr << "OBJModelLoader: wrong format" << std::endl;
                        nextLine(source);
                        continue;
                    }
                    for(size_t i = 0; i < size; ++i){
                        //convert relative indices to absolute
                        if(texCoords[i] < 0){
                            texCoords[i] = numTexCoords + texCoords[i];
                        }
                        else if(texCoords[i] == 0){ //error while parsing (or zero index in file)
                            //error                            
                            std::cerr << "OBJModelLoader: wrong format" << std::endl;
                        }
                        else{
                            --texCoords[i];
                        }
                    }
                }

                //check number of normals (should be 'size' or 0)
                //and convert relative indices to absolute
                if(areThereNormals){
                    if(normals.size() != size){ //error
                        std::cerr << "OBJModelLoader: wrong format" << std::endl;
                        nextLine(source);
                        continue;
                    }
                    for(size_t i = 0; i < size; ++i){
                        //convert relative indices
                        if(normals[i] < 0){
                            normals[i] = numNormals + normals[i] + 1;
                        }
                        else if(normals[i] == 0){ //error while parsing (or zero index in file)
                            std::cerr << "OBJModelLoader: wrong format" << std::endl;
                            //error                            
                        }
                        else{
                            --normals[i];
                        }
                    }

                }

                //add vertices and face (may produce redundant vertices)
                for(size_t i = 0; i < coords.size(); ++i){
                    Vertex vert;
                    if(temp_coords.size() < static_cast<unsigned int>(coords[i])) throw std::runtime_error("OBJModelLoader: wrong vertex's index in OBJ file");
                    vert.mCoord = temp_coords[coords[i]];
                    if(areThereTexCoords){
                        if(temp_texCoords.size() < static_cast<unsigned int>(texCoords[i])) throw std::runtime_error("OBJModelLoader: wrong texture coordinate's index in OBJ file");
                        vert.mTexCoord = temp_texCoords[texCoords[i]];
                    }
                    if(areThereNormals){
                        if(temp_normals.size() < static_cast<unsigned int>(normals[i])) throw std::runtime_error("OBJModelLoader: wrong normal's index in OBJ file");
                        vert.mNormal = temp_normals[normals[i]];
                    }
                    workingMesh->addVertex(vert);
                }
                //split non-triangle faces to triangles
                if(coords.size() > 3){
                    std::vector<unsigned int> indices = splitFace(coords.size());
                    size_t startIndex = workingMesh->getVertexNum() - coords.size();
                    for(size_t i = 0; i < indices.size()/3; ++i){
                        workingMesh->mIndices.push_back(startIndex + indices[3 * i]);
                        workingMesh->mIndices.push_back(startIndex + indices[3 * i + 1]);
                        workingMesh->mIndices.push_back(startIndex + indices[3 * i + 2]);
                    }
                }
                else{ //if already triangle face
                    size_t numOfVert = workingMesh->getVertexNum();
                    workingMesh->mIndices.push_back(numOfVert - 3);
                    workingMesh->mIndices.push_back(numOfVert - 2);
                    workingMesh->mIndices.push_back(numOfVert - 1);
                }
                workingMesh->mMode = Mesh::TRIANGLES;
            }
            else if(command == "usemtl"){ //use material
                std::string materialName = getNextToken(source);
                workingMesh = getMeshByMaterialName(model, materialName);
                if(!workingMesh) throw std::runtime_error("OBJModelLoader: error, unknown material name found :'" + materialName + "'");
            }
            else if(command == "g"){ //group
                //std::string groupName = "";
                //std::getline(source, groupName, ' ');
                //TODO!!!!!!!!!
            }
            else if(command == "s"){ //smooth group
                //int smoothGroup = 0;
                //source >> smoothGroup;
                //no smooth groups now!
            }
            else if(command == "mtllib"){ //load mtl
                std::string path = getNextToken(source);

                ModelPtr materials = inManager.load<Model>(path);
                BOOST_FOREACH(MeshPtr nextMesh, materials->mMeshes){
                    //if no such mesh/material
                    if(!getMeshByMaterialName(model, nextMesh->mMaterial->mName)){
                        //add mesh/material
                        model->mMeshes.push_back(MeshPtr(new Mesh()));
                        //we do not copy it (shared ptr will do all the job after
                        //'materials' model will be destroyed)
                        model->mMeshes.back()->mMaterial = nextMesh->mMaterial;
                    }
                }
                Model::ImageCont& images = materials->mImages;
                BOOST_FOREACH(Model::ImageCont::value_type nextPair, images)
                    model->addImage(nextPair.first, nextPair.second);
            }
            else if(command == "o"){ //object name
                std::string objectName = getNextToken(source);
                model->mName = objectName;
            }
            else{
                std::cerr << "OBJModelLoader: Unknown command found: '" + std::string(command) + "' skipping line" << std::endl;
            }

            nextLine(source);
        }

        //drop all empty meshes (they were created for unused materials)
        model->dropEmptyMeshes();

        //generate normals if no any
        if(!wereNormals){
            BOOST_FOREACH(MeshPtr nextMesh, model->mMeshes)
                nextMesh->generateNormals();
        }

        //we should delete redundant vertices as they may appear after load
        //or they could be in file
        BOOST_FOREACH(MeshPtr nextMesh, model->mMeshes)
            nextMesh->deleteRedundantVertices();

        //we can optimize meshes for video-card here, but we don't do that
        //user may do that by using  rendering::MeshOptimizer class

        return model;
    }
};

OBJModelLoader::OBJModelLoader(): mImpl(new OBJModelLoader::OBJImpl()){

}

OBJModelLoader::~OBJModelLoader(){

}

void OBJModelLoader::init(){

}

bool OBJModelLoader::isInited() const{
    return true;
}

ModelPtr OBJModelLoader::doLoad(std::istream& source, Manager& inManager, const std::string& inModelType){
    assert(mImpl);
    //get model's type from input string
    std::string sourceDataType = inModelType;
    //transform input string to lower case
    std::transform(sourceDataType.begin(), sourceDataType.end(), 
                   sourceDataType.begin(), tolower);

    if(inModelType == "mtl") return mImpl->loadMtl(source, inManager);
    else if(inModelType == "obj") return mImpl->loadObj(source, inManager);
    else throw std::runtime_error("OBJModelLoader: error, can't load data of specified type");
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
