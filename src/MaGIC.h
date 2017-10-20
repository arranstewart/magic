/*
*				MaGIC.h			May 1993
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


#include <ctype.h>
#include <signal.h>
#include <unistd.h>

#ifdef SYSTEMV
#include <sys/systm.h>
#endif

#include "vntr.h"
#include "Mdef.h"
#include "axioms.h"
#include "Mtypes.h"
#include "Mglob.h"
#include "Mproto.h"



/*
*	We need more timing apparatus if times.h is available.
*/

#ifdef HASTIMES
#include <sys/times.h>
#include <limits.h>
struct tms time_buffer;
#endif
