#ifndef _SBR_CHAR_DEVICE_H_
#define _SBR_CHAR_DEVICE_H_

#include <asm/ioctl.h>

#define SBR_MAGIC 'L'
/*

    Defining constants require us to decide upon four things

        1. type or magic number (type)
        2. sequence number which is eigth bits wide. This means
           we can have up to 256 ioctl commands (nr)
        3. direction, whether we are reading or writing 
        4. size it is the size of user data involved.
           
   To arrive at unique numbers easily we use the following macros
   _IO(type, nr);	    an ioctl with no parameters
   _IOW(type, nr, dataitem) an ioctl with write parameters (copy_from_user)
   _IOR(type, nr, dataitem)an ioctl with read parameters  (copy_to_user)
   _IOWR(type, nr, dataitem)an ioctl with both write and read parameters.
*/

#define SBR_GET_FIRST_NBVPN_REC  _IOR(SBR_MAGIC,1, void * )
#define SBR_GET_NEXT_NBVPN_REC  _IOWR(SBR_MAGIC,2, void *)

#define SBR_ADD_NBVPN_REC  _IOW(SBR_MAGIC,3,void *)
#define SBR_DEL_NBVPN_REC _IOWR(SBR_MAGIC,4, void *)
#define SBR_LIST_NBVPN_REC _IOWR(SBR_MAGIC,5, void *)
#endif
