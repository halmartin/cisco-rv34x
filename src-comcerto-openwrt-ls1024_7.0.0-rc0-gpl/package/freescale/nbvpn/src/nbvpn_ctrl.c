/**************************************************************************
 * Copyright 2015, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        nbvpn_ctrl.c
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/spinlock_types.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#include "nbvpn_core.h"
#include "nbvpn_ctrl.h"

LIST_HEAD(nbvpn_list_head);
EXPORT_SYMBOL(nbvpn_list_head);

spinlock_t nbvpn_list_lock;

void sbr_nbvpn_add(nbvpn_t * pRec);
void sbr_nbvpn_del(nbvpn_t * pRec);
void sbr_nbvpn_get_first(nbvpn_t * pRec);
void sbr_nbvpn_get_next(nbvpn_t * pRec);
void sbr_nbvpn_list(void);

void sbr_nbvpn_ioctl_add(nbvpn_ctrl_t * rec)
{
	nbvpn_ctrl_t ctrlrec;
	nbvpn_t *pRec;

	copy_from_user(&ctrlrec, rec, sizeof(nbvpn_ctrl_t));

	pRec = kmalloc(sizeof(nbvpn_t), GFP_ATOMIC);
	if (!pRec) {
		printk("Unable to allocate memory\r\n");
		return;
	}

	pRec->left_subnet = ctrlrec.left_subnet;
	pRec->left_subnet_mask = ctrlrec.left_subnet_mask;
	pRec->right_subnet = ctrlrec.right_subnet;
	pRec->right_subnet_mask = ctrlrec.right_subnet_mask;
	pRec->right_bcst = ctrlrec.right_bcst;

	sbr_nbvpn_add(pRec);
}

EXPORT_SYMBOL(sbr_nbvpn_ioctl_add);

void sbr_nbvpn_ioctl_del(nbvpn_ctrl_t * rec)
{
	nbvpn_ctrl_t ctrlrec;
	nbvpn_t *pRec = NULL;

	pRec = kmalloc(sizeof(nbvpn_t), GFP_ATOMIC);
	if (!pRec) {
		printk("Unable to allocate memory\r\n");
		return;
	}

	copy_from_user(&ctrlrec, rec, sizeof(nbvpn_ctrl_t));
	pRec->left_subnet = ctrlrec.left_subnet;
	pRec->left_subnet_mask = ctrlrec.left_subnet_mask;
	pRec->right_subnet = ctrlrec.right_subnet;
	pRec->right_subnet_mask = ctrlrec.right_subnet_mask;
	pRec->right_bcst = ctrlrec.right_bcst;
	sbr_nbvpn_del(pRec);
	kfree(pRec);
}

EXPORT_SYMBOL(sbr_nbvpn_ioctl_del);

void sbr_nbvpn_ioctl_get_first(nbvpn_ctrl_t * rec)
{
	nbvpn_ctrl_t ctrlrec;
	nbvpn_t *pRec = NULL;

	pRec = kmalloc(sizeof(nbvpn_t), GFP_ATOMIC);
	if (!pRec) {
		printk("Unable to allocate memory\r\n");
		return;
	}

	sbr_nbvpn_get_first(pRec);
	ctrlrec.left_subnet = pRec->left_subnet;
	ctrlrec.left_subnet_mask = pRec->left_subnet_mask;
	ctrlrec.right_subnet = pRec->right_subnet;
	ctrlrec.right_subnet_mask = pRec->right_subnet_mask;
	ctrlrec.right_bcst = pRec->right_bcst;
	copy_to_user(rec, &ctrlrec, sizeof(nbvpn_ctrl_t));
	kfree(pRec);
}

EXPORT_SYMBOL(sbr_nbvpn_ioctl_get_first);

void sbr_nbvpn_ioctl_get_next(nbvpn_ctrl_t * rec)
{
	nbvpn_ctrl_t ctrlrec;
	nbvpn_t *pRec = NULL;

	copy_from_user(&ctrlrec, rec, sizeof(nbvpn_ctrl_t));
	pRec = kmalloc(sizeof(nbvpn_t), GFP_ATOMIC);
	if (!pRec) {
		printk("Unable to allocate memory\r\n");
		return;
	}
	pRec->left_subnet = ctrlrec.left_subnet;
	pRec->left_subnet_mask = ctrlrec.left_subnet_mask;
	pRec->right_subnet = ctrlrec.right_subnet;
	pRec->right_subnet_mask = ctrlrec.right_subnet_mask;
	pRec->right_bcst = ctrlrec.right_bcst;

	sbr_nbvpn_get_next(pRec);

	ctrlrec.left_subnet = pRec->left_subnet;
	ctrlrec.left_subnet_mask = pRec->left_subnet_mask;
	ctrlrec.right_subnet = pRec->right_subnet;
	ctrlrec.right_subnet_mask = pRec->right_subnet_mask;
	ctrlrec.right_bcst = pRec->right_bcst;

	copy_to_user(rec, &ctrlrec, sizeof(nbvpn_ctrl_t));
	kfree(pRec);
}

EXPORT_SYMBOL(sbr_nbvpn_ioctl_get_next);

void sbr_nbvpn_ioctl_list()
{
	sbr_nbvpn_list();
}

EXPORT_SYMBOL(sbr_nbvpn_ioctl_list);

void sbr_nbvpn_add(nbvpn_t * pRec)
{
	INIT_LIST_HEAD(&pRec->list);

	spin_lock(&nbvpn_list_lock);
	list_add_tail(&pRec->list, &nbvpn_list_head);
	spin_unlock(&nbvpn_list_lock);
	//printk \
	    ("leftsubnet:%pI4 left_subnet_mask:%pI4 right_subnet:%pI4 right_subnet_mask:%pI4 right_bcst:%pI4 \r\n", \
	     &pRec->left_subnet, &pRec->left_subnet_mask, &pRec->right_subnet, \
	     &pRec->right_subnet_mask, &pRec->right_bcst);

}

void sbr_nbvpn_del(nbvpn_t * pRec)
{
	nbvpn_t *tmp, *pNode;
	list_for_each_entry_safe(pNode, tmp, &nbvpn_list_head, list) {
		if ((pNode->left_subnet == pRec->left_subnet)
		    && (pNode->right_subnet == pRec->right_subnet)) {
			spin_lock(&nbvpn_list_lock);
			list_del(&pNode->list);
			kfree(pNode);
			spin_unlock(&nbvpn_list_lock);
			break;
		}
	}
}

void sbr_nbvpn_get_first(nbvpn_t * pRec)
{
	nbvpn_t *pNode;
	pNode = list_first_entry(&nbvpn_list_head, nbvpn_t, list);
	pRec->left_subnet = pNode->left_subnet;
	pRec->left_subnet_mask = pNode->left_subnet_mask;
	pRec->right_subnet = pNode->right_subnet;
	pRec->right_subnet_mask = pNode->right_subnet_mask;
	pRec->right_bcst = pNode->right_bcst;
}

void sbr_nbvpn_get_next(nbvpn_t * pRec)
{
	nbvpn_t *pNode;
	int isNext = 0;
	list_for_each_entry(pNode, &nbvpn_list_head, list) {
		if ((pNode->left_subnet == pRec->left_subnet)
		    && (pNode->right_subnet == pRec->right_subnet)) {
			isNext = 1;
			continue;
		}
		if (isNext == 1) {
			pRec->left_subnet = pNode->left_subnet;
			pRec->left_subnet_mask = pNode->left_subnet_mask;
			pRec->right_subnet = pNode->right_subnet;
			pRec->right_subnet_mask = pNode->right_subnet_mask;
			pRec->right_bcst = pNode->right_bcst;
			break;
		}
	}
}

void sbr_nbvpn_list()
{
	nbvpn_t *pNode;
	list_for_each_entry(pNode, &nbvpn_list_head, list) {
		printk
		    ("leftsubnet:%pI4 left_subnet_mask:%pI4 right_subnet:%pI4 right_subnet_mask:%pI4 right_bcst:%pI4 \r\n",
		     &pNode->left_subnet, &pNode->left_subnet_mask,
		     &pNode->right_subnet, &pNode->right_subnet_mask,
		     &pNode->right_bcst);
	}
}
