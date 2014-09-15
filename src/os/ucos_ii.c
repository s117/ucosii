/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
* File : uCOS_II.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#define  OS_GLOBALS                           /* Declare GLOBAL variables                              */
#include "../includes/includes.h"


#define  OS_MASTER_FILE                       /* Prevent the following files from including includes.h */
#include "OS_CORE.C"
#include "OS_FLAG.C"
#include "OS_MBOX.C"
#include "OS_MEM.C"
#include "OS_MUTEX.C"
#include "OS_Q.C"
#include "OS_SEM.C"
#include "OS_TASK.C"
#include "OS_TIME.C"
