/**************************************************************************
 * Copyright 2015, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        nbvpn_ctrl.h
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
#ifndef _SBR_NBVPN_CTRL_H_
#define _SBR_NBVPN_CTRL_H_

typedef struct nbvpn_ctrl_s {
	unsigned int left_subnet;
	unsigned int left_subnet_mask;
	unsigned int right_subnet;
	unsigned int right_subnet_mask;
	unsigned int right_bcst;
} nbvpn_ctrl_t;

void sbr_nbvpn_ioctl_add(nbvpn_ctrl_t * rec);
void sbr_nbvpn_ioctl_del(nbvpn_ctrl_t * rec);
void sbr_nbvpn_ioctl_get_first(nbvpn_ctrl_t * rec);
void sbr_nbvpn_ioctl_get_next(nbvpn_ctrl_t * rec);
void sbr_nbvpn_ioctl_list(void);

#endif
