#ifndef _LFI_TYPE_H
#define _LFI_TYPE_H
/**** *lfi_type.h* - Declaration of Fortran types
 *
 *    Author. 
 *    ------- 
 *     Philippe Marguinaud *METEO-FRANCE*
 *     Original : 12-08-2013
 *
 */

#if defined(__PGI) || (defined (__GNUC__) && !defined (__INTEL_COMPILER))

typedef int integer32;
typedef long long int integer64;
typedef int logical;
typedef int character_len;
typedef char character;
typedef double real64;

#define istrue(L) (L)
#define fort_TRUE 1
#define fort_FALSE 0

#elif defined (__INTEL_COMPILER)

typedef int integer32;
typedef long long int integer64;
typedef int logical;
typedef unsigned long long int character_len;
typedef char character;
typedef double real64;

#define istrue(L) (L)
#define fort_TRUE 1
#define fort_FALSE 0

#elif defined (NECSX)

typedef int integer32;
typedef long long int integer64;
typedef int logical;
typedef int character_len;
typedef char character;
typedef double real64;

#define istrue(L) (L)
#define fort_TRUE 1
#define fort_FALSE 0

#elif defined (RS6K)

typedef int integer32;
typedef long long int integer64;
typedef int logical;
typedef int character_len;
typedef char character;
typedef double real64;

#define istrue(L) (L)
#define fort_TRUE 1
#define fort_FALSE 0

#elif defined (_CRAYC)

typedef int integer32;
typedef long long int integer64;
typedef int logical;
typedef long long int character_len;
typedef char character;
typedef double real64;

#define istrue(L) (L)
#define fort_TRUE 1
#define fort_FALSE 0

#else

#error "Unknown architecture" 

#endif

#endif

