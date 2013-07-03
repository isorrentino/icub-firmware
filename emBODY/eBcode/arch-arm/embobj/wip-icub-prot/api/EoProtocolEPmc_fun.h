/*
 * Copyright (C) 2013 iCub Facility - Istituto Italiano di Tecnologia
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

#ifndef _EOPROTOCOLEPMC_FUN_H_
#define _EOPROTOCOLEPMC_FUN_H_




/** @file       EoProtocolEPmc_fun.h
	@brief      This header file contains the declarations of the functions which can be ovverridden in the management of motion control 
	@author     marco.accame@iit.it
	@date       05/29/2013
**/

/** @defgroup eo_EoProtocolEPmc_fun Functions used in motion control endpoint
    Here are the functions used in motion control endpoint. They can be overridden as they are weakly defined in the .c file.
    
    @{		
 **/



// - external dependencies --------------------------------------------------------------------------------------------

#include "EOnv.h"
#include "EOrop.h"


 

// - public #define  --------------------------------------------------------------------------------------------------
// empty-section


// - declaration of public user-defined types ------------------------------------------------------------------------- 
// empty-section

    
// - declaration of extern public variables, ... but better using use _get/_set instead -------------------------------
// empty-section

// - declaration of extern public functions ---------------------------------------------------------------------------

// all of them are weakly-defined or/and overridable by means of a specific include file

extern void eoprot_ep_mc_fun_INITIALISE(eOnvEP_t ep, void *ram);


extern void eoprot_ep_mc_fun_INIT_joint_wholeitem(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_wholeitem(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_config(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_config(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_config_pidposition(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_config_pidposition(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_config_pidvelocity(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_config_pidvelocity(const EOnv* nv, const eOropdescriptor_t* rd);
   
extern void eoprot_ep_mc_fun_INIT_joint_config_pidtorque(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_config_pidtorque(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_config_impedance(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_config_impedance(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_config_minpositionofjoint(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_config_minpositionofjoint(const EOnv* nv, const eOropdescriptor_t* rd);
   
extern void eoprot_ep_mc_fun_INIT_joint_config_maxpositionofjoint(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_config_maxpositionofjoint(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_config_velocitysetpointtimeout(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_config_velocitysetpointtimeout(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_config_motionmonitormode(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_config_motionmonitormode(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_config_encoderconversionfactor(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_config_encoderconversionfactor(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_config_encoderconversionoffset(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_config_encoderconversionoffset(const EOnv* nv, const eOropdescriptor_t* rd);


extern void eoprot_ep_mc_fun_INIT_joint_status(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_status(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_status_basic(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_status_basic(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_status_ofpid(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_status_ofpid(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_status_chamaleon04(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_status_chamaleon04(const EOnv* nv, const eOropdescriptor_t* rd);


extern void eoprot_ep_mc_fun_INIT_joint_inputs(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_inputs(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_inputs_externallymeasuredtorque(const EOnv* nv);   
extern void eoprot_ep_mc_fun_UPDT_joint_inputs_externallymeasuredtorque(const EOnv* nv, const eOropdescriptor_t* rd);


extern void eoprot_ep_mc_fun_INIT_joint_cmmnds_calibration(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_cmmnds_calibration(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_cmmnds_setpoint(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_cmmnds_setpoint(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_cmmnds_stoptrajectory(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_cmmnds_stoptrajectory(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_joint_cmmnds_controlmode(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_joint_cmmnds_controlmode(const EOnv* nv, const eOropdescriptor_t* rd);


extern void eoprot_ep_mc_fun_INIT_motor_wholeitem(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_motor_wholeitem(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_motor_config(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_motor_config(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_motor_config_pidcurrent(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_motor_config_pidcurrent(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_motor_config_maxvelocityofmotor(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_motor_config_maxvelocityofmotor(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_motor_config_maxcurrentofmotor(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_motor_config_maxcurrentofmotor(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_motor_status(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_motor_status(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_motor_status_basic(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_motor_status_basic(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_motor_status_chamaleon04(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_motor_status_chamaleon04(const EOnv* nv, const eOropdescriptor_t* rd);
   
   
extern void eoprot_ep_mc_fun_INIT_controller_wholeitem(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_controller_wholeitem(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_controller_config(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_controller_config(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_controller_config_durationofctrlloop(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_controller_config_durationofctrlloop(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_controller_status(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_controller_status(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_controller_status_alljomoinitted(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_controller_status_alljomoinitted(const EOnv* nv, const eOropdescriptor_t* rd);

extern void eoprot_ep_mc_fun_INIT_controller_cmmnds_go2stateofcontroller(const EOnv* nv);
extern void eoprot_ep_mc_fun_UPDT_controller_cmmnds_go2stateofcontroller(const EOnv* nv, const eOropdescriptor_t* rd);



/** @}            
    end of group eo_EoProtocolEPmc_fun  
 **/

#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------

