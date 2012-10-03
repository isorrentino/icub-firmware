/*
 * main-transceiver.hpp
 *
 *  Created on: Feb 20, 2012
 *      Author: icub
 */

#ifndef MAIN_TRANSCEIVER_HPP_
#define MAIN_TRANSCEIVER_HPP_

#define ERROR (-1)
#define OK 		0
#define TRUE	1
#define FALSE	0

#define DEFAULT_LAPTOP_IP	"10.255.37.155" // <- dhcp;   "10.0.0.1" da usare col pc104
//#define DEFAULT_EMS_IP 		"10.255.39.152" // ems
#define DEFAULT_EMS_IP 		"10.255.37.24" // ip della workstation qui dietro.

//#define DEFAULT_LAPTOP_IP		"10.0.0.1"
//#define DEFAULT_EMS_IP 		"10.0.0.2"

#define DEFAULT_PORT		3333
#define SIZE 				1500

#define PC104
#define _DEEP_DEBUG_



#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include <string>
#include <signal.h>
#include <iostream>

using namespace std;
// Ace stuff
#include <ace/ACE.h>
#include "ace/SOCK_Dgram.h"
#include "ace/Addr.h"
#include "ace/Thread.h"
#include "ace/Logging_Strategy.h"	// for logging stuff

#include "main-transceiver.hpp"
#include "hostTransceiver.hpp"

#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/CanBusInterface.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/RateThread.h>
#include <yarp/dev/ControlBoardInterfacesImpl.h>
#include "yarp/dev/ControlBoardInterfacesImpl.inl" //ControlBoardHelper

#include "EOnv_hid.h"
#include "EoMotionControl.h"
#include "EoSkin.h"
#include "EoSensors.h"

#define _AC_
//#include "IRobotInterface.h"
//#include "FeatureInterface.h"
#include "SkinWrapper.h"
#include "debugFunctions.h"

#ifdef _LINUX_UDP_SOCKET_
#include "udp.h"
#endif

#define hal_trace_puts(arg)		printf("%s", arg)



// Catch CTRL + C
static int keepGoingOn   = TRUE;

static void sighandler(int _signum)
{
  printf("Sighandler's got signal %d\n", _signum);
  keepGoingOn = FALSE;
}

static void usage(void);
void commands(void);
void *recvThread(void * arg);
void *sendThread(void * arg);
void *skinThread(void * arg);


extern hostTransceiver *transceiver;
extern uint8_t boardN;

typedef struct
{
		char field[SIZE];
}Data;

typedef struct
{
	string	 		address_string;
	ACE_INET_Addr	addr;
	char 			data[SIZE];
} Board_connect_info;


// Callback functions
void s_callback_button_0(void);
void s_callback_button_1(void);
void s_callback_button_2(void);
void s_callback_button_3(void);
void s_callback_button_4(void);
void s_callback_button_5(void);
void s_callback_button_6(void);
void s_callback_button_7(void);
void s_callback_button_8(void);
void s_callback_button_9(void);
#endif /* MAIN_TRANSCEIVER_HPP_ */
