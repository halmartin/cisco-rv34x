#ifndef RV_BOARDINFO_API_H
#define RV_BOARDINFO_API_H

/*****************************************************************
 * Description: This function returns product ID value as a char-
 * acter string.
 * Input: Pinter to Allocated character string >=12 bytes
 * Output: NULL terminated character string when return value is
 * '0'. NULL if return value is '-1'
 * Return Value: '0' or '-1'
 * **************************************************************/
int boardinfo_getpid(unsigned char *pid);

/*****************************************************************
 * Description: This function returns Vendor ID value as a char-
 * acter string.
 * Input: Pointer to allocated character string >= 4 bytes
 * Output: NULL terminated character string when return value is
 * '0'. NULL if return value is '-1'
 * Return Value: '0' or '-1'
 * **************************************************************/
int boardinfo_getvid(unsigned char *vid);

/*****************************************************************
 * Description: This function returns serial no. value as a char-
 * acter string.
 * Input: Pointer to allocated character string >=12 bytes
 * Output: NULL terminated character string when return value is
 * '0'. NULL if return value is '-1'
 * Return Value: '0' or '-1'
 * **************************************************************/
int boardinfo_getserial(unsigned char *sernum);

/*****************************************************************
 * Description: This function returns list of mac addresses as  
 * two-dimentional character array.
 * Input: Pointer to pointer to allocated character string 3X7 bytes
 * Output: Poiter to NULL terminated character strings when return 
 * value is * '0'. NULL if return value is '-1'
 * Return Value: '0' or '-1'
 * **************************************************************/
int boardinfo_getmac(unsigned char **macinfo);

/*****************************************************************
 * Description: This function returns part number value as a char-
 * acter string.
 * Input: Pointer to allocated character string >= 12 bytes
 * Output: Poiter to NULL terminated character strings when return 
 * value is * '0'. NULL if return value is '-1'
 * Return Value: '0' or '-1'
 * **************************************************************/
int boardinfo_getpart(unsigned char *partno);

/*****************************************************************
 * Description: This function returns software version as a char-
 * acter string.
 * Input: Pointer to allocated character string >=33 bytes
 * Output: Poiter to NULL terminated character strings when return 
 * value is * '0'. NULL if return value is '-1'
 * Return Value: '0' or '-1'
 * **************************************************************/
int boardinfo_getswver(unsigned char *swver);

/*****************************************************************
 * Description: This function returns hardware version as a char-
 * acter string.
 * Input: Pointer to allocated character string >=4 bytes
 * Output: Poiter to NULL terminated character strings when return 
 * value is * '0'. NULL if return value is '-1'
 * Return Value: '0' or '-1'
 * **************************************************************/
int boardinfo_gethwver(unsigned char *hwver);

/*****************************************************************
 * Description: This function returns product name as a char-
 * acter string.
 * Input: Pointer to allocated character string >=8 bytes
 * Output: Poiter to NULL terminated character strings when return 
 * value is * '0'. NULL if return value is '-1'
 * Return Value: '0' or '-1'
 * **************************************************************/
int boardinfo_getprodname(unsigned char *prodname);

/*****************************************************************
 * Description: This function returns product series as a char-
 * acter string.
 * Input: Pointer to allocated character string >=32 bytes
 * Output: Poiter to NULL terminated character strings when return 
 * value is * '0'. NULL if return value is '-1'
 * Return Value: '0' or '-1'
 * **************************************************************/
int boardinfo_getprodser(unsigned char *prodser);

/*****************************************************************
 * Description: This function returns system description as a char-
 * acter string.
 * Input: Pointer to allocated character string >=128 bytes
 * Output: Poiter to NULL terminated character strings when return 
 * value is * '0'. NULL if return value is '-1'
 * Return Value: '0' or '-1'
 * **************************************************************/
int boardinfo_getsysdesc(unsigned char *sysdesc);

/*****************************************************************
 * Description: This function returns system object id as a char-
 * acter string.
 * Input: Pointer to allocated character string >=32 bytes
 * Output: Poiter to NULL terminated character strings when return 
 * value is * '0'. NULL if return value is '-1'
 * Return Value: '0' or '-1'
 * **************************************************************/
int boardinfo_getobjid(unsigned char *objid);

#endif
