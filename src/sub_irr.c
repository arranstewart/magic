/*				One.c
*
*	This uses RM (Read Matrices) to read ugly output from MaGIC and calls
*	a function "si" to choose those matrices which are subdirectly
*	irreducible under the connectives present in the fragment. The
*	matrices are printed out. The print format may be changed from the
*	default (UGLY) to PRETTY or TeX by means of the command line switches
*	"-p" or "-t".
*
*	For details of the matrix structures and other goodies defined in the
*	header, see RM.h. The following should be fairly self-explanatory,
*	however. Note that my_values and my_string are optional extras used
*	by printmat. These are re-initialised to <-1,-1,-1,....> and "" by
*	mat_malloc. Use strcpy to put a message in m->my_string.
*
*	Command line option -u (ugly) is allowed but has no effect since UGLY
*	is the default print mode.
*/



#include "RM.h"


int si(MATRIX *m);
int non_degenerate(MATRIX* m, int g1, int g2, int congruence[]);
void adjust(MATRIX *m, int congruence[], int from, int to);
int extended(MATRIX *m, int congruence[], int a, int b);
int monex(MATRIX *m, int arr[], int a, int b, int congruence[]);
int dyex(MATRIX *m, int arr[][SZ], int a, int b, int congruence[]);
int orthogonal(MATRIX *m, int theCongruences[][SZ]);



int main( argc, argv )
int argc;
char *argv[];
{
  int option;
  PRINTMODE p = UGLY;
  int g1();
  int getopt();

  while ((option = getopt (argc, argv, "upt")) != -1)
    switch( option ) {
    case 'p': p = PRETTY;	break;
    case 't': p = TeX;
    }
  selectmats(si,p);
  return 0;
}




int si(MATRIX *m)
{
  int theCongruences[2][SZ];
  int gen1a, gen1b, gen2a, gen2b;

  FORALL(m,gen1a) FORALL(m,gen1b) if (gen1b < gen1a) {
    if (non_degenerate(m,gen1a,gen1b,theCongruences[0])) {
      FORALL(m,gen2a) FORALL(m,gen2b) if (gen2b < gen2a) {
	if (non_degenerate(m,gen2a,gen2b,theCongruences[1]) &&
	    orthogonal(m,theCongruences))
	  return 0;
      }
    }
  }
  return(1);
}


int non_degenerate(MATRIX* m, int g1, int g2, int congruence[])
{
  int x, y, temp;

  for (x=0; x<SZ; x++)
    congruence[x] = -1;
  FORALL(m,x)
    congruence[x] = x;
  congruence[g2] = g1;
  
  /*
   * EXTEND THE CONGRUENCE TO A FIXED POINT
   */

  do {
    temp = 0;
    FORALL(m,x) FORALL(m,y)
      if (x < y && congruence[x] == congruence[y]) {
	if (extended(m,congruence,x,y))
	  temp = 1;
      }
  }
  while (temp);

  /*
   * Now renumber the congruence classes contiguously starting from 0,
   * in the order of their smallest-numbered members
   */

  temp = -1;
  FORALL(m,x)
    if (congruence[x] >= 0) {
      temp--;
      adjust(m,congruence,congruence[x],temp);
    }
  for (x=-2,y=0; x>=temp; x--,y++)
    adjust(m,congruence,x,y);
  if (y == 0)
    return 0;
  return 1;
}


void adjust(MATRIX *m, int congruence[], int from, int to)
{
  int x;

  FORALL(m,x)
    if (congruence[x] == from)
      congruence[x] = to;
}



int extended(MATRIX *m, int congruence[], int a, int b)
{
  int x;
  int ex = 0;

  if (m->fragment[NEG] &&
      monex(m,m->neg,a,b,congruence))
    ex = 1;
  if (m->fragment[BOX] &&
      monex(m,m->box,a,b,congruence))
    ex = 1;
  FORALLCON(m,x)
    if (m->adicity[x] == 1 &&
	monex(m,m->monadic[x],a,b,congruence))
      ex = 1;
  if (dyex(m,m->C,a,b,congruence))
    ex = 1;
  if (m->fragment[LAT] &&
      dyex(m,m->K,a,b,congruence))
    ex = 1;
  if (m->fragment[LAT] &&
      dyex(m,m->A,a,b,congruence))
    ex = 1;
  if ( m->fragment[FUS] &&
       dyex(m,m->fus,a,b,congruence))
    ex = 1;
  FORALLCON(m,x)
    if ( m->adicity[x] == 2 &&
	 dyex(m,m->dyadic[x],a,b,congruence))
      ex = 1;
  return ex;
}


int monex(MATRIX *m, int arr[], int a, int b, int congruence[])
{
  if (congruence[arr[a]] == congruence[arr[b]])
    return 0;
  adjust(m,congruence,congruence[arr[b]],congruence[arr[a]]);
  return 1;
}


int dyex(MATRIX *m, int arr[][SZ], int a, int b, int congruence[])
{
  int x;
  int ext = 0;

  FORALL(m,x) {
    if (congruence[arr[x][a]] != congruence[arr[x][b]]) {
      adjust(m,congruence,congruence[arr[x][b]],congruence[arr[x][a]]);
      ext = 1;
    }
    if (congruence[arr[a][x]] != congruence[arr[b][x]]) {
      adjust(m,congruence,congruence[arr[b][x]],congruence[arr[a][x]]);
      ext = 1;
    }
  }
  return ext;
}



int orthogonal(MATRIX *m, int theCongruences[][SZ])
{
  int x,y;

  FORALL(m,x)
    FORALL(m,y)
    if (x<y &&
	theCongruences[0][x] == theCongruences[0][y] &&
	theCongruences[1][x] == theCongruences[1][y])
      return 0;
  return 1;
}
