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
 
/* @file       rt_iit_AdvTimer.c
	@brief      This file implements internals of the advanced timers module.
	@author     marco.accame@iit.it
    @date       07/30/2012
**/


// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "string.h"
#include "oosiit.h"
#include "rt_TypeDef.h"
#include "rt_task.h"
#include "RTX_Config.h" 
#include "rt_MemBox.h"
#include "rt_List.h"
#include "rt_System.h"

#include "rt_iit_AdvTimer.h"
#include "oosiit_hid.h"         

#if 0
// extern variables or functions called inside this module. we dont want to include the "osiit_cfg.h" to keep
// complete isolation from configuration. 
extern oosiit_arrayhead_t *oosiit_cfg_advtmr_ptrs;
extern U32 const *oosiit_cfg_advtmr;
extern U16 const oosiit_cfg_advtmr_size;
#else
#include "oosiit_storage_hid.h"
#endif



// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "rt_iit_AdvTimer.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------

//#define _DEBUG_THE_TIME_SHIFT_
#undef _DEBUG_THE_TIME_SHIFT_

#define ISRCMD_STOP         0
#define ISRCMD_STOPNSTART   1
#define ISRCMD_START        2
#define ISRCMD_DELETE       3

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------
// empty section



// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------
 

// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------

static OS_RESULT s_rt_iit_advtmr_start (OS_ID timer, void* timing, void* action);
static OS_RESULT s_rt_iit_advtmr_stop (OS_ID timer);
static OS_ID s_rt_iit_advtmr_delete (OS_ID timer);

static void s_rt_iit_advtmr_insert(PIIT_ADVTMR p_tmr, WIDETIME_t itcnt);

static void s_rt_iit_advtmr_clear(PIIT_ADVTMR p_tmr);



// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------

/* User Timer list pointer */
static struct OSIIT_XADVTMR os_iit_advtmr;


// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------



OS_ID rt_iit_advtmr_new (void)  {

   PIIT_ADVTMR p_tmr;

   

   // return null if we dont have the timers
   if (oosiit_cfg_advtmr == NULL)  {
      
      return (NULL);
   }

   // get ram
   p_tmr = rt_alloc_box ((U32 *)oosiit_cfg_advtmr);
   if (!p_tmr)  {
      
      return (NULL);
   }

   // reset fields
   s_rt_iit_advtmr_clear(p_tmr);

   
   return (p_tmr);
}

OS_RESULT rt_iit_advtmr_start (OS_ID timer, void* timing, void* action) {
    OS_RESULT res;
	
   	res = s_rt_iit_advtmr_start(timer, timing, action);
   	
	return(res);
}

OS_RESULT rt_iit_advtmr_stop (OS_ID timer)  {
    OS_RESULT res;
	
   	res = s_rt_iit_advtmr_stop(timer);
   	
	return(res);
}



OS_RESULT rt_iit_advtmr_isactive(OS_ID timer)
{
    return((1 == ((PIIT_ADVTMR)timer)->isactive) ? (OS_R_OK) : (OS_R_NOK));
}

OS_ID rt_iit_advtmr_delete (OS_ID timer)  {
    OS_ID res;
	
   	res = s_rt_iit_advtmr_delete(timer);
   	
	return(res);
}


void rt_iit_advtmr_init(void) {
   // init iit advanced timers
   
   os_iit_advtmr.next = NULL;
   os_iit_advtmr.tcnt = 0;

   if(NULL != oosiit_cfg_advtmr_ptrs)
   {
        ((oosiit_arrayhead_t*)oosiit_cfg_advtmr_ptrs)->size = 0;
   }

   if(NULL != oosiit_cfg_advtmr)
   {
        rt_init_box ((U32 *)oosiit_cfg_advtmr, oosiit_cfg_advtmr_size, sizeof(struct OSIIT_ADVTMR));
   }
}


#ifdef _DEBUG_THE_TIME_SHIFT_
typedef struct
{
    U32     tmr;
    U32     delta;
    U64     abstime;
    U16     tid;
} debug_list_info_t;

debug_list_info_t debug_linfo_data[6];
U8 debug_linfo_size = 0;
U8 debug_tmrs[6];

static void s_rt_iit_advtmr_debug_read_list_of_active_timers(void)
{
    struct OSIIT_XADVTMR my_iit_advtmr;
    PIIT_ADVTMR pp;

    debug_linfo_size = 0;
    my_iit_advtmr.next = os_iit_advtmr.next;
    my_iit_advtmr.tcnt = os_iit_advtmr.tcnt;
    while((pp = my_iit_advtmr.next) != NULL)
    {
        debug_tmrs[debug_linfo_size]                = (((U32)pp->par) & 0xFF00)>>8;
        if(0 == debug_tmrs[debug_linfo_size])       debug_tmrs[debug_linfo_size] = 1;
        debug_linfo_data[debug_linfo_size].tmr      = (U32)pp;
        debug_linfo_data[debug_linfo_size].delta    = pp->tcnt;
        debug_linfo_data[debug_linfo_size].abstime  = pp->abstime_iit;
        debug_linfo_data[debug_linfo_size].tid      = (((U32)pp->par) & 0xFFFF);

        debug_linfo_size++;

        my_iit_advtmr.tcnt = pp->tcnt;
        my_iit_advtmr.next = pp->next;
    }

    debug_linfo_data[0].tmr = debug_linfo_data[0].tmr;
}
#endif

void rt_iit_advtmr_tick (void) {
   /* Decrement delta count of timer list head. Timers having the value of   */
   /* zero are removed from the list and the callback function is called.    */
   PIIT_ADVTMR p;
   void_fp_voidp_voidp fn_cback;

   if (os_iit_advtmr.next == NULL) {
      return;
   }
   os_iit_advtmr.tcnt--;

#ifdef _DEBUG_THE_TIME_SHIFT_	
   s_rt_iit_advtmr_debug_read_list_of_active_timers();
#endif

   while (os_iit_advtmr.tcnt == 0 && (p = os_iit_advtmr.next) != NULL) {

      // compute for next timer
      os_iit_advtmr.tcnt = p->tcnt;
      os_iit_advtmr.next = p->next;

      // treat the one just expired ... only if it is NOT being processed by a post-isr 

      if(0 == p->isrbusy)
      { 
          // 1. call a user provided function
          fn_cback = p->cbk;
          if(NULL != fn_cback)
          {
                fn_cback(p, p->par);
          }
    
          // 2. evaluate if it is periodic or oneshot
          if(0 != p->period_iit) 
          {  // periodic
             if(OOSIIT_ASAPTIME != p->abstime_iit)
             {  // absolute
                p->abstime_iit += p->period_iit;
                s_rt_iit_advtmr_insert(p, p->abstime_iit - oosiit_time);
             }
             else
             {  // relative
                s_rt_iit_advtmr_insert(p, p->period_iit);
             }
          }
          else 
          {  // one-shot
             // dont delete the one-shot expired timer
             // rt_free_box ((U32 *)oosiit_cfg_advtmr, p);
             // just reset it
             s_rt_iit_advtmr_clear(p);
          }

      }

   }
   
#ifdef _DEBUG_THE_TIME_SHIFT_ 
   s_rt_iit_advtmr_debug_read_list_of_active_timers();
#endif

} 

void rt_iit_advtmr_synchronise(U64 oldtime) 
{
    // remove the absolute timers and put them in a temporary array.
    // then reinsert them again in the list.
    U8 i;
    PIIT_ADVTMR prev, p_tmr;
    oosiit_advtmr_timing_t timing;
    oosiit_advtmr_action_t action;


    memset(&((oosiit_arrayhead_t*)oosiit_cfg_advtmr_ptrs)->data[0], 0, 4*((oosiit_arrayhead_t*)oosiit_cfg_advtmr_ptrs)->capacity);
    ((oosiit_arrayhead_t*)oosiit_cfg_advtmr_ptrs)->size = 0;

    prev = (PIIT_ADVTMR)&os_iit_advtmr;

#ifdef _DEBUG_THE_TIME_SHIFT_
    s_rt_iit_advtmr_debug_read_list_of_active_timers();
#endif

    // stage 1: remove from the list every absolute timer
    while(NULL != prev->next)  
    {
        // work on its next.
        p_tmr = prev->next;
#ifdef _DEBUG_THE_TIME_SHIFT_
        s_rt_iit_advtmr_debug_read_list_of_active_timers();
#endif
        if(OOSIIT_ASAPTIME == p_tmr->abstime_iit) 
        {   // the timer is an incremental one
            prev = p_tmr;
            continue;        
        }
        else
        {   // found an absolute timer

            // remove the timer but dont destroy it.
            prev->next = p_tmr->next;
            prev->tcnt += p_tmr->tcnt;
#ifdef _DEBUG_THE_TIME_SHIFT_
            s_rt_iit_advtmr_debug_read_list_of_active_timers();
#endif
            // put the timer inside the array
            ((oosiit_arrayhead_t*)oosiit_cfg_advtmr_ptrs)->data[((oosiit_arrayhead_t*)oosiit_cfg_advtmr_ptrs)->size++] = (U32)p_tmr;

        }

    }

    // stage 2: insert them back in the list
#ifdef _DEBUG_THE_TIME_SHIFT_
    s_rt_iit_advtmr_debug_read_list_of_active_timers();
#endif
    for(i=0; i<((oosiit_arrayhead_t*)oosiit_cfg_advtmr_ptrs)->size; i++)
    {
        p_tmr = (PIIT_ADVTMR) ((oosiit_arrayhead_t*)oosiit_cfg_advtmr_ptrs)->data[i];

        // fill action
        action.cbk          = p_tmr->cbk;
        action.par          = p_tmr->par;

        // fill timing. 
        // if periodic it is easy: we leave the same values. if singleshot in the future ok. if in the past we set curtime+1.

        if(0 != p_tmr->period_iit)
        {   // periodic: easy as we leave the same values
            timing.startat      = p_tmr->startat_iit - p_tmr->period_iit;
            timing.firstshot    = p_tmr->period_iit;
            timing.othershots   = p_tmr->period_iit;
        }
        else if(p_tmr->abstime_iit > oosiit_time)
        {   // singleshot in the future: we set startat with the desired expiry time
            timing.startat      = p_tmr->abstime_iit;
            timing.firstshot    = 0;
            timing.othershots   = 0;
        }
        else
        {   // singleshot in the past: we execute it at next tick
            timing.startat      = oosiit_time + 1;
            timing.firstshot    = 0;
            timing.othershots   = 0;
        }


        // now clear the timer and start it
        s_rt_iit_advtmr_clear(p_tmr);
        s_rt_iit_advtmr_start(p_tmr, &timing, &action);
#ifdef _DEBUG_THE_TIME_SHIFT_
        s_rt_iit_advtmr_debug_read_list_of_active_timers();
#endif
    }

}

#if 0
void rt_iit_advtmr_synchronise_buggy_version(U64 oldtime) 
{
    // search for the os_iit_advtmr. if any timer w/ absolyte time then ...

    PIIT_ADVTMR p, p_tmr;
    U64 qsa;

    p = (PIIT_ADVTMR)&os_iit_advtmr;

    // search the whole list for absolute timers
    while(NULL != p->next)  
    {
        // work on its next.
        p_tmr = p->next;

        if(OOSIIT_ASAPTIME != p_tmr->abstime_iit) 
        {   // found an absolute timer 
        
            // remove it from the list but dont destroy it
            p->next = p_tmr->next;
            p->tcnt += p_tmr->tcnt;
            
            // if expired, compute prev abstime and insert in list with delta equal to zero (or 1)
            if(p_tmr->abstime_iit <= oosiit_time)
            {
                if(0 == p_tmr->period_iit)  // one shot
                {
                    p_tmr->abstime_iit = p_tmr->abstime_iit;
                    // execute asap.... but it is ok not now but at next tick
                    s_rt_iit_advtmr_insert(p_tmr, 1);
                }
                else                    // periodic
                {
                    #define _DROP_PAST_PERIODIC_ONE_
                    // drop this time and go to next
                    qsa = (oosiit_time - p_tmr->abstime_iit)% p_tmr->period_iit;
                    #ifdef _DROP_PAST_PERIODIC_ONE_
                    p_tmr->abstime_iit = oosiit_time + p_tmr->period_iit - qsa;
                    s_rt_iit_advtmr_insert(p_tmr, p_tmr->abstime_iit - oosiit_time);
                    #else
                    p_tmr->abstime_iit = oosiit_time - qsa;
                    s_rt_iit_advtmr_insert(p_tmr, 1);
                    #endif 
                    
                }

                
            }
            // if not expired, expiry remains the same. however, insert in list at proper delta.
            else
            {
                s_rt_iit_advtmr_insert(p_tmr, p_tmr->abstime_iit - oosiit_time);
            }

        }

        p = p->next;
   }

}
#endif


OS_RESULT isr_iit_advtmr_start(OS_ID timer, void* timing, void* action) 
{
    PIIT_ADVTMR p_tmr = (PIIT_ADVTMR)timer;
    const oosiit_advtmr_timing_t *tmng = (const oosiit_advtmr_timing_t*)timing;
    const oosiit_advtmr_action_t *acti = (const oosiit_advtmr_action_t*)action;
//    U32 cmd = 0;

    if(p_tmr == NULL) 
    {
        return(OS_R_NOK);
    }

    // cannot start it twice in the isr ...
    if(1 == p_tmr->isrbusy)
    {
        return(OS_R_NOK);
    }

    p_tmr->isrbusy = 1;

//    if(1 == p_tmr->isactive)
//    {   // 0 is stop, 1 is stop+start, 2 is start-only
//        cmd = ISRCMD_STOPNSTART;
//    }
//    else
//    {
//        cmd = ISRCMD_START;
//    }

//    cmd = ISRCMD_START; // because the start() shall internally call stop() if the isactive flag is true

    p_tmr->abstime_iit = tmng->startat;
    p_tmr->tcnt = tmng->firstshot;
    p_tmr->period_iit = tmng->othershots;
    p_tmr->startat_iit  = tmng->startat;
    p_tmr->cbk = acti->cbk;
    p_tmr->par = acti->par;

    // the start() shall internally call stop() if the isactive flag is true
    rt_psq_enq (p_tmr, ISRCMD_START);
    rt_psh_req ();

    return(OS_R_OK);
}

OS_RESULT isr_iit_advtmr_stop(OS_ID timer) 
{
    PIIT_ADVTMR p_tmr = (PIIT_ADVTMR)timer;

    if(p_tmr == NULL) 
    {
        return(OS_R_NOK);
    }

    if(0 == p_tmr->isactive)
    {   // already stopped
        return(OS_R_OK);
    }

    if(1 == p_tmr->isrbusy)
    {
        return(OS_R_NOK);
    }

    p_tmr->isrbusy = 1;

    rt_psq_enq (p_tmr, ISRCMD_STOP); // 0 is stop, 1 is stop+start, 2 is start-only
    rt_psh_req ();
    
    return(OS_R_NOK);
}

OS_RESULT isr_iit_advtmr_delete(OS_ID timer) 
{
    PIIT_ADVTMR p_tmr = (PIIT_ADVTMR)timer;

    if(p_tmr == NULL) 
    {
        return(OS_R_NOK);
    }

    if(1 == p_tmr->isrbusy)
    {
        return(OS_R_NOK);
    }

    p_tmr->isrbusy = 1;


    rt_psq_enq (p_tmr, ISRCMD_DELETE); // 0 is stop, 1 is stop+start, 2 is start-only, 3 is delete
    rt_psh_req ();

    return(OS_R_OK);
}

// mode = 0 is stop, 1 is stop+start, 2 is start-only
void rt_advtmr_psh (OS_ID timer, U32 mode)
{
    PIIT_ADVTMR p_tmr = (PIIT_ADVTMR)timer;
    static volatile oosiit_advtmr_timing_t timing;
    static volatile oosiit_advtmr_action_t action;

    p_tmr->isrbusy = 0;

    switch(mode)
    {
        case ISRCMD_STOP:
        {
            s_rt_iit_advtmr_stop(timer);   
        } break;

//        case ISRCMD_STOPNSTART:
//        {
//            timing.startat = p_tmr->abstime_iit;
//            timing.firstshot = p_tmr->tcnt;
//            timing.othershots = p_tmr->period_iit;
//            action.cbk = p_tmr->cbk;
//            action.par = p_tmr->par;
//            // do internal stop by setting p_tmr->isactive to 1
//            p_tmr->isactive = 1;
//            s_rt_iit_advtmr_start(timer, (void*)&timing, (void*)&action);
//        } break;

        case ISRCMD_START:
        {
            timing.startat = p_tmr->abstime_iit;
            timing.firstshot = p_tmr->tcnt;
            timing.othershots = p_tmr->period_iit;
            action.cbk = p_tmr->cbk;
            action.par = p_tmr->par;
            // dont do internal stop by setting p_tmr->isactive to 0
            //p_tmr->tcnt = p_tmr->abstime_iit = p_tmr->period_iit = 0;
            //p_tmr->cbk = NULL;
            //p_tmr->par = NULL;
            //p_tmr->isactive = 0;
            s_rt_iit_advtmr_start(timer, (void*)&timing, (void*)&action);
        } break;

        case ISRCMD_DELETE:
        {
            s_rt_iit_advtmr_delete(timer);
        } break;
    }

}



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------

//// legacy functions 
//OS_ID rt_iit_advtmr_create (void *timing, void (*cbk)(void*, void*), void* par)  {
//
//   PIIT_ADVTMR p_tmr, p;
//   WIDETIME_t delta,itcnt;
//
//   osiit_advtmr_timing_t *tmng = (osiit_advtmr_timing_t*)timing;
//
//
//   // return null if we dont have the timers
//   if (osiit_cfg_advtmr == NULL)  {
//      return (NULL);
//   }
//
//
//    if(OOSIIT_ASAPTIME != tmng->startat)
//    {
//        if(((tmng->startat+tmng->firstshot) <= osiit_time) && (0 == tmng->othershots))
//        {
//            return(NULL);
//        }
//    }
//    else if(0 == (tmng->firstshot+tmng->othershots))
//    {
//        return(NULL);
//    }
//    
//   // now we have either a valid incremental or a future absolute (at least in one of its periods)
//
//   // get the first delta
//   if(OOSIIT_ASAPTIME == tmng->startat)
//   {
//        // the delta is this
//        itcnt = (0 != tmng->firstshot) ? (tmng->firstshot) : (tmng->othershots);
//   }
//   else
//   {
//        // first absolute occurrence
//        itcnt = tmng->startat + ((0 != tmng->firstshot) ? (tmng->firstshot) : (tmng->othershots));
//
//        // if it is in the past, find smallest multiple bigger than osiit_time
//        if(itcnt <= osiit_time)
//        {
//            itcnt = osiit_time + tmng->othershots - ((osiit_time - itcnt) % tmng->othershots);
//        }
//        
//        // and now ... save in startat the first occurrence
//        tmng->startat = itcnt;
//        // and in itcnt the delta
//        itcnt = tmng->startat - osiit_time;
//   }
//
//
//   // get ram
//   p_tmr = rt_alloc_box ((U32 *)osiit_cfg_advtmr);
//   if (!p_tmr)  {
//      return (NULL);
//   }
//
//   // fill fields:
//   p_tmr->period_iit    = tmng->othershots; // the period is constant
//   p_tmr->abstime_iit   = tmng->startat;  // it may also be 0xfffffffffffffffffffffffffffffffffff
//   p_tmr->cbk = cbk;
//   p_tmr->par = par;
//
//   // look for the rigth entry 
//   p = (PIIT_ADVTMR)&os_iit_advtmr;
//   delta = p->tcnt;
//   while (delta < itcnt && p->next != NULL) {
//      p = p->next;
//      delta += p->tcnt;
//   }
//   /* Right place found, insert timer into the list */
//   p_tmr->next = p->next;
//   p_tmr->tcnt = (TIME_t)(delta - itcnt);
//   p->next = p_tmr;
//   p->tcnt -= p_tmr->tcnt;
//   return (p_tmr);
//}
//
//
//
//
//OS_ID rt_iit_advtmr_kill (OS_ID timer)  {
//   /* Remove user timer from the chained timer list. */
//   PIIT_ADVTMR p, p_tmr;
//
//   p_tmr = (PIIT_ADVTMR)timer;
//   p = (PIIT_ADVTMR)&os_iit_advtmr;
//   /* Search timer list for requested timer */
//   while (p->next != p_tmr)  {
//      if (p->next == NULL) {
//         /* Failed, "timer" is not in the timer list */
//         return (p_tmr);
//      }
//      p = p->next;
//   }
//   /* Timer was found, remove it from the list */
//   p->next = p_tmr->next;
//   p->tcnt += p_tmr->tcnt;
//   rt_free_box ((U32 *)osiit_cfg_advtmr, p_tmr);
//   /* Timer killed */
//   return (NULL);
//} 



// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------

static OS_RESULT s_rt_iit_advtmr_start (OS_ID timer, void* timing, void* action) {

    PIIT_ADVTMR p_tmr, p;
    WIDETIME_t delta,itcnt;
    WIDETIME_t startat = 0;

    const oosiit_advtmr_timing_t *tmng = (const oosiit_advtmr_timing_t*)timing;
    const oosiit_advtmr_action_t *acti = (const oosiit_advtmr_action_t*)action;

    // return error if we have a null timer
    if (timer == NULL)  {
        return (OS_R_NOK);
    }

    startat = tmng->startat;

    // adjust the time values
    if(OOSIIT_ASAPTIME != tmng->startat)
    {
        if(((tmng->startat+tmng->firstshot) <= oosiit_time) && (0 == tmng->othershots))
        {
            return(OS_R_NOK);
        }
    }
    else if(0 == (tmng->firstshot+tmng->othershots))
    {
        return(OS_R_NOK);
    }
    
   // now we have either a valid incremental or a future absolute (at least in one of its periods)

   // get the first delta
   if(OOSIIT_ASAPTIME == tmng->startat)
   {
        // the delta is this
        itcnt = (0 != tmng->firstshot) ? (tmng->firstshot) : (tmng->othershots);
   }
   else
   {
        // first absolute occurrence
        itcnt = tmng->startat + ((0 != tmng->firstshot) ? (tmng->firstshot) : (tmng->othershots));

        // if it is in the past, find smallest multiple which is bigger than oosiit_time
        if(itcnt <= oosiit_time)
        {
            itcnt = oosiit_time + tmng->othershots - ((oosiit_time - itcnt) % tmng->othershots);
        }
        
        // and now ... save in startat the first occurrence
        startat = itcnt;
        // and in itcnt the delta
        itcnt = startat - oosiit_time;
   }

   // we already have ram. it is in tmr
   p_tmr = (PIIT_ADVTMR) timer;

   // if timer is running
   if(1 == p_tmr->isactive)
   {  // stop it
      s_rt_iit_advtmr_stop(timer);
   }

   // fill fields:
   p_tmr->isactive = 1;
   p_tmr->period_iit    = tmng->othershots; // the period is constant
   p_tmr->startat_iit   = startat;
   p_tmr->abstime_iit   = startat;  // it may also be 0xfffffffffffffffffffffffffffffffffff
   p_tmr->cbk = acti->cbk;
   p_tmr->par = acti->par;

   // look for the rigth entry 
   p = (PIIT_ADVTMR)&os_iit_advtmr;
   delta = p->tcnt;
   while (delta < itcnt && p->next != NULL) {
      p = p->next;
      delta += p->tcnt;
   }
   /* Right place found, insert timer into the list */
   p_tmr->next = p->next;
   p_tmr->tcnt = (TIME_t)(delta - itcnt);
   p->next = p_tmr;
   p->tcnt -= p_tmr->tcnt;
   
   return (OS_R_OK);
}

OS_RESULT s_rt_iit_advtmr_stop (OS_ID timer)  {
   /* Remove user timer from the chained timer list. */
   PIIT_ADVTMR p, p_tmr;

   p_tmr = (PIIT_ADVTMR)timer;

   if(0 == p_tmr->isactive)
   {   // already stopped 
       return(OS_R_OK);
   }

   p = (PIIT_ADVTMR)&os_iit_advtmr;
   /* Search timer list for requested timer */
   while (p->next != p_tmr)  {
      if (p->next == NULL) {
         /* Failed, "timer" is not in the timer list */
         return (OS_R_NOK);
      }
      p = p->next;
   }
   /* Timer was found, remove it from the list */
   p->next = p_tmr->next;
   p->tcnt += p_tmr->tcnt;
   
   // reset fields
   s_rt_iit_advtmr_clear(p_tmr);
   
   /* Timer stopped */
   return (OS_R_OK);
} 


static OS_ID s_rt_iit_advtmr_delete (OS_ID timer)  {
   /* Remove user timer from the chained timer list only if it is running. */
   PIIT_ADVTMR p_tmr;

   p_tmr = (PIIT_ADVTMR)timer;
   
   // if timer is running
   if(1 == p_tmr->isactive)
   {  // stop it
      s_rt_iit_advtmr_stop(timer);
   }
   
   rt_free_box ((U32 *)oosiit_cfg_advtmr, p_tmr);

   /* Timer killed */
   return (NULL);
} 

static void s_rt_iit_advtmr_insert(PIIT_ADVTMR p_tmr, WIDETIME_t itcnt) {
  
   PIIT_ADVTMR p;  
   WIDETIME_t delta;



   p = (PIIT_ADVTMR)&os_iit_advtmr;
   delta = p->tcnt;
   while (delta < itcnt && p->next != NULL) {
      p = p->next;
      delta += p->tcnt;
   }
   /* Right place found, insert timer into the list */
   p_tmr->next = p->next;
   p_tmr->tcnt = (TIME_t)(delta - itcnt);
   p->next = p_tmr;
   p->tcnt -= p_tmr->tcnt;

}


static void s_rt_iit_advtmr_clear(PIIT_ADVTMR p_tmr)
{
    p_tmr->cb_type          = ATCB; 
    p_tmr->isrbusy          = 0;
    p_tmr->isactive         = 0;
    p_tmr->next             = NULL;
    p_tmr->tcnt             = 0;
    p_tmr->period_iit       = 0; 
    p_tmr->startat_iit      = 0;
    p_tmr->abstime_iit      = 0;  
    p_tmr->cbk              = NULL;
    p_tmr->par              = NULL;
}


// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------


