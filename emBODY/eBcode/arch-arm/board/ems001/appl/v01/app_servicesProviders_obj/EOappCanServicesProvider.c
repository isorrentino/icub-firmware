/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author:  Valentina Gaggero
 * email:   valentina.gaggero@iit.it
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

// --------------------------------------------------------------------------------------------------------------------
// - doxy
// --------------------------------------------------------------------------------------------------------------------

/* @file       EOMappCanServicesProvider.c
    @brief      This file implements CAN services.
    @author    valentina.gaggero@iit.it
    @date       02/14/2012
**/



// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------
//sys
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

//abs
#include "hal.h"

#if     defined(HAL_USE_VERSION_2) || defined(HAL_IS_VERSION_2)
#include "hal_mpu_name_stm32f407ig.h"
#include "hal_core_cfg.h"
#else
#error --> hal version 1 is not supported anymore
#endif

//embobj
#include "EoCommon.h"
#include "EOtheMemoryPool.h"
#include "EOtheErrorManager.h"
#include "EoError.h"

//embobj-icub
#include "EOicubCanProto.h"
#include "EOicubCanProto_specifications.h"

//embobj-icub-cfg
#include "eOcfg_icubCanProto_messagesLookupTbl.h"

//appl 
#include "EOappTheDataBase.h"

//diagnostics
#include "EOtheEMSapplDiagnostics.h"


#include "EOicubCanProto_hid.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------
#include "EOappCanServicesProvider.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------
#include "EOappCanServicesProvider_hid.h"


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------
#define eoappCanSP_timeoutsenddiagnostics           1000
#define eoappCanSP_onEvtMode_timeoutSendFrame       2000
#define eoappCanSP_onDemandMode_timeoutSendFrame    17000


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables. deprecated: better using _get(), _set() on static variables 
// --------------------------------------------------------------------------------------------------------------------
// empty-section






// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------
static eOresult_t s_eo_appCanSP_canPeriphInit(EOappCanSP *p);
static eOresult_t s_eo_appCanSP_formAndSendFrame(EOappCanSP *p, eOcanport_t emscanport, eOicubCanProto_msgDestination_t dest, 
                                                 eOicubCanProto_msgCommand_t msgCmd, void *val_ptr);
static void s_eo_appCanSP_callbackOnTx_port1_waittransmission(void *arg);
static void s_eo_appCanSP_callbackOnTx_port2_waittransmission(void *arg);
static void s_eo_appCanSP_callbackOnTx_portx_waittransmission(void *arg, hal_can_port_t port);

static void s_eo_appCanSP_callbackOnRx_port1_allertOnReception(void *arg);
static void s_eo_appCanSP_callbackOnRx_port2_allertOnReception(void *arg);
static void s_eo_appCanSP_callbackOnRx_portx_allertOnReception(void *arg, hal_can_port_t port);

static void s_eo_appCanSP_callbackOnErr_port1(void *arg);
static void s_eo_appCanSP_callbackOnErr_port2(void *arg);
static void s_eo_appCanSP_callbackOnErr_portx(void *arg, eOcanport_t port);

static void s_eo_appCanSP_updateDiagnosticValues(EOappCanSP *p, eOcanport_t port);
static void s_eo_appCanSP_clearDiagnosticValues(EOappCanSP *p, eOcanport_t port);


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------
static eOappCanSP_cfg_t defaultcfg = 
{
    .cbkonrx   = 
        {
            // port1
            {
                .fn = NULL,
                .argoffn = NULL
            },
            // port2
            {
                .fn = NULL,
                .argoffn = NULL
            }
        }
};

static const char s_eobj_ownname[] = "EOappCanSP";






volatile uint8_t numtx[2] = {0,0};
//runnning_data_t run_data;
// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------

extern EOappCanSP* eo_appCanSP_New(eOappCanSP_cfg_t *cfg)
{
    eOresult_t      res;
    EOappCanSP      *retptr = NULL;
    EOicubCanProto  *icubCanProto_ptr;

    if(NULL == cfg)
    {
        cfg = &defaultcfg;
    }
    

    eo_icubCanProto_cfg_t icubCanProto_cfg = 
    {
        EO_INIT(.msgClasses_LUTbl__ptr)     icubCanProto_msgClasses_lookupTbl__ptr 
    };



//1) initialise icubCanProto
    icubCanProto_ptr = eo_icubCanProto_New(&icubCanProto_cfg);
    if(NULL == icubCanProto_ptr)
    {
        return(NULL);
    }

//3) create the obj (i get the memory for the object)
    retptr = eo_mempool_GetMemory(eo_mempool_GetHandle(), eo_mempool_align_32bit, sizeof(EOappCanSP), 1);

    memcpy(&retptr->cfg, cfg, sizeof(eOappCanSP_cfg_t));
    retptr->icubCanProto_ptr = icubCanProto_ptr;
    retptr->runmode = eo_appCanSP_runMode__onEvent;
    
    //wait machanism
    retptr->waittxdata[hal_can_port1].numoftxframe2send = 0;
    retptr->waittxdata[hal_can_port2].numoftxframe2send = 0;
    
    retptr->waittxdata[hal_can_port1].waitenable = eobool_false;
    retptr->waittxdata[hal_can_port2].waitenable = eobool_false;
    

    extern const hal_cfg_t *hal_cfgMINE;
    //can port 1
    //the max num of token in semaphore is equal to the max num of frame in can out queue.
    retptr->waittxdata[hal_can_port1].semaphore = osal_semaphore_new(255/*maxtokens*/, 0/*current num of token*/);
    if(NULL == retptr->waittxdata[hal_can_port1].semaphore)
    {
        return(NULL);
    }
     osal_semaphore_set(retptr->waittxdata[hal_can_port1].semaphore, 0);
    
    //can port 2
    //the max num of token in semaphore is equal to the max num of frame in can out queue.
    retptr->waittxdata[hal_can_port2].semaphore = osal_semaphore_new(255/*maxtokens*/, 0/*current num of token*/);
    if(NULL == retptr->waittxdata[hal_can_port2].semaphore)
    {
        return(NULL);
    }
    
    osal_semaphore_set(retptr->waittxdata[hal_can_port2].semaphore, 0);
        

// 4) initialise peripheral
    res = (eOresult_t)s_eo_appCanSP_canPeriphInit(retptr);
    if(eores_OK != res)
    {
        return(NULL);
    }
    
    
    retptr->run_data.isrunning = 0;
    
    
    retptr->run_data.numoftxframe[0] = 0;
    retptr->run_data.semafori[0] = osal_semaphore_new(255/*maxtokens*/, 0/*current num of token*/);
    if(retptr->run_data.semafori[0] == NULL)
    {
        return(NULL);
    }
    retptr->run_data.semafori[1] = osal_semaphore_new(255/*maxtokens*/, 0/*current num of token*/);
    retptr->run_data.numoftxframe[1] = 0;
    if(retptr->run_data.semafori[1] == NULL)
    {
        return(NULL);
    }

    return(retptr);
}


extern eOresult_t eo_appCanSP_starttransmit_XXX(EOappCanSP *p, eOcanport_t port, uint8_t *numofTXframe)
{
    uint8_t                 numofoutframe = 0;
#if     defined(HAL_USE_VERSION_2) || defined(HAL_IS_VERSION_2)  
    hal_irqn_t              irqn = (eOcanport1 == port)? hal_mpu_name_stm32f407ig_CAN1_TX_IRQn : hal_mpu_name_stm32f407ig_CAN2_TX_IRQn;    
#else
    hal_irqn_t              irqn = (eOcanport1 == port)? hal_arch_arm_CAN1_TX_IRQn : hal_arch_arm_CAN2_TX_IRQn;
#endif    
   
    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }
    
    //disa tx
    hal_sys_irqn_disable(irqn);
    
    
    //set num of can frame in out queue
    hal_can_out_get((hal_can_port_t)port, &numofoutframe);
    numtx[port] = numofoutframe;
    p->run_data.numoftxframe[port] = numofoutframe;
    
    hal_sys_irqn_enable(irqn);
    
    if(numofoutframe != 0)
    {
        hal_can_transmit((hal_can_port_t)port);
    }
    
    if(p->periphstatus[port].isnewvalue)
    {
        s_eo_appCanSP_updateDiagnosticValues(p, port);
        eo_theEMSdgn_Signalerror(eo_theEMSdgn_GetHandle(), eodgn_nvidbdoor_emsperiph , eoappCanSP_timeoutsenddiagnostics);
        s_eo_appCanSP_clearDiagnosticValues(p, port);
    }

    if(NULL != numofTXframe)
    {
        *numofTXframe = numofoutframe;
    }
    return(eores_OK);
}



extern eOresult_t eo_appCanSP_wait_XXX(EOappCanSP *p, eOcanport_t port)
{
//    uint8_t                 numofoutframe = 0;
//    hal_arch_arm_irqn_t     irqn;
    osal_result_t           osal_res = osal_res_OK ;

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }
    
    if(p->run_data.numoftxframe[port] != 0)
    {
        //osal_res = osal_semaphore_decrement(p->run_data.semafori[port], /*osal_reltimeINFINITE*/ eoappCanSP_onDemandMode_timeoutSendFrame);
        osal_res = osal_semaphore_decrement(p->run_data.semafori[port], 3*osal_reltime1ms); 
        #warning marco.accame: in here i put a timeout of 3 ms as there were problems (before it was 17 ms)
        if(osal_res != osal_res_OK)
        {
            eOerrmanDescriptor_t errdes = {0};
            errdes.code                 = eoerror_code_get(eoerror_category_System, eoerror_value_SYS_canservices_cantxfailure);
            errdes.param                = (eOcanport1 == port) ? (1) : (2);
            errdes.sourcedevice         = eo_errman_sourcedevice_localboard;
            errdes.sourceaddress        = 0;  
//            if(eOcanport1 == port)            
//                hal_trace_puts("tout-c1"); 
//            else 
//                hal_trace_puts("tout-c2");                
            eo_errman_Error(eo_errman_GetHandle(), eo_errortype_error, NULL, s_eobj_ownname, &errdes);    
        }
    }
    return((eOresult_t)osal_res);
}



extern eOresult_t eo_appCanSP_SendCmd2Joint(EOappCanSP *p, eOmc_jointId_t jId, eOicubCanProto_msgCommand_t msgCmd, void *val_ptr)
{
    eOresult_t                                  res;
    eOappTheDB_jointOrMotorCanLocation_t        canLoc;
    eOicubCanProto_msgDestination_t             msgdest;

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    res = eo_appTheDB_GetJointCanLocation(eo_appTheDB_GetHandle(), jId,  &canLoc, NULL);
    if(eores_OK != res)
    {
        return(res);
    }


    //set destination of message
    msgdest.dest =ICUBCANPROTO_MSGDEST_CREATE(canLoc.indexinsidecanboard, canLoc.addr);
   
    res = s_eo_appCanSP_formAndSendFrame(p, canLoc.emscanport, msgdest, msgCmd, val_ptr);
    return(res);
}


extern eOresult_t eo_appCanSP_SendCmd2Motor(EOappCanSP *p, eOmc_motorId_t mId, eOicubCanProto_msgCommand_t msgCmd, void *val_ptr)
{
    eOresult_t                                  res;
    eOappTheDB_jointOrMotorCanLocation_t        canLoc;
    eOicubCanProto_msgDestination_t             msgdest;

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    res = eo_appTheDB_GetMotorCanLocation(eo_appTheDB_GetHandle(), mId,  &canLoc, NULL);
    if(eores_OK != res)
    {
        return(res);
    }

    //set destination of message 
    msgdest.dest =ICUBCANPROTO_MSGDEST_CREATE(canLoc.indexinsidecanboard, canLoc.addr);
    
    res = s_eo_appCanSP_formAndSendFrame(p, canLoc.emscanport, msgdest, msgCmd, val_ptr);
    return(res);
}


extern eOresult_t eo_appCanSP_SendCmd2SnrMais(EOappCanSP *p, eOas_maisId_t sId, eOicubCanProto_msgCommand_t msgCmd, void *val_ptr)
{
    eOresult_t                                  res;
    eOappTheDB_board_canlocation_t              canLoc;
    eOicubCanProto_msgDestination_t             msgdest;

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    res = eo_appTheDB_GetSnsrMaisCanLocation(eo_appTheDB_GetHandle(), sId,  &canLoc);
    if(eores_OK != res)
    {
        return(res);
    }

    msgdest.dest = ICUBCANPROTO_MSGDEST_CREATE(0, canLoc.addr);
    res = s_eo_appCanSP_formAndSendFrame(p, (eOcanport_t)canLoc.emscanport, msgdest, msgCmd, val_ptr);
    
    return(res);
    
}

extern eOresult_t eo_appCanSP_SendCmd2SnrStrain(EOappCanSP *p, eOas_strainId_t sId, eOicubCanProto_msgCommand_t msgCmd, void *val_ptr)
{
    eOresult_t                                  res;
    eOappTheDB_board_canlocation_t              canLoc;
    eOicubCanProto_msgDestination_t             msgdest;

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    res = eo_appTheDB_GetSnsrStrainCanLocation(eo_appTheDB_GetHandle(), sId,  &canLoc);
    if(eores_OK != res)
    {
        return(res);
    }

    msgdest.dest = ICUBCANPROTO_MSGDEST_CREATE(0, canLoc.addr);
    res = s_eo_appCanSP_formAndSendFrame(p, (eOcanport_t)canLoc.emscanport, msgdest, msgCmd, val_ptr);
    
    return(res);
    
}


extern eOresult_t eo_appCanSP_SendCmd(EOappCanSP *p, eOcanport_t emscanport, eOicubCanProto_msgDestination_t dest, eOicubCanProto_msgCommand_t msgCmd, void *val_ptr)
{
    eOresult_t      res;

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    res = s_eo_appCanSP_formAndSendFrame(p, emscanport, dest, msgCmd, val_ptr);
    return(res);
}




extern eOresult_t eo_appCanSP_read(EOappCanSP *p, eOcanport_t canport, uint8_t numofcanframe, uint8_t *numofreadcanframes)
{
    eOresult_t          res = eores_NOK_generic;
    hal_result_t        halres = hal_res_NOK_nodata;
    hal_can_frame_t     canframe = {0};
    uint8_t             i;
    uint8_t readcanframes = 0;
    

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    for(i=0; i<numofcanframe ; i++)
    {
        memset(&canframe, 0, sizeof(hal_can_frame_t));
        
        halres = hal_can_get((hal_can_port_t)canport, &canframe, NULL);
        if(hal_res_OK != halres)
        {
            break;      // marco.accame on 12 jan 2015: changed the original continue in a break because:
                        // if we have a NOK then we cannot go on because the fifo is surely empty.                     
        }

        readcanframes ++;
               
        
        res = eo_icubCanProto_ParseCanFrame(p->icubCanProto_ptr, (eOcanframe_t*)&canframe, (eOcanport_t)canport);
        res = res;
        #warning marco.accame: if eo_icubCanProto_ParseCanFrame() does not recognise a canframe it must return an error code to be sent up via diagnostics
//        if(eores_OK != res) 
//        {                
//        }
    }
    
    if(NULL != numofreadcanframes)
    {
        *numofreadcanframes = readcanframes;    
    }

    if(p->periphstatus[canport].isnewvalue)
    {
        s_eo_appCanSP_updateDiagnosticValues(p, canport);
        eo_theEMSdgn_Signalerror(eo_theEMSdgn_GetHandle(), eodgn_nvidbdoor_emsperiph , eoappCanSP_timeoutsenddiagnostics);
        s_eo_appCanSP_clearDiagnosticValues(p, canport);
    }
    
    return(eores_OK);
}

extern eOresult_t eo_appCanSP_GetNumOfRecCanframe(EOappCanSP *p, eOcanport_t canport, uint8_t *numofRXcanframe)
{
    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    // here not check if numofRXcanframe is null because hal function already do it.   
    return((eOresult_t)hal_can_received((hal_can_port_t)canport, numofRXcanframe));
}


extern eOresult_t eo_appCanSP_GetNumOfTxCanframe(EOappCanSP *p, eOcanport_t canport, uint8_t *numofTXcanframe)
{
    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    // here not check if numofRXcanframe is null because hal function already do it.   
    return((eOresult_t)hal_can_out_get((hal_can_port_t)canport, numofTXcanframe));
}


extern eOresult_t eo_appCanSP_SetRunMode(EOappCanSP *p, eo_appCanSP_runMode_t runmode)
{
    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }
    
    p->runmode = runmode;
    
//     if(eo_appCanSP_runMode__onDemand ==  p->runmode)
//     {
//         p->waittxdata[hal_can_port1].waitenable = eobool_true;
//         p->waittxdata[hal_can_port2].waitenable = eobool_true;
//     }
//     else
//     {
        p->waittxdata[hal_can_port1].waitenable = eobool_false;
        p->waittxdata[hal_can_port2].waitenable = eobool_false;
//     }
    
    if(eo_appCanSP_runMode__onDemand ==  p->runmode)
    {
        p->run_data.isrunning = 1;
    }
    else
    {
        p->run_data.isrunning = 0;
    }
    
    return(eores_OK);
}

extern eOresult_t eo_appCanSP_StartTransmitCanFrames(EOappCanSP *p, eOcanport_t canport, eOboolvalues_t waitflag)
{
    uint8_t                 numofoutframe = 0;
    eOresult_t              res = eores_OK;
    hal_result_t            halres;


    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

#if     defined(HAL_USE_VERSION_2) || defined(HAL_IS_VERSION_2)  
    hal_irqn_t              irqn = (eOcanport1 == canport)? hal_mpu_name_stm32f407ig_CAN1_TX_IRQn : hal_mpu_name_stm32f407ig_CAN2_TX_IRQn;    
#else
    hal_irqn_t              irqn = (eOcanport1 == canport)? hal_arch_arm_CAN1_TX_IRQn : hal_arch_arm_CAN2_TX_IRQn;
#endif    
     

    //disa tx
    hal_sys_irqn_disable(irqn);
    
    //set num of can frame in out queue
    hal_can_out_get((hal_can_port_t)canport, &numofoutframe);

    if((waitflag) && (numofoutframe >0))     //prepare data for wait sending can frame
    {
        p->waittxdata[canport].waitenable = eobool_true;
    }
    p->waittxdata[canport].numoftxframe2send = numofoutframe;

    //ena tx
    hal_sys_irqn_enable(irqn);

    
    if(numofoutframe != 0)
    {
        halres = hal_can_transmit((hal_can_port_t)canport);
        res = (hal_res_OK == halres) ? (eores_OK):(eores_NOK_generic);
    }

    
    return(res);
}


extern eOresult_t eo_appCanSP_WaitTransmitCanFrames(EOappCanSP *p, eOcanport_t canport)
{
    osal_result_t osal_res;
    
    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    if(p->waittxdata[canport].waitenable == eobool_false)
    {
        return(eores_NOK_nodata);
    }
    

//    osal_res = osal_semaphore_decrement(p->run_data.semafori[canport], osal_reltimeINFINITE);
//    
//    if(osal_res != osal_res_OK)
//    {
//        char str[100];
//        snprintf(str, sizeof(str)-1, "Err osal decr = %d", osal_res );        
//        hal_trace_puts(str); 
//    }
    
    #warning marco.accame: put a timeout of 5 ms (before it was infinite)
    osal_result_t osalres = osal_semaphore_decrement(p->run_data.semafori[canport], 5*osal_reltime1ms);
    
    if(osal_res_OK != osalres)
    {
        // manage error
        eOerrmanDescriptor_t errdes = {0};
        errdes.code                 = eoerror_code_get(eoerror_category_System, eoerror_value_SYS_canservices_cantxfailure);
        errdes.param                = (eOcanport1 == canport) ? (1) : (2);
        errdes.sourcedevice         = eo_errman_sourcedevice_localboard;
        errdes.sourceaddress        = 0;  
        //if(eOcanport1 == canport)            
        //    hal_trace_puts("tout-zz"); 
        //else 
        //    hal_trace_puts("tout-zz");                
        eo_errman_Error(eo_errman_GetHandle(), eo_errortype_error, NULL, s_eobj_ownname, &errdes);    
    }    
    
    
    //if i'm here i wake up
#if     defined(HAL_USE_VERSION_2) || defined(HAL_IS_VERSION_2)  
    hal_irqn_t              irqn = (eOcanport1 == canport)? hal_mpu_name_stm32f407ig_CAN1_TX_IRQn : hal_mpu_name_stm32f407ig_CAN2_TX_IRQn;    
#else
    hal_irqn_t              irqn = (eOcanport1 == canport)? hal_arch_arm_CAN1_TX_IRQn : hal_arch_arm_CAN2_TX_IRQn;
#endif  
   
    //disa tx
    hal_sys_irqn_disable(irqn);
    
    p->waittxdata[canport].waitenable = eobool_false;
    p->waittxdata[canport].numoftxframe2send = 0;
    
    //ena tx
    hal_sys_irqn_enable(irqn);
    
    return((eOresult_t)osal_res);

}

extern eOresult_t eo_appCanSP_StartTransmitAndWait(EOappCanSP *p, eOcanport_t canport)
{
    uint8_t                 numofoutframe = 0, after = 0;

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }


#if     defined(HAL_USE_VERSION_2) || defined(HAL_IS_VERSION_2)  
    hal_irqn_t              irqn = (eOcanport1 == canport)? hal_mpu_name_stm32f407ig_CAN1_TX_IRQn : hal_mpu_name_stm32f407ig_CAN2_TX_IRQn;    
#else
    hal_irqn_t              irqn = (eOcanport1 == canport)? hal_arch_arm_CAN1_TX_IRQn : hal_arch_arm_CAN2_TX_IRQn;
#endif  

    //disa tx
    hal_sys_irqn_disable(irqn);
    
    p->waittxdata[canport].waitenable = eobool_true;
    
    //set num of can frame in out queue
    hal_can_out_get((hal_can_port_t)canport, &numofoutframe);
    p->waittxdata[canport].numoftxframe2send = numofoutframe;
    //ena tx
    hal_sys_irqn_enable(irqn);

    
    hal_can_transmit((hal_can_port_t)canport);

//    osal_semaphore_decrement(p->waittxdata[canport].semaphore, osal_reltimeINFINITE);
    
    #warning marco.accame: put a timeout of 5 ms (before it was infinite)
    osal_result_t osalres = osal_semaphore_decrement(p->waittxdata[canport].semaphore, 5*osal_reltime1ms);
    
    if(osal_res_OK != osalres)
    {
        // manage error
        eOerrmanDescriptor_t errdes = {0};
        errdes.code                 = eoerror_code_get(eoerror_category_System, eoerror_value_SYS_canservices_cantxfailure);
        errdes.param                = (eOcanport1 == canport) ? (1) : (2);
        errdes.sourcedevice         = eo_errman_sourcedevice_localboard;
        errdes.sourceaddress        = 0;  
        //if(eOcanport1 == canport)            
        //    hal_trace_puts("tout-qq"); 
        //else 
        //    hal_trace_puts("tout-qq");                
        eo_errman_Error(eo_errman_GetHandle(), eo_errortype_error, NULL, s_eobj_ownname, &errdes);    
    }
    
    //if i'm here i just wake up
    hal_sys_irqn_disable(irqn);
    p->waittxdata[canport].waitenable = eobool_false;    
    hal_can_out_get((hal_can_port_t)canport, &after);
  
    
    hal_sys_irqn_enable(irqn);

    return(eores_OK);
    
}

extern eOresult_t eo_appCanSP_EmptyCanOutputQueue(EOappCanSP *p, eOcanport_t canport)
{
    uint8_t numberofoutcanframe;
    //uint8_t after;
    //char str[100];
    
    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

#if     defined(HAL_USE_VERSION_2) || defined(HAL_IS_VERSION_2)  
    hal_irqn_t              irqn = (eOcanport1 == canport)? hal_mpu_name_stm32f407ig_CAN1_TX_IRQn : hal_mpu_name_stm32f407ig_CAN2_TX_IRQn;    
#else
    hal_irqn_t              irqn = (eOcanport1 == canport)? hal_arch_arm_CAN1_TX_IRQn : hal_arch_arm_CAN2_TX_IRQn;
#endif  
   
    //disa tx
    hal_sys_irqn_disable(irqn);
    
    //set num of can frame in out queue
    hal_can_out_get((hal_can_port_t)canport, &numberofoutcanframe);
    
    if(0 == numberofoutcanframe)
    {
        //ena tx
        hal_sys_irqn_enable(irqn);
        //the queue is empty, so i do nothing.
        return(eores_OK);
    }
    
    p->waittxdata[canport].waitenable = eobool_true;
    p->waittxdata[canport].numoftxframe2send = numberofoutcanframe;
    
    //ena tx
    hal_sys_irqn_enable(irqn);
    
    //wait until all can frame are sent
    #warning --> put a timeout of 5 ms in here, otherwise there are errors when entering the RUN if the can does not have attached boards.
    osal_result_t osalres = osal_semaphore_decrement(p->waittxdata[canport].semaphore, 5*osal_reltime1ms);
    
    if(osal_res_OK != osalres)
    {
        // manage error
        eOerrmanDescriptor_t errdes = {0};
        errdes.code                 = eoerror_code_get(eoerror_category_System, eoerror_value_SYS_canservices_cantxfailure);
        errdes.param                = (eOcanport1 == canport) ? (1) : (2);
        errdes.sourcedevice         = eo_errman_sourcedevice_localboard;
        errdes.sourceaddress        = 0;  
        //if(eOcanport1 == canport)            
        //    hal_trace_puts("tout-xx"); 
        //else 
        //    hal_trace_puts("tout-xx");                
        eo_errman_Error(eo_errman_GetHandle(), eo_errortype_error, NULL, s_eobj_ownname, &errdes);    
    }
 
    //if i'm here i just wake up
    hal_sys_irqn_disable(irqn);
    p->waittxdata[canport].waitenable = eobool_false;        
    hal_sys_irqn_enable(irqn);
    
    return(eores_OK);
}

extern eOresult_t eo_appCanSP_EmptyCanInputQueue(EOappCanSP *p, eOcanport_t canport)
{
    eOresult_t res;
    uint8_t numofRXcanframe;
    
    res = eo_appCanSP_GetNumOfRecCanframe(p, canport, &numofRXcanframe);
    if(eores_OK != res)
    {
        return(res);
    }
    res = eo_appCanSP_read(p, canport, numofRXcanframe, NULL);
    return(res);
}


extern eOresult_t eo_appCanSP_SendMessage_TEST(EOappCanSP *p, uint8_t *payload_ptr)
{
    eOresult_t                                  res;
    eOcanframe_t                                canFrame;

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    //form can frame
    canFrame.id = 10;
    canFrame.id_type = 0; //standard id
    canFrame.frame_type = 0; //data frame
    canFrame.size = 8;

    memcpy(canFrame.data, payload_ptr, 8);

    res = (eOresult_t)hal_can_put(hal_can_port1, (hal_can_frame_t*)&canFrame, 
                                 ((p->runmode == eo_appCanSP_runMode__onEvent)? hal_can_send_normprio_now : hal_can_send_normprio_later));
    
     return(res);
}






#ifdef _USE_PROTO_TEST_
extern eOresult_t eo_appCanSP_ConfigMotor(EOappCanSP *p, eOmc_motorId_t mId, eOmc_motor_config_t *cfg)
{
    eOresult_t res;
    eOcanframe_t canFrame;
    eo_icubCanProto_msgDestination_t dest;
    eo_emsCanNetTopo_jointOrMotorCanLocation_t canLoc;
    eo_icubCanProto_msgCommand_t msgCmd = 
    {
        EO_INIT(.class) icubCanProto_msgCmdClass_pollingMotorControl,
        EO_INIT(.cmdId) 0
    };

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    res = eo_emsCanNetTopo_GetMotorCanLocation_ByMotorId(p->emsCanNetTopo_ptr, mId, &canLoc, NULL);
    if(eores_OK != res)
    {
        return(res);
    }

    //set destination of message (one for all msg)
    dest.axis = canLoc.jm_idInBoard;
    dest.canAddr = canLoc.canaddr;


    // 1) send current pid
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_CURRENT_PID;
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->pidcurrent, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame,
                                  ((p->runmode == eo_appCanSP_runMode__onEvent)? hal_can_send_normprio_now : hal_can_send_normprio_later));
    if(eores_OK != res)
    {
        return(res);
    }

//     // 2) send current pid limits
//     msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_CURRENT_PIDLIMITS;
//     res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->pidcurrent, &canFrame);
//     if(eores_OK != res)
//     {
//         return(res);
//     }
//     res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame,
//                                    ((p->runmode == eo_appCanSP_runMode__onEvent)? hal_can_send_normprio_now : hal_can_send_normprio_later));
//     if(eores_OK != res)
//     {
//         return(res);
//     }

//     // 3) set max velocity   
//     msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_MAX_VELOCITY;
//     res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->maxvelocityofmotor, &canFrame);
//     if(eores_OK != res)
//     {
//         return(res);
//     }
//     res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame,
//                                    ((p->runmode == eo_appCanSP_runMode__onEvent)? hal_can_send_normprio_now : hal_can_send_normprio_later));
//     if(eores_OK != res)
//     {
//         return(res);
//     }

//     // 4) set current limit  
//     msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_CURRENT_LIMIT;
//     res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->maxcurrentofmotor, &canFrame);
//     if(eores_OK != res)
//     {
//         return(res);
//     }
//     res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame,
//                                   ((p->runmode == eo_appCanSP_runMode__onEvent)? hal_can_send_normprio_now : hal_can_send_normprio_later));
//     if(eores_OK != res)
//     {
//         return(res);
//     }


//     // 5) set perriodic msg contents  ??? DOVE PRENDO L'INFO
// //    res = eo_icubCanProto_formCanFrame4MotorBoard(ICUBCANPROTO_POL_MB_CMD__SET_CURRENT_LIMIT, (void*)&cfg->maxcurrentofmotor,
// //                                                  boardAddr, axis, &canFrame);
// //    if(eores_OK != res)
// //    {
// //        return(res);
// //    }
// //    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
// //    if(eores_OK != res)
// //    {
// //        return(res);
// //    }


//     // 6) set i2t param      MANCANO!!!
// //    res = eo_icubCanProto_formCanFrame4MotorBoard(ICUBCANPROTO_POL_MB_CMD__SET_I2T_PARAMS, (void*)&cfg->velocityshiftfactor,
// //                                                  boardAddr, axis, &canFrame);
// //    if(eores_OK != res)
// //    {
// //        return(res);
// //    }
// //    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
// //    if(eores_OK != res)
// //    {
// //        return(res);
// //    }


// //enable pwm and control loop

// //     // 5) set pwm pad
// //     msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__ENABLE_PWM_PAD;
// //     res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, NULL, &canFrame);
// //     if(eores_OK != res)
// //     {
// //         return(res);
// //     }
// //     res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
// //     if(eores_OK != res)
// //     {
// //         return(res);
// //     }


// //     // 6) set controller run
// //     msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__CONTROLLER_RUN;
// //     res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, NULL, &canFrame);
// //     if(eores_OK != res)
// //     {
// //         return(res);
// //     }
// //     res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
// //     if(eores_OK != res)
// //     {
// //         return(res);
// //     }

// //#warning aggiunto un messaggio x test!!!    
// //    // 7) set controller run
// //    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__DISABLE_PWM_PAD;
// //    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, NULL, &canFrame);
// //    if(eores_OK != res)
// //    {
// //        return(res);
// //    }
// //    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
// //    if(eores_OK != res)
// //    {
// //        return(res);
// //    }
    return(eores_OK);
}
#endif




#ifdef _USE_PROTO_TEST_
extern eOresult_t eo_appCanSP_SendSetPoint(EOappCanSP *p, eOmc_jointId_t jId, eOmc_setpoint_t *setPoint)
{
    eOresult_t res;
    eOcanframe_t canFrame;
    eo_icubCanProto_msgDestination_t dest;
    eo_emsCanNetTopo_jointOrMotorCanLocation_t canLoc;
    void *val_ptr = NULL;
    eo_icubCanProto_msgCommand_t msgCmd = 
    {
        EO_INIT(.class) icubCanProto_msgCmdClass_pollingMotorControl,
        EO_INIT(.cmdId) 0
    };

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }
    res = eo_emsCanNetTopo_GetJointCanLocation_ByJointId(p->emsCanNetTopo_ptr, jId, &canLoc, NULL);
    if(eores_OK != res)
    {
        return(res);
    }

    dest.axis = canLoc.jm_idInBoard;
    dest.canAddr = canLoc.canaddr;


    switch( setPoint->type)
    {
        case eomc_setpoint_position:
        {
            msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__POSITION_MOVE; 
            val_ptr =  &(setPoint->to.position);    
        }break;

        case eomc_setpoint_velocity:
        {
            msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__VELOCITY_MOVE;                 
            val_ptr =  &(setPoint->to.velocity);    
        }break;

        case eomc_setpoint_torque:
        {
            msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_DESIRED_TORQUE;           
            val_ptr =  &(setPoint->to.torque.value);    
        }break;

        case eomc_setpoint_current:
        {
            msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_DISIRED_CURRENT;                             
            val_ptr =  &(setPoint->to.current.value);    
        }break;

        default:
        {
            return(eores_NOK_unsupported);
        }
    }

    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)val_ptr, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }

    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame,
                                  ((p->runmode == eo_appCanSP_runMode__onEvent)? hal_can_send_normprio_now : hal_can_send_normprio_later));

    if(eores_OK != res)
    {
        return(res);
    }

    return(eores_OK);
}
#endif



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------
static eOresult_t s_eo_appCanSP_canPeriphInit(EOappCanSP *p)
{
    hal_can_cfg_t can_cfg_port1, can_cfg_port2;
    eOresult_t res;

    can_cfg_port1.runmode            = hal_can_runmode_isr_1txq1rxq;
    can_cfg_port1.baudrate           = hal_can_baudrate_1mbps; 
    can_cfg_port1.priorx             = hal_int_priority11;
    can_cfg_port1.priotx             = hal_int_priority11;
#if     defined(HAL_USE_VERSION_2) || defined(HAL_IS_VERSION_2)    
    can_cfg_port1.capacityofrxfifoofframes   = HALCAN1capacityofrxfifoofframes;
    can_cfg_port1.capacityoftxfifoofframes   = HALCAN1capacityoftxfifoofframes;
    can_cfg_port1.capacityoftxfifohighprio   = HALCAN1capacityoftxfifohighprio;
#endif       
    can_cfg_port1.callback_on_rx     = s_eo_appCanSP_callbackOnRx_port1_allertOnReception;
    can_cfg_port1.arg_cb_rx          = (void*)p;
    can_cfg_port1.callback_on_tx     = s_eo_appCanSP_callbackOnTx_port1_waittransmission;
    can_cfg_port1.arg_cb_tx          = (void*)p;
    can_cfg_port1.callback_on_err    = s_eo_appCanSP_callbackOnErr_port1;
    can_cfg_port1.arg_cb_err         = (void*)p;


    
    can_cfg_port2.runmode            = hal_can_runmode_isr_1txq1rxq;
    can_cfg_port2.baudrate           = hal_can_baudrate_1mbps; 
    can_cfg_port2.priorx             = hal_int_priority11;
    can_cfg_port2.priotx             = hal_int_priority11;
#if     defined(HAL_USE_VERSION_2) || defined(HAL_IS_VERSION_2)    
    can_cfg_port2.capacityofrxfifoofframes   = HALCAN2capacityofrxfifoofframes;
    can_cfg_port2.capacityoftxfifoofframes   = HALCAN2capacityoftxfifoofframes;
    can_cfg_port2.capacityoftxfifohighprio   = HALCAN2capacityoftxfifohighprio;    
#endif    
    can_cfg_port2.callback_on_rx     = s_eo_appCanSP_callbackOnRx_port2_allertOnReception;
    can_cfg_port2.arg_cb_rx          = (void*)p;
    can_cfg_port2.callback_on_tx     = s_eo_appCanSP_callbackOnTx_port2_waittransmission;
    can_cfg_port2.arg_cb_tx          = (void*)p;
    can_cfg_port2.callback_on_err    = s_eo_appCanSP_callbackOnErr_port2;
    can_cfg_port2.arg_cb_err         = (void*)p;

    res = (eOresult_t)hal_can_init(hal_can_port1, &can_cfg_port1);

    if(eores_OK != res)
    {
       return(res);
    }

    res = (eOresult_t)hal_can_init(hal_can_port2, &can_cfg_port2);

    if(eores_OK != res)
    {
        return(res);
    }

    res = (eOresult_t)hal_can_enable(hal_can_port1);

    if(eores_OK != res)
    {
        return(res);
    }

    res = (eOresult_t)hal_can_enable(hal_can_port2);

    if(eores_OK != res)
    {
        return(res);
    }

    return(eores_OK);
}



static void s_eo_appCanSP_callbackOnTx_port1_waittransmission(void *arg)
{
    s_eo_appCanSP_callbackOnTx_portx_waittransmission(arg, hal_can_port1);
}

static void s_eo_appCanSP_callbackOnTx_port2_waittransmission(void *arg)
{
    s_eo_appCanSP_callbackOnTx_portx_waittransmission(arg, hal_can_port2);
}


extern uint8_t numofsentframe;
static void s_eo_appCanSP_callbackOnTx_portx_waittransmission(void *arg, hal_can_port_t port)
{
    EOappCanSP *p = (EOappCanSP *)arg;
    
    if(eobool_true == p->waittxdata[port].waitenable)
    {
        p->waittxdata[port].numoftxframe2send --;
        
        if(0 == p->waittxdata[port].numoftxframe2send)
        {
            p->waittxdata[port].waitenable = eobool_false;
            
            #warning --> marco.accame: i think that we should set p->waittxdata[port].waitenable to eobool_false in here and not inside user-level code
            // reason is ... if we dont change it to false quick enough, it may be that numoftxframe2send is further decremented.    
            
            if(osal_res_OK != osal_semaphore_increment(p->waittxdata[port].semaphore, osal_callerISR))
            {
                #warning --> marco.accame: shall we put any eo_errman_Error() in here ? we are inside an ISR. better NO
                // but we may set a flag which will trigger an error later
                hal_trace_puts("error in s_eo_appCanSP_callbackOnTx_portx_waittransmission"); 
            }
        }
    }

    
    if(p->run_data.isrunning)
    {
        if(p->run_data.numoftxframe[port] == 0)
        {
            char str[100];
            snprintf(str, sizeof(str)-1, "Err isr numoftxframe[%d]=0 and numtx[port]=%d", port, numtx[port]);        
            hal_trace_puts(str); 
        }    
        numtx[port]--;
        if(numtx[port] == 0)
        {
            osal_result_t osal_res;
            osal_res  = osal_semaphore_increment(p->run_data.semafori[port], osal_callerISR);
            osal_res = osal_res;
//             if(osal_res != osal_res_OK)
//             {
//                 char str[100];
//                 snprintf(str, sizeof(str)-1, "Err osal incr = %d", osal_res );        
//                 hal_trace_puts(str); 
//             }

        }
    }
}

static void s_eo_appCanSP_callbackOnRx_port1_allertOnReception(void *arg)
{
    s_eo_appCanSP_callbackOnRx_portx_allertOnReception(arg, hal_can_port1);
}

static void s_eo_appCanSP_callbackOnRx_port2_allertOnReception(void *arg)
{
    s_eo_appCanSP_callbackOnRx_portx_allertOnReception(arg, hal_can_port2);
}


static void s_eo_appCanSP_callbackOnRx_portx_allertOnReception(void *arg, hal_can_port_t port)
{
    EOappCanSP *p = (EOappCanSP *)arg;
    
    if(eo_appCanSP_runMode__onDemand == p->runmode)
    {
        return;
    }
    //eom_task_isrSetEvent(p->allertonrxdata[port].task, p->allertonrxdata[port].evt);
    if(NULL == p->cfg.cbkonrx[port].fn)
    {
        return;
    }
    p->cfg.cbkonrx[port].fn(p->cfg.cbkonrx[port].argoffn);
}

static eOresult_t s_eo_appCanSP_formAndSendFrame(EOappCanSP *p, eOcanport_t emscanport, eOicubCanProto_msgDestination_t dest, eOicubCanProto_msgCommand_t msgCmd, void *val_ptr)
{
    eOresult_t          res;
    eOcanframe_t        canFrame;
    osal_result_t       osal_res;
    hal_result_t        hal_res = hal_res_NOK_generic;
    
#ifdef _GET_CANQUEUE_STATISTICS_
    uint8_t             numofoutframe=0;
#endif
    
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, val_ptr, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    
    if(eo_appCanSP_runMode__onEvent == p->runmode)
    {
        
#ifdef _GET_CANQUEUE_STATISTICS_
        hal_can_out_get((hal_can_port_t)emscanport, &numofoutframe);
        eo_theEMSdgn_updateCanTXqueueStatisticsOnConfigMode(emscanport, numofoutframe+1);
#endif
        // marco.accame on nov 28, 2014
        // i try to put a frame into the fifo-tx queue. the only good result is hal_res_OK. 
        // other results can be:
        // - hal_res_NOK_generic (wrong parameters), which it does not happen because the port is valid and frame is not NULL
        // - hal_res_NOK_busy (full fifo-tx), which can happen.
        hal_res = hal_can_put((hal_can_port_t)emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now );
        
        if(hal_res_OK == hal_res)
        {
            // we all are happy: hal_can_put() succesfully accepts a canframe in its tx fifo.
        }
        else
        {
            // problems ...
            
            if(hal_res_NOK_busy == hal_res)
            {   
                // i try a small recovery in case of hal_res_NOK_busy: wait until one frame is sent (by semaphore wait),
                // and then attempt again a hal_can_put().
                
                #if     defined(HAL_USE_VERSION_2) || defined(HAL_IS_VERSION_2)  
                    hal_irqn_t              irqn = (eOcanport1 == emscanport)? hal_mpu_name_stm32f407ig_CAN1_TX_IRQn : hal_mpu_name_stm32f407ig_CAN2_TX_IRQn;    
                #else
                    hal_irqn_t              irqn = (eOcanport1 == emscanport)? hal_arch_arm_CAN1_TX_IRQn : hal_arch_arm_CAN2_TX_IRQn;
                #endif  
                
                hal_sys_irqn_disable(irqn);
                p->waittxdata[emscanport].waitenable        = eobool_true;
                p->waittxdata[emscanport].numoftxframe2send = 1; // one frame only
                hal_sys_irqn_enable(irqn);

                // wait for some time, but NOT FOREVER. otherwise the application hangs up
                osal_res = osal_semaphore_decrement(p->waittxdata[emscanport].semaphore, eoappCanSP_onEvtMode_timeoutSendFrame);
                
                // marco accame: i must reset these values only in case if osal_res is timed-out because otherwise teh isr did that/
                // .... but for now i keep teh previous code and i reset them anyway
                hal_sys_irqn_disable(irqn);
                p->waittxdata[emscanport].waitenable        = eobool_false;
                p->waittxdata[emscanport].numoftxframe2send = 0; 
                hal_sys_irqn_enable(irqn);
                
                if(osal_res_OK == osal_res)
                {
                    // if the semaphore decremented succesfully, i can attempt a second attempt of hal_can_put()
                    hal_res = hal_can_put((hal_can_port_t)emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now );
                    if(eores_OK != res)
                    {
                       eo_errman_Error(eo_errman_GetHandle(), eo_errortype_warning, "hal_can_put() fails for txfifo full after 2nd attempt", s_eobj_ownname, &eo_errman_DescrTobedecided); 
                    }
                }
                else
                {
                    // the sempahore was not decremented correctly because the can-isr did not incremented anything. issue an error
                    #warning --> marco.accame: this situation can happen many times inside the ctrl-loop. think of how to issue a very serious message only ONCE 
                    eOerrmanDescriptor_t errdes = {0};
                    errdes.code                 = eoerror_code_get(eoerror_category_System, eoerror_value_SYS_canservices_cantxfailure);
                    errdes.param                = (eOcanport1 == emscanport) ? (1) : (2);
                    errdes.sourcedevice         = eo_errman_sourcedevice_localboard;
                    errdes.sourceaddress        = 0;  
                    //if(eOcanport1 == emscanport)            
                    //    hal_trace_puts("tout-ccc1"); 
                    //else 
                    //    hal_trace_puts("tout-ccc2");    

                    eo_errman_Error(eo_errman_GetHandle(), eo_errortype_error, NULL, s_eobj_ownname, &errdes);                                           
                }
            }
            else
            {
                eo_errman_Error(eo_errman_GetHandle(), eo_errortype_error, "very unlikely error in hal_can_put() returning hal_res_NOK_generic", s_eobj_ownname, &eo_errman_DescrTobedecided);
            }
            
        }
        
    }
    else // eo_appCanSP_runMode__onDemand
    {
        hal_res = hal_can_put((hal_can_port_t)emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_later);
        
        if(hal_res_NOK_busy == hal_res)
        {
            //Note: removed fatal error because in some case it ca be usefull disconnect can and going on with appl.
            //using emsbackdoor it is possible know if errors occur.
            //eo_errman_Error(eo_errman_GetHandle(), eo_errortype_fatal, s_eobj_ownname, "lost can frame (out-queue full)");
            //here don't update diagnostics because hal_can_put already done it.
            eo_errman_Error(eo_errman_GetHandle(), eo_errortype_error, "hal_can_put() full tx fifo in eo_appCanSP_runMode__onDemand mode", s_eobj_ownname, &eo_errman_DescrTobedecided);
        }
    }
    
    if(p->periphstatus[emscanport].isnewvalue)
    {
        s_eo_appCanSP_updateDiagnosticValues(p, emscanport);
        eo_theEMSdgn_Signalerror(eo_theEMSdgn_GetHandle(), eodgn_nvidbdoor_emsperiph , eoappCanSP_timeoutsenddiagnostics);
        s_eo_appCanSP_clearDiagnosticValues(p, emscanport);
    }
    
    return(res);
}


static void s_eo_appCanSP_callbackOnErr_port1(void *arg)
{
    s_eo_appCanSP_callbackOnErr_portx(arg, eOcanport1);
}

static void s_eo_appCanSP_callbackOnErr_port2(void *arg)
{
    s_eo_appCanSP_callbackOnErr_portx(arg, eOcanport2);
}

static void s_eo_appCanSP_callbackOnErr_portx(void *arg, eOcanport_t port)
{
    EOappCanSP *p = (EOappCanSP *)arg;
    
    hal_can_getstatus((hal_can_port_t)port, &(p->periphstatus[port].st));
    p->periphstatus[port].isnewvalue = 1;
}

static void s_eo_appCanSP_updateDiagnosticValues(EOappCanSP *p, eOcanport_t port)
{
    eo_dgn_emsperiph.can_dev[port].hw.warning = p->periphstatus[port].st.u.s.hw_status.warning;
    eo_dgn_emsperiph.can_dev[port].hw.passive = p->periphstatus[port].st.u.s.hw_status.passive;
    eo_dgn_emsperiph.can_dev[port].hw.busoff = p->periphstatus[port].st.u.s.hw_status.busoff;
    eo_dgn_emsperiph.can_dev[port].sw.txqueueisfull = p->periphstatus[port].st.u.s.sw_status.txqueueisfull;
    eo_dgn_emsperiph.can_dev[port].sw.rxqueueisfull = p->periphstatus[port].st.u.s.sw_status.rxqueueisfull;
}

static void s_eo_appCanSP_clearDiagnosticValues(EOappCanSP *p, eOcanport_t port)
{
    memset(&p->periphstatus[port].st, 0, sizeof(eOappCanSP_periphstatus_t));
    p->periphstatus[port].isnewvalue = 0;
}

// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------



