//CGProgram.cpp

#include "rendering/CGProgram.hpp"
#include <Cg/cg.h>
#include "rendering/CGError.hpp"

#include <cassert>

using hydra::rendering::CGContext;
using hydra::rendering::CGProfile;
using hydra::rendering::CGParameter;
using hydra::rendering::CGProgram;

//utility function to actually create program
static CGprogram createProgram(CGContext& context, CGProfile& profile, const std::string& sourceCode){
    //todo: some checks?

    CGprogram program = cgCreateProgram(context.getRepresentation(), CG_SOURCE, sourceCode.c_str(), profile.getRepresentation(), 0, 0);

    context.checkForErrors();

    return program;
}


///creates program
CGProgram::CGProgram(CGContext& context, CGProfile& profile, const std::string& sourceCode){
    program = createProgram(context, profile, sourceCode);
}

///creates program using data from input stream (utility function)
CGProgram::CGProgram(CGContext& context, CGProfile& profile, std::istream& stream){
    if(!stream.good()){
        throw std::runtime_error("Could not read program source code from stream.");
    }

    //read data from stream
    std::string sourceCode;
    sourceCode.reserve(10240); //reserve 10KB
    
    //read all the data till EOF
    std::getline(stream, sourceCode, '\0');

    //create program
    program = createProgram(context, profile, sourceCode);
}

///destroys the program (all references become invalid)
CGProgram::~CGProgram(){
    if(program && cgIsProgram(program)){
        cgDestroyProgram(program);
    }
}

void CGProgram::load(){
    if(program && cgIsProgram(program) && !cgGLIsProgramLoaded(program)){
        cgGLLoadProgram(program);
    }
}

void CGProgram::bind(){
    load();
    cgGLBindProgram(program);
}


void CGProgram::updateParameters(){
    assert(program && cgIsProgram(program));

    cgUpdateProgramParameters(program);
}

///returns native program representation
CGprogram CGProgram::getRepresentation() const{
    return program;
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
