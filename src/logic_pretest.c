/*
*			logic_pretest.c		V2.1 (May 1993)
*
*	Most 2-refutations can be found before the search for matrices
*	starts.  The function find_twos does this.  As in the cases of
*	the test and setup routines, this process is very case-ridden.
*
*	When a 2-refutation is found, it is reported to transref by
*	means of the call new_two_ref.  Transref takes care of all the
*	processing, including making sure it is not a repeat of one 
*	already reported or subsumed by a 1-refutation.
*
*	As for set_poss, the parameter info is the vector of sets of
*	possible values.  It must not be changed by anything in this
*	file, as is is needed (and sometimes updated) by transref.
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



#include "MaGIC.h"


static unsigned possval[SZ][SZ];

static trs Tpt;



void new_two_ref( int a, int x, int b, int y )
{
  int cells[3], vals[3];
  boolean valency[3];

  cells[0] = a;	vals[0] = x;	valency[0] = false;
  cells[1] = b;	vals[1] = y;	valency[1] = false;
  cells[2] = -1;	vals[2] = 0;	valency[2] = false;
  new_small_ref(cells, vals, valency, Tpt);
}



boolean find_twos(unsigned info[], trs Tr)
{
  int i, j;

  FORALL(i) FORALL(j) possval[i][j] = info[impindex[i][j]];
  Tpt = Tr;

  if ( theJob->f_fus ) pretest_fus();

  for ( i = 1; i < AXMAX; i++ ) if ( taxiom[i] && TL[i].two_test )
    (*(TL[i].two_test))();
  return true;
}



boolean find_twos_and_threes(unsigned info[], trs Tr)
{
  int i, j;

  FORALL(i) FORALL(j) possval[i][j] = info[impindex[i][j]];
  Tpt = Tr;

  if ( theJob->f_fus ) pretest_fus();

  for ( i = 1; i < AXMAX; i++ ) if ( taxiom[i] && TL[i].two_test )
    (*(TL[i].two_test))();

  for ( i = 1; i < AXMAX; i++ ) if ( taxiom[i] && TL[i].three_test )
    (*(TL[i].three_test))();

  return true;
}



void new_three_ref( int a, int x, int b, int y, int c, int z )
{
  int cells[4], vals[4];
  boolean valency[4];

  cells[0] = a;	vals[0] = x;	valency[0] = false;
  cells[1] = b;	vals[1] = y;	valency[1] = false;
  cells[2] = c;	vals[2] = z;	valency[2] = false;
  cells[3] = -1; vals[3] = 0;	valency[3] = false;
  new_small_ref(cells, vals, valency, Tpt);
}



/*
*	Where m is maximal, a->m is maximal for all a. Also, if m1 and 
*	m2 are maximal and different then a->m1 and a->m2 are different.
*/



void pretest_fus()
{
  int m1, m2, a, x;

  FORALL(m1) if ( maximal[m1] )
    for ( m2 = m1+1; m2 <= siz; m2++ ) if ( maximal[m2] )
      FORALL(x) if ( maximal[x] )
	FORALL(a) 
	  new_two_ref( impindex[a][m1], x, impindex[a][m2], x );
}


/*
*	For affixing, we need to look at possval, which is why this is here.
*	Report the incompatible pairs of values for a->b, c->d.
*/

void affix_case(int a, int b, int c, int d)
{
  int i, j;

  FORALL(i) if ( IN(i,possval[a][b]) )
    FORALL(j) if ( IN(j,possval[c][d]) )
      if ( !ord[i][j] )
	new_two_ref( impindex[a][b], i, impindex[c][d], j );
}




void test_assertion()
{
  int i, j;
  int a, b;

  FORALL(a) FORALL(j) if ( !ord[a][j] )
    FORALL(i) if ( a != i || j == i )
      FORALL(b) if ( IN(i,possval[a][b]) && IN(j,possval[i][b]) )
	new_two_ref( impindex[a][b], i, impindex[i][b], j );
}




void test_contraction()
{
  int i, j;
  int a, b;

  FORALL(i) FORALL(j) if ( !ord[j][i] )
    FORALL(b) if ( i != b )
      FORALL(a) if ( IN(i,possval[a][b]) )
	if ( IN(j,possval[a][i]) )
	  new_two_ref( impindex[a][b], i, impindex[a][i], j );
}
