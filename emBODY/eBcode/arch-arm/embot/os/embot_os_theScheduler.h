/*
 * Copyright (C) 2017 iCub Facility - Istituto Italiano di Tecnologia
 * Author:  Marco Accame
 * email:   marco.accame@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

// - include guard ----------------------------------------------------------------------------------------------------

#ifndef _EMBOT_OS_THESCHEDULER_H_
#define _EMBOT_OS_THESCHEDULER_H_

#include "embot_core.h"
#include "embot_os_common.h"
#include "embot_os_Thread.h"

#include <memory>

namespace embot { namespace os {
    
    // the method theScheduler::start(cfg) does the following:
    // if already started or if the cfg is not valid: it stays in a forever loop otherwise:
    // 1. starts the OS with cfg.timing.ticktime, sets the idle task as specified by cfg.behaviour.idleconfig
    // 2. executes at maximum priority the init task as specified by cfg.behaviour.initconfig
    // after the function cfg.behaviour.initconfig.startup(cfg.behaviour.initconfig.param) returns, the init task
    // is deleted and the OS schedules any task started inside startup() or the idle task, depending on their
    // priority and ready/blocked status. the OS executte the ready task w/ highest priority.    
    // the method theScheduler::start(cfg) does does not return

    class theScheduler
    {
    public:
        static theScheduler& getInstance();
        
    public:
                            
        struct Timing
        {
            embot::core::relTime ticktime {embot::core::time1millisec}; // the resolution of the scheduler
            
            Timing(core::relTime t = embot::core::time1millisec) : ticktime(t) {}
            void clear() { ticktime = embot::core::time1millisec; }
            bool isvalid() const { if((0 == ticktime)) { return false; } else { return true; } }
        };
        
        struct Behaviour
        {
            embot::os::InitThread::Config initconfig {};
            embot::os::IdleThread::Config idleconfig {};
            embot::core::Callback onOSerror {};
            
            Behaviour() = default;
            Behaviour(const embot::os::InitThread::Config &ini, const embot::os::IdleThread::Config &idl, const embot::core::Callback &err) : initconfig(ini), idleconfig(idl), onOSerror(err) {}            
            bool isvalid() const { if((false == initconfig.isvalid()) || (false == idleconfig.isvalid())) { return false; } else { return true; } }
        };
        
        struct Config
        {
            Timing timing {};
            Behaviour behaviour {};  
            
            Config() = default;
            Config(const Timing &tim, const Behaviour &beh) : timing(tim), behaviour(beh) {}
            bool isvalid() const { if((false == timing.isvalid()) || (false == behaviour.isvalid())) { return false; } else { return true; } }
        }; 
        
        
        [[noreturn]] void start(const Config &config);  
        
        bool started() const;
        
        embot::core::relTime ticktime() const;
        
        Thread * scheduled() const;    

        const char * getOSerror(int &errorcode) const;
        
    private:
        theScheduler();  
        ~theScheduler(); 

    private:    
        struct Impl;
        std::unique_ptr<Impl> pImpl;   
    };   

}} // namespace embot { namespace os {


#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------
