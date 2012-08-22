//PerformanceTimer.hpp

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


#ifndef COMMON_PERFORMANCE_TIMER_HPP__
#define COMMON_PERFORMANCE_TIMER_HPP__

/**
 * \class hydra::common::PerformanceTimer
 * \brief Simple timer to get approximation of processor time used by the program.
 *
 * PerformanceTimer measures CPU time used by your program in some period of time.
 * You can start (restart) it and get elapsed processor time
 * using special member functions. Its precision may differ
 * depending on system. However it is high enough in most cases.
 *  
 * If you need to measure absolute time not a precessor time you
 * should use common::Timer instead.
 *
 * \todo Add new functionality, change interface
 *
 * \see hydra::common::Timer
 *
 * \author A.V.Medvedev
 * \date 14.07.2010
 */

#include <ctime>

namespace hydra{

namespace common{

class PerformanceTimer{

public:

    ///saves current time (starts/restarts timer to
    ///count time from current moment)
    inline void start(){
        mStartTime = clock();
    }

    ///Returns amount of processor time elapsed from timer's start in seconds (float value).
    ///Produce undefined behaviour if timer hasn't been started yet.
    inline float getSeconds(){
        return static_cast<float>(clock() - mStartTime) / CLOCKS_PER_SEC;
    }

    ///Returns amount of processor ticks from timer's start.
    ///Produce undefined behaviour if timer hasn't been started yet.
    inline long getTicks(){
        return static_cast<long>(clock() - mStartTime);
    }

private:
    time_t mStartTime;
};

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
