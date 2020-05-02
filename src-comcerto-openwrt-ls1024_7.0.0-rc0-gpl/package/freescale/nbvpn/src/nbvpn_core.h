/**************************************************************************
 * Copyright 2015, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        nbvpn_core.h
 *
 * Description: NBVPN packet processing
 *
 * Authors:     Sridhar Pothuganti <sridhar.pothuganti@freescale.com>
 *
 */
/* History
 *  Version     Date            Author                  Change Description
 *    1.0       19/07/2015      Sridhar Pothuganti      Initial Development
 *    1.1       22/07/2015      Chaitanya Sakinam       Full functionality implimentation
*/
/****************************************************************************/
#ifndef _SBR_NBVPN_H_
#define _SBR_NBVPN_H_

typedef struct nbvpn_s {
	unsigned int left_subnet;
	unsigned int left_subnet_mask;
	unsigned int right_subnet;
	unsigned int right_subnet_mask;
	unsigned int right_bcst;;

	struct list_head list;
} nbvpn_t;

#ifdef DEBUG
#define nbvpn_debug(fmt, arg...)  \
	printk(KERN_ALERT"[CPU %d ln %d fn %s] - " fmt, smp_processor_id(), __LINE__, __func__, ##arg)
#else
#define nbvpn_debug(fmt, arg...)
#endif

#define NB_NAME_SERVICE_PORT 137
#define NB_DATAGRAM_SESSION_SERVICE  138

#define T_SUCCESS 0
#define T_FAILURE -1

#endif
