#ifndef _LFI_NETW_H
#define _LFI_NETW_H
/**** *lfi_alts.h* - Network LFI file library
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2019
 *
 */

#include "LFI/args.h"
#include "LFI/call.h"
#include "LFI/hndl.h"

extern lficb_t lficb_netw;

extern lfi_hndl_t * lfi_get_netw_hndl (void *);

#endif

