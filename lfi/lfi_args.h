#ifndef _LFI_ARGS_H
#define _LFI_ARGS_H

/**** *lfi_args.h* - Defines LFI routines arguments
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 * Description :
 * Argument lists come in two flavours : 
 * - argument list declaration (for function declarations)
 * - simple argument list (for function calls)
 */


#include "lfi_type.h"

#define LFIOUV_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, logical * LDNOMM, character * CDNOMF, character * CDSTTO, logical * LDERFA, logical * LDIMST, integer64 * KNIMES, integer64 * KNBARP, integer64 * KNBARI, character_len CDNOMF_len, character_len CDSTTO_len
#define LFICAS_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, character * CDNOMA, integer64 * KLONG, integer64 * KPOSEX, logical * LDAVAN, character_len CDNOMA_len
#define LFIECR_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, character * CDNOMA, integer64 * KTAB, integer64 * KLONG, character_len CDNOMA_len
#define LFIFER_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, character * CDSTTC, character_len CDSTTC_len
#define LFILEC_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, character * CDNOMA, integer64 * KTAB, integer64 * KLONG, character_len CDNOMA_len
#define LFINFO_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, character * CDNOMA, integer64 * KLONG, integer64 * KPOSEX, character_len CDNOMA_len
#define LFIPOS_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER
#define LFIVER_ARGS_DECL void * LFI, real64 * PVEROU, character * CDSENS, character_len CDSENS_len
#define LFIOFM_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, integer64 * KFACTM, logical * LDOUVR
#define LFINEG_ARGS_DECL void * LFI, integer64 * KNIVAU
#define LFILAF_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, logical * LDTOUT
#define LFIOSG_ARGS_DECL void * LFI, integer64 * KNIVAU
#define LFINUM_ARGS_DECL void * LFI, integer64 * KNUMER, integer64 * KRANG
#define LFISUP_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, character * CDNOMA, integer64 * KLONUT, character_len CDNOMA_len
#define LFIOPT_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, logical * LDNOMM, character * CDNOMF, character * CDSTTO, logical * LDERFA, logical * LDIMST, integer64 * KNIMES, character_len CDNOMF_len, character_len CDSTTO_len
#define LFINMG_ARGS_DECL void * LFI, integer64 * KNIVAU, integer64 * KULOUT
#define LFICAP_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, character * CDNOMA, integer64 * KLONG, integer64 * KPOSEX, logical * LDRECU, character_len CDNOMA_len
#define LFIFRA_ARGS_DECL void * LFI, logical * LDFRAN
#define LFICFG_ARGS_DECL void * LFI
#define LFIERF_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, logical * LDERFA
#define LFILAS_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, character * CDNOMA, integer64 * KTAB, integer64 * KLONG, character_len CDNOMA_len
#define LFIREN_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, character * CDNOM1, character * CDNOM2, character_len CDNOM1_len, character_len CDNOM2_len
#define LFIINI_ARGS_DECL void * LFI, integer64 * KOPTIO
#define LFIPXF_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, integer64 * KNUMEX, character * CDCFGX, integer64 * KLAREX, integer64 * KXCNEX, integer64 * KFACEX, integer64 * KNUTRA, character * CDNOMA, integer64 * KLONG, character_len CDCFGX_len, character_len CDNOMA_len
#define LFIOEG_ARGS_DECL void * LFI, integer64 * KNIVAU
#define LFINAF_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, integer64 * KNALDO, integer64 * KNTROU, integer64 * KNARES, integer64 * KNAMAX
#define LFIOFD_ARGS_DECL void * LFI, integer64 * KFACMD
#define LFIOMF_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, integer64 * KNIMES
#define LFIAFM_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, integer64 * KFACTM
#define LFISTA_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER
#define LFIOSF_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, logical * LDIMST
#define LFILAP_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, character * CDNOMA, integer64 * KTAB, integer64 * KLONG, character_len CDNOMA_len
#define LFIOEF_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, logical * LDERFA
#define LFIMST_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, logical * LDIMST
#define LFINIM_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER, integer64 * KNIMES
#define LFISFM_ARGS_DECL void * LFI, integer64 * KREP, integer64 * KNUMER
#define LFINSG_ARGS_DECL void * LFI, integer64 * KNIVAU
#define LFIDEB_ARGS_DECL void * LFI, logical * LDEBUG
#define LFIOMG_ARGS_DECL void * LFI, integer64 * KNIVAU, integer64 * KULOUT
#define LFIFMD_ARGS_DECL void * LFI, integer64 * KFACMD
#define LFIPRO_ARGS_DECL void * LFI, integer64 * KNUMER, logical * LDOPEN


#define LFIOUV_ARGS_LIST LFI, KREP, KNUMER, LDNOMM, CDNOMF, CDSTTO, LDERFA, LDIMST, KNIMES, KNBARP, KNBARI, CDNOMF_len, CDSTTO_len
#define LFICAS_ARGS_LIST LFI, KREP, KNUMER, CDNOMA, KLONG, KPOSEX, LDAVAN, CDNOMA_len
#define LFIECR_ARGS_LIST LFI, KREP, KNUMER, CDNOMA, KTAB, KLONG, CDNOMA_len
#define LFIFER_ARGS_LIST LFI, KREP, KNUMER, CDSTTC, CDSTTC_len
#define LFILEC_ARGS_LIST LFI, KREP, KNUMER, CDNOMA, KTAB, KLONG, CDNOMA_len
#define LFINFO_ARGS_LIST LFI, KREP, KNUMER, CDNOMA, KLONG, KPOSEX, CDNOMA_len
#define LFIPOS_ARGS_LIST LFI, KREP, KNUMER
#define LFIVER_ARGS_LIST LFI, PVEROU, CDSENS, CDSENS_len
#define LFIOFM_ARGS_LIST LFI, KREP, KNUMER, KFACTM, LDOUVR
#define LFINEG_ARGS_LIST LFI, KNIVAU
#define LFILAF_ARGS_LIST LFI, KREP, KNUMER, LDTOUT
#define LFIOSG_ARGS_LIST LFI, KNIVAU
#define LFINUM_ARGS_LIST LFI, KNUMER, KRANG
#define LFISUP_ARGS_LIST LFI, KREP, KNUMER, CDNOMA, KLONUT, CDNOMA_len
#define LFIOPT_ARGS_LIST LFI, KREP, KNUMER, LDNOMM, CDNOMF, CDSTTO, LDERFA, LDIMST, KNIMES, CDNOMF_len, CDSTTO_len
#define LFINMG_ARGS_LIST LFI, KNIVAU, KULOUT
#define LFICAP_ARGS_LIST LFI, KREP, KNUMER, CDNOMA, KLONG, KPOSEX, LDRECU, CDNOMA_len
#define LFIFRA_ARGS_LIST LFI, LDFRAN
#define LFICFG_ARGS_LIST LFI
#define LFIERF_ARGS_LIST LFI, KREP, KNUMER, LDERFA
#define LFILAS_ARGS_LIST LFI, KREP, KNUMER, CDNOMA, KTAB, KLONG, CDNOMA_len
#define LFIREN_ARGS_LIST LFI, KREP, KNUMER, CDNOM1, CDNOM2, CDNOM1_len, CDNOM2_len
#define LFIINI_ARGS_LIST LFI, KOPTIO
#define LFIPXF_ARGS_LIST LFI, KREP, KNUMER, KNUMEX, CDCFGX, KLAREX, KXCNEX, KFACEX, KNUTRA, CDNOMA, KLONG, CDCFGX_len, CDNOMA_len
#define LFIOEG_ARGS_LIST LFI, KNIVAU
#define LFINAF_ARGS_LIST LFI, KREP, KNUMER, KNALDO, KNTROU, KNARES, KNAMAX
#define LFIOFD_ARGS_LIST LFI, KFACMD
#define LFIOMF_ARGS_LIST LFI, KREP, KNUMER, KNIMES
#define LFIAFM_ARGS_LIST LFI, KREP, KNUMER, KFACTM
#define LFISTA_ARGS_LIST LFI, KREP, KNUMER
#define LFIOSF_ARGS_LIST LFI, KREP, KNUMER, LDIMST
#define LFILAP_ARGS_LIST LFI, KREP, KNUMER, CDNOMA, KTAB, KLONG, CDNOMA_len
#define LFIOEF_ARGS_LIST LFI, KREP, KNUMER, LDERFA
#define LFIMST_ARGS_LIST LFI, KREP, KNUMER, LDIMST
#define LFINIM_ARGS_LIST LFI, KREP, KNUMER, KNIMES
#define LFISFM_ARGS_LIST LFI, KREP, KNUMER
#define LFINSG_ARGS_LIST LFI, KNIVAU
#define LFIDEB_ARGS_LIST LFI, LDEBUG
#define LFIOMG_ARGS_LIST LFI, KNIVAU, KULOUT
#define LFIFMD_ARGS_LIST LFI, KFACMD
#define LFIPRO_ARGS_LIST LFI, KNUMER, LDOPEN


#endif
