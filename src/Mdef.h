/*
*			Mdef.h
*
*	This is part of the header for MaGIC. It contains the definitions
*	of symbolic constants.
*/

	/****************************************************************
	*								*
	*			    MaGIC 2.1				*
	*								*
	*	    (C) 1993 Australian National University		*
	*								*
	* 		       All rights reserved			*
	*								*
	* The information in this software is subject to change without	*
	* notice and should not be construed as a commitment by the	*
	* Australian National University. The Australian National Uni-	*
	* versity makes no representations about the suitability of	*
	* this software for any purpose. It is supplied "as is" without	*
	* express or implied warranty.  If the software is modified in 	*
	* a manner creating derivative copyright rights, appropriate	*
	* legends may be placed on the derivative work in addition to	*
	* that set forth above.						*
	*								*
	* Permission to use, copy, modify and distribute this software	*
	* and its documentation for any purpose and without fee is	*
	* hereby granted, provided that both the above copyright notice	*
	* and this permission notice appear in all copies and sup-	*
	* porting documentation, and that the name of the Australian	*
	* National University not be used in advertising or publicity	*
	* pertaining to distribution of the software without specific,	*
	* written prior permission.					*
	*								*
	****************************************************************/


#define VERSION "2.1"
#define RELEASE_DATE "May 1993"


/*
*       We need to distinguish between the main file where global 
*       variables are mostly defined and the other files where they are 
*       just declared.
*/

#ifdef TOPFILE
#define GLOBAL
#else
#define GLOBAL extern
#endif


/*
*	The input data files are all in a directory:
*/

#ifndef DATA_DIR
#define DATA_DIR "/usr/local/lib/MaGIC/"
#endif

/*
*	The clock returns times in 1/TICK sec.:
*/

#define TICK 100


/*
*	Next the bounds for various arrays. These are the maxima for
*	meaningful symbols (SYMBOLMAX), user-defined connectives (CMAX), 
*	(sub)formulae (FMAX), testable rules (TMAX) and isomorphs
*	per process (ISOMAX).  VMAX is the maximum number of variables 
*	for formulae. RTMAX is the maximum number of premises (or 
*	conclusions) in a rule.
*/

#define SYMBOLMAX 256
#define CMAX 32
#define FMAX 1024
#define TMAX 64
#define ISOMAX 8192
#define SH_VL 128
#define VMAX 22
#define RTMAX 8
#define ISLMAX (ISOMAX * SH_VL)
#define SLEN 80

/*
*	And a dummy formula offset for primitives.
*/

#define PRIMITIVE -1

/*
*	And the maximum sizes for the various fragments
*/

#ifdef SMALLDISK
#define S_pot  7
#else
#define S_pot  8
#endif
#define S_pO   6
#define S_pont 8
#define S_poN  7
#define S_ln   10
#define S_lat  8
#define S_ba   16
#define S_dln  14
#define S_dlat 10
#define S_Ton  16
#define S_to   16


/*
*	The null premise and null conclusion are negative integers
*/

#define TRIVIAL -1
#define ABSURD -2


/*
*	These constructs are useful abbreviations
*/

#define FORALL(x) for (x=0; x<=siz; x++)
#define FORaLL(x) for (x=siz; x>=0; x--)
#define READLN { fflush(stdout); while (getchar() != '\n') ; }
#define IFF(x,y) ((x && y) || (!x && !y))
#define REMOVE(x,y) y &= ~((unsigned)1 << x)
#define ADDTO(x,y) y |= ((unsigned)1 << x)
#define IN(x,y) ((y & (1 << x)) != 0)
#define SINGLETON(x) (!(x & (x-1)))
#define EP { if ( xdialog ) { printf("E\n"); fflush(stdout); }}
#define outfml(p,q,f) outformula(p,q,f,VARS);
#define symbol_listed(x,s) (symbol_position(x,s) >= 0)
