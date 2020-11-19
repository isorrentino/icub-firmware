/*
 * Copyright (C) 2019 iCub Facility - Istituto Italiano di Tecnologia
 * Author:  Marco Accame, Luca Tricerri
 * email:   marco.accame@iit.it,luca.tricerri@iit.it
 * website:https://github.com/icub-tech-iit
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
#ifndef _EOMTHEEMSDIAGNOSTIC_H_
#define _EOMTHEEMSDIAGNOSTIC_H_

/** @file       EOMtheEMSDiagnostic.h
        @brief      This header file implements public interface to the EMS diagnostic singleton.
        @author     luca.tricerri@iit.it
        @date       01/08/2019
 **/
     
#include "EoCommon.h"
#include "EOpacket.h"
#include "EOMtask.h"
#include "EOsocketDatagram.h"
#include "EOVmutex.h"

#include "embot_prot_eth_diagnostic.h"

#include "embot_prot_eth_diagnostic_Node.h" 
#include "embot_prot_eth_diagnostic_Host.h"

class EOMtheEMSDiagnostic
{
    public:
        
        class Params
        {
            public:
            uint8_t taskpriority_{41};
            uint16_t taskstacksize_{2048};  
            eOsizecntnr_t inpdatagramnumber_{2};
            eOsizecntnr_t outdatagramnumber_{1};
            eObool_t usemutex_{eobool_true};   
        };
        
        enum DiagnosticEvents :uint32_t
        {   // must be single bits: 1, 2, 4, 8, 16, 32, 64 etc.
            timeout = 0,
            diagnosticConfigEvent_evt_packet_received = 1,
            diagnosticEvent_evt_packet_tobesent = 2,
        };

        static EOMtheEMSDiagnostic& instance();
        bool initialise(const Params&);
                   
        bool send(const embot::prot::eth::diagnostic::InfoBasic &ib, bool flush);
        bool send(const embot::prot::eth::diagnostic::Info &ii, bool flush);
        
    private:

        EOMtheEMSDiagnostic();
        EOMtheEMSDiagnostic(const EOMtheEMSDiagnostic&)=delete;
        EOMtheEMSDiagnostic(EOMtheEMSDiagnostic&&)=delete;
        EOMtheEMSDiagnostic& operator=(const EOMtheEMSDiagnostic&)=delete;
        EOMtheEMSDiagnostic& operator=(EOMtheEMSDiagnostic&)=delete;
        
        eOresult_t connect(eOipv4addr_t remaddr);            

        //task
        void taskStartup(EOMtask*,uint32_t);
        void taskRun(EOMtask*,uint32_t);            
        const eOreltime_t wakeupTask_{100*eok_reltime1ms};
        EOMtask* task_{nullptr};    
        
        Params params_;
        bool traceIsActive_{true};
        bool forceFlush_{false};//Every ROPS is send without waiting the thread timeout          
        
        EOsocketDatagram* socket_{nullptr};       
    
        //udp packet
        EOpacket* txpkt_{nullptr};
        EOpacket *rxpkt_{nullptr};
        static constexpr uint16_t rawcapacity_ {513};
        uint8_t rawdata[rawcapacity_];
        static constexpr uint16_t inputudpmaxsize_ {128};
        
        EOVmutexDerived* mutexNode_{nullptr};
        EOVmutexDerived* mutexUdpPackage_{nullptr};
        
        //process event
        void processEventRxPacket();
        eOresult_t transmitUdpPackage();
        bool manageArrivedMessage(EOpacket*);
    
        bool connected2host_{false};     
        eOipv4addr_t hostaddress_{EO_COMMON_IPV4ADDR_LOCALHOST};
        const eOipv4addr_t remoteAddr_{EO_COMMON_IPV4ADDR(10,0,1,104)};
        const eOipv4port_t remotePort_{11000};
        const eOipv4port_t localPort_{11000};

        static constexpr char s_eobj_ownname[]{"EOMtheEMSdiagnostic"};        

 
#if defined(DIAGNOSTIC2_enabled) & defined(DIAGNOSTIC2_send_to_daemon)     
        embot::prot::eth::diagnostic::Node node_;
#endif
        
#if defined(DIAGNOSTIC2_enabled) & defined(DIAGNOSTIC2_receive_from_daemon)    
        embot::prot::eth::diagnostic::Host host_;
#endif
     
};

#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------
