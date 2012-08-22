//CGContext.cpp

#include "rendering/CGContext.hpp"
#include <Cg/cg.h>
#include <cassert>

using hydra::rendering::CGContext;

CGContext::CGContext(){
    context = cgCreateContext();

    checkForErrors();

    assert(context);
}

CGContext::~CGContext(){
    if(isValid()){
        cgDestroyContext(context);
    }
}

bool CGContext::isValid() const{
    if(context){
        return cgIsContext(context);
    }
    else return false;
}

void CGContext::setDefferedParameterSettingMode(bool enable){
    if(enable){
        cgSetParameterSettingMode(context, CG_DEFERRED_PARAMETER_SETTING);
    }
    else{
        cgSetParameterSettingMode(context, CG_IMMEDIATE_PARAMETER_SETTING);
    }

    checkForErrors();
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
