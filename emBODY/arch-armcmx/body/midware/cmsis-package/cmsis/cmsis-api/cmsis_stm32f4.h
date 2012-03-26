/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
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
#ifndef _CMSIS_STM32F4_H_
#define _CMSIS_STM32F4_H_


/** @file       cmsis_stm32f4.h
	@brief      This header file implements public interface to the cmsis for the stm32f4x from stmicroelectronics 
	@author     marco.accame@iit.it
	@date       12/28/2011
**/

/* @defgroup cmsis_stm32f4 
    
    @todo   maybe to place in here some decription of the api used for tha hal ...
    
    @{		
 **/


// - external dependencies --------------------------------------------------------------------------------------------

#include "stm32f4xx.h"



// - public #define  --------------------------------------------------------------------------------------------------
// empty-section

// - declaration of public user-defined types ------------------------------------------------------------------------- 
// empty-section

    
// - declaration of extern public variables, ... but better using use _get/_set instead -------------------------------
// empty-section

// - declaration of extern public functions ---------------------------------------------------------------------------

extern void cmsis_stm32f4_hid_set_system_core_clock(void);



/* @}            
    end of group cmsis_stm32f4  
 **/

#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------

