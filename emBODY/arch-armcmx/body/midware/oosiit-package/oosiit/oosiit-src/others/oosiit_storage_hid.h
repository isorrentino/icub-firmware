/*
 * Copyright (C) 2012 iCub Facility - Istituto Italiano di Tecnologia
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
#ifndef _OOSIIT_STORAGE_HID_H_
#define _OOSIIT_STORAGE_HID_H_

#ifdef __cplusplus
extern "C" {
#endif


/* @file       oosiit_storage_hid.h
    @brief      This header file implements hidden interface to a module.
    @author     marco.accame@iit.it
    @date       07/27/2012
 **/


// - external dependencies --------------------------------------------------------------------------------------------

#include "stdint.h"
#include "oosiit.h"

// - declaration of extern public interface ---------------------------------------------------------------------------
 



// - declaration of externally defined variables ----------------------------------------------------------------------

// - variables which have been transformed into pointers holding memory and in theirs sizes

// 32-0
extern uint32_t*    oosiit_cfg_advtmr_ptrs;
extern uint32_t*    oosiit_cfg_advtmr;
extern uint16_t     oosiit_cfg_advtmr_size;
// 32-1
extern uint32_t*    oosiit_cfg_mutex;
extern uint16_t     oosiit_cfg_mutex_size;
// 32-2
extern uint32_t*    oosiit_cfg_semaphore;
extern uint16_t     oosiit_cfg_semaphore_size;
// 32-3
extern uint32_t*    oosiit_cfg_mbox;
extern uint16_t     oosiit_cfg_mbox_size;
// 32-4
extern uint32_t*    oosiit_cfg_mutex_memory;
// 32-5
extern uint32_t*    mp_tcb;      
extern uint16_t     mp_tcb_size;
// 32-6
extern uint32_t*    os_fifo;
extern uint8_t      os_fifo_size;
// 32-7
extern void**       os_active_TCB;
// 32-8
extern uint32_t*    m_tmr;
extern uint16_t     mp_tmr_size;


// 64-0
extern uint64_t*    oosiit_cfg_globstack;
extern uint16_t     oosiit_cfg_globstack_size;
// 64-1
extern uint64_t*    mp_stk;
extern uint32_t     mp_stk_size;


extern uint32_t    (*std_libspace)[96/4];


// variables which have been transformed from constants into variables

extern uint16_t     os_maxtaskrun;
extern uint32_t     os_stackinfo;
extern uint32_t     os_rrobin;
extern uint32_t     os_rrobin_tout;
extern uint8_t      os_rrobin_use ;
extern uint32_t     os_trv;
extern uint8_t      os_flags;


// other variables

extern uint32_t os_clockrate;
extern uint32_t os_timernum;


// - definition of hidden constants -----------------------------------------------------------------------------------



// - definition of hidden types ---------------------------------------------------------------------------------------



// - declaration of extern hidden functions ---------------------------------------------------------------------------



#ifdef __cplusplus
}       // closing brace for extern "C"
#endif 


#endif  // include guard

// - end-of-file (leave a blank line after)----------------------------------------------------------------------------



