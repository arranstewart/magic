/*			RM.c (Read_Matrices)		March 1991
*
*	This is the basic post-processor for MaGIC.
*
*	Selectmats reads in matrix representations of algebraic
*	models for propositional logics and performs an operation 
*	on each one that satisfies a condition.  It is called with 
*	three parameters, being a pointer to a MATRIX structure, 
*	the name of the user-defined selection function and the 
*	print mode (UGLY, PRETTY, TeX or NONE).
*
*	The selector is allowed to return TRUE (1), FALSE (0) or 
*	END_OF_JOB (-1).  Any other value is treated as 0.
*
*	See RM.h for a description of the data structures.
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



#include "RM.h"


void selectmats(int (*selector)(), PRINTMODE printmode)
{
  int i, done;
  MATRIX *m;

  m = mat_initial();
  m->total_got = m->total_put = 0;
  done = 0;
  read_header(stdin,m);
  if ( newsiz(stdin,m) )
    do {
      m->total_got++;
      for (i=0; i<VALUEMAX; i++)
	m->my_values[i] = -1;
      *(m->my_string) = '\0';
      switch ((*selector)(m)) {
      case 1:
	m->total_put++;
	increment_ok(m);
	printmat(m,printmode,SELECTED,stdout);
	break;
      case -1:
	done = 1;
      }
    }
    while (!done && newcase(stdin,m));
  switch(printmode) {
  case UGLY:
    FORALLCON((m),i) printf(" -1");
    printf(" -1 -1 -1 -1 -1\n");
    break;
  case PRETTY:
    printf("\n\n Total read: %d\n Total written: %d\n\n",
	   m->total_got, m->total_put );
    break;
  case TeX:
    printf("\n\n\\end{document}\n");
    break;
  case NONE:
    break;
  }
}




void relatemats(int (*selector)(), char *filename, char *relname)
{
  int i, done;
  MATRIX *m1;
  FILE *f1;
  MATRIX *m2;
  FILE *f2;

  f1 = fopen(filename,"r");
  m1 = mat_initial();
  m1->total_got = m1->total_put = 0;
  done = 0;
  read_header(f1,m1);
  if (newsiz(f1,m1))
    do {
      m1->total_got++;
      for (i=0; i<VALUEMAX; i++)
	m1->my_values[i] = -1;
      m1->my_string[0] = 0;
      f2 = fopen(filename,"r");
      m2 = mat_initial();
      m2->total_got = m1->total_put = 0;
      done = 0;
      read_header(f2,m2);
      if (newsiz(f2,m2))
	do {
	  m2->total_got++;
	  for (i=0; i<VALUEMAX; i++)
	    m2->my_values[i] = -1;
	  *m2->my_string = 0;
	  switch ((*selector)(m1,m2)) {
	  case 1:
	    print_related(m1,m2,relname);
	    break;
	  case -1:
	    done = 1;
	  }
	}
	while (!done && newcase(f2,m2));
      fclose(f2);
    }
    while (!done && newcase(f1,m1));
}




/*
*	True is a dummy function which can be used to force every
*	matrix to be printed out just as it is read in.
*/

int True(MATRIX *m)
{
  return 1;
}






/*
*	There are two versions of the matrix numbers.  The ones 
*	prefixed with "ok" record the numbers on which compute has
*	been performed.  Those without record the numbers read in.
*
*	Before compute happens, the "ok" numbers are incremented.
*/

void increment_ok(MATRIX *m)
{
  int first, i;

  for ( first = 0; first < m->cmax && m->okmatplus[first]; first++ ) ;
  for ( i = first; i < m->cmax; i++ ) m->okmatplus[i] = 1;
  if ( first ) m->okmatplus[first-1]++;
  else {
    if ( !m->okmatno ) {
      if ( !m->okdesno ) {
	if ( !m->okordno ) {
	  if ( m->fragment[NEG] )
	    m->oknegno++;
	}
	m->okordno++;
      }
      m->okdesno++;
    }
    m->okmatno++;
  }
}





/*
*	Mat_malloc allocates memory to a MATRIX pointer and does
*	the necessary initialisation.
*/

MATRIX *mat_initial()
{
  MATRIX *m;

  m = (MATRIX *) malloc(sizeof(MATRIX));
/*
*	WFF initialisation will go in here
*/
  return m;
}





/*
*	An abort function is provided.  This causes a clean exit 
*	after printing an error message (with integer insert). 
*
*	Setting the integer argument to -1 disables this option.
*/


void Abort( char *s, int x)
{
  fprintf( stderr, "\n\n Aborting on detection of an error\n " );
  if ( x == -1 ) fprintf( stderr, s );
  else fprintf( stderr, s, x );
  fprintf( stderr, "\n" );
  exit(1);
}
