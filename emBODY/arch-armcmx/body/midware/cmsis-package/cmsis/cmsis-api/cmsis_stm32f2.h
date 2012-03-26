
// - include guard ----------------------------------------------------------------------------------------------------
#ifndef _CMSIS_STM32F2_H_
#define _CMSIS_STM32F2_H_


/** @file       cmsis_stm32f2.h
	@brief      This header file implements public interface to the cmsis for the stm32f2x from stmicroelectronics 
	@author     marco.accame@iit.it
	@date       11/11/2011
**/

/* @defgroup cmsis_stm32f2 
    
    @todo   maybe to place in here some decription of the api used for tha hal ...
    
    @{		
 **/


// - external dependencies --------------------------------------------------------------------------------------------

#include "stm32f2xx.h"



// - public #define  --------------------------------------------------------------------------------------------------
// empty-section

// - declaration of public user-defined types ------------------------------------------------------------------------- 
// empty-section

    
// - declaration of extern public variables, ... but better using use _get/_set instead -------------------------------
// empty-section

// - declaration of extern public functions ---------------------------------------------------------------------------

extern void cmsis_stm32f2_hid_set_system_core_clock(void);



/* @}            
    end of group cmsis_stm32f2  
 **/

#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------

