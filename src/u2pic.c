/*				Upic.c
*
*	This uses RM (Read Matrices) to read ugly output from 
*	MaGIC and calls a function "choose" to allow the user 
*	to decide of each matrix whether it should be kept or 
*	ignored.  The chosen matrices are printed in the ugly
*	format.
*
*	For details of the matrix structures and other goodies 
*	defined in the header, see RM.h.  The following should be
*	fairly self-explanatory, however.
*/




#include "RM.h"

static FILE *tty;


int main()
{
  int choose();

  tty = fopen( "/dev/tty", "r" );
  selectmats( choose, UGLY );
  return 0;
}




int choose(m)
MATRIX *m;
{
  displaymat( m, "Keep this matrix?  (y/n/q)....." );

  for (;;)
    switch( getc(tty) ) {
    case 'y': return 1;
    case 'n': return 0;
    case 'q': return -1;
    }
}



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
