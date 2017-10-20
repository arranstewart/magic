
#include "MaGIC.h"

/*
*	This file contains the code for testing each axiom. There are 
*	four types of case. First are axioms which do not depend on the
*	contents of the implication, or box matrices at all. These give 
*	rise to 0-refutations. That is, they are either true or false 
*	of the extensional setup. They can be tested before entry to the 
*	search routine. An example is the law of the excluded middle
*	A v ~A which depends only on the lattice, the negation operation 
*	and the choice of designated values.
*
*	The second group consists of axioms such as A & (A->B) -> B, 
*	which require one reference to the implication or box matrix.
*	These are tested at the stage of setting up the search space
*	and secured by taking out the impossible values (such as any x
*	for cell [A->B] such that not A&x ord B).
*
*	The third group consists of axioms which require a small number 
*	of matrix references. `Small' means 2 in MaGIC at present. An 
*	example is the S4 axiom !A -> !!A, any failure of which requires 
*	values x for !A and y for !x such that not x ord y.
*
*	Finally, there are a few cases in which multiple matrix entries 
*	are involved. These are treated by `lazy constraint generation' 
*	at the testing stage after candidate matrices have been found.
*/


/*
*	GROUP 0 (0-refutations)
*/

boolean f_arrow_t()		/***   f -> t   ***/
{
  return ( ord[N[des]][des] );
}

boolean Exmid()			/***   a v ~a   ***/
{
  int a;

  FORALL(a)
    if ( !desig[A[a][N[a]]]) return false;
  return true;
}


boolean Boolalg()		/***   a&~a -> b   ***/
{
  int a;

  FORALL(a)
    if ( A[a][N[a]] != siz ) return false;
  return true;
}


boolean SemiBool()		/***   a&~a -> bv~b   ***/
{
  int a, b;

  FORALL(a) FORALL(b)
    if ( !ord[K[a][N[a]]][A[b][N[b]]] ) return false;
  return true;
}


boolean paradox()		/***   des = T   ***/
{
  int a;

  FORALL(a) if ( !ord[a][des] ) return false;
  return true;
}


/**********************************************************************/

/*
*	GROUP 1 (1-refutations)
*
*	The parameter here is the vector of possible-value sets.
*	The general form of the procedure is
*		for each relevant possible-value set s do
*			for each impossible value x of s do
*				remove x from s
*			end for
*		end for
*	The vector is indexed by impindex and boxindex. Impindex[x][y]
*	picks out the offset of the vector entry corresponding to the 
*	implication [x->y] while boxindex[x] picks out that corresponding 
*	to [!x]. REMOVE is a macro for taking a value out of a set.
*
*	Note that no value needs to be returned by these operations, as 
*	only the side effect of diminishing the possible-value sets is 
*	intended.
*/

void E_assertion(unsigned info[])	/***   t->a -> a   ***/
{
  int a, b;

  FORALL(b) if ( desig[b] )
    FORALL(a)
      info[impindex[b][a]] &= less_than[a];
}


void contraction(unsigned info[])	/***  (a->.a->b) ->. a->b  ***/
{
  int a, b, c;

  if ( theJob->f_lat )	Wstar(info);
  if ( theJob->f_n )	Reductio(info);

  FORALL(a) FORALL(b) if ( !ord[a][b] )
    FORALL(c) if ( ord[a][c] )
      REMOVE(c,info[impindex[a][b]]);
}


void Wstar(unsigned info[])	/***   a & (a->b) -> b   ***/
{
  int a, b, c;

  FORALL(a) FORALL(b) FORALL(c)
    if ( !ord[K[a][c]][b] )
      REMOVE(c,info[impindex[a][b]]);
}


void Reductio(unsigned info[])	/***   a->~a -> ~a   ***/
{
  int a, b;

  if ( theJob->f_lat ) Exmid();
  FORALL(a) FORALL(b)
    if ( !ord[b][N[a]] )
      REMOVE(b,info[impindex[a][N[a]]]);
}


void assertion(unsigned info[])	/***   0->a = T,   t->a = a   ***/
{
  int a, b;

  FORALL(a) FORALL(b) {
    if ( b < siz ) REMOVE(b,info[impindex[0][a]]);
    if ( b != a ) REMOVE(b,info[impindex[des][a]]);
  }
}


void ata(unsigned info[])	/***   a ->. t->a   ***/
{
  int a;

  FORALL(a)
    info[impindex[des][a]] &= greater_than[a];
}


void TaT(unsigned info[])		/***   T ->. a->T   ***/
{
  int a, b;

  FORALL(a) if ( !ord[a][siz] ) {
    *info = 0;
    return;
  }
  FORALL(b) if ( b < siz )
    FORALL(a)
      REMOVE(b,info[impindex[a][siz]]);
}


void mingle(unsigned info[])		/***   a ->. a->a   ***/
{
  int a;

  FORALL(a)
    info[impindex[a][a]] &= greater_than[a];
}


void t_atomic(unsigned info[])		/***   p v (p -> q)   ***/
{
  int a, b, c;

  FORALL(a) FORALL(c) if ( !desig[A[a][c]] ) 
    FORALL(b)
      REMOVE(c,info[impindex[a][b]]);
}


void FF_T(unsigned info[])		/***   T ->. F -> F   ***/
{
  int a, b;

  FORALL(a)
    if ( !ord[0][a] || !ord[a][siz] ) *info = 0;

  FORALL(b) if ( b < siz )
    FORALL(a) 
      REMOVE(b,info[impindex[0][a]]);

  if ( F_N || theJob->axiom[AxB] || theJob->axiom[AxB2] )
    FORALL(a) FORALL(b)
      if ( b < siz )
	REMOVE(b,info[impindex[a][siz]]);
}


void PARADOX(unsigned info[])		/***   a ->. b->a   ***/
{
  int a, b;

  if ( !paradox() ) *info = 0;
  FORALL(a)  FORALL(b)
    info[impindex[a][b]] &= greater_than[b];
}


void RWX(unsigned info[])		/*** If LEM, a+1 & (a+1->0) = 0 ***/
{
  int a, b;

  FORALL(a)
    if ( !desig[A[a][N[a]]] ) return;

  FORALL(a) if ( a )
    FORALL(b) if ( K[a][b] )
      REMOVE(b,info[impindex[a][0]]);
}


void necessity(unsigned info[])		/* !a implies a */
{
  int a, x;

  FORALL(a)
    FORALL(x) if ( !ord[x][a] )
      REMOVE(x,info[boxindex[a]]);
}


void necessitation(unsigned info[])	/* des(a) ==> des(!a) */
{
  int a, x;

  FORALL(a) if ( desig[a] )
    FORALL(x) if ( !desig[x] )
      REMOVE(x,info[boxindex[a]]);
}


void NKI_test(unsigned info[])		/* des(a) ==> !b implies a */
{
  int a, b, x;

  FORALL(a) if ( desig[a] )
    FORALL(x) if ( !ord[x][a] )
      FORALL(b)
	REMOVE(x,info[boxindex[b]]);
}

/*
*	This affixing check takes considerable time, but pays.
*/

void set_prefix(unsigned info[])
{
  int i, j, k;

  FORALL(i)  FORALL(j)
    for ( k = j+1; k <=siz; k++ )
      if ( ord[j][k] && !( F_N && j < N[k] ))
	if ( !(Greater_than[j] & Less_than[k]) )
	  squeeze( info, i, j, i, k );
}

void set_suffix(unsigned info[])
{
  int i, j, k;

  FORALL(i)  FORALL(j)
    for ( k = 0; k < i; k++ )
      if ( ord[k][i] && !( F_N && i < N[k] ))
	if ( !(Greater_than[k] & Less_than[i]) )
	  squeeze( info, i, j, k, j );
}

/*
*	Squeeze, a subroutine of the above, squeezes out of the
*	search space any values which are incompatible with this
*	particular case of affixing: ord[ a->b ][ c->d ].
*/

void squeeze(unsigned info[], int a, int b, int c, int d)
{ 
  int i;
  int k, m;

  k = impindex[a][b]; m = impindex[c][d];

  FORALL(i) {
    if ( IN(i,info[k]) ) {
      if ( !(greater_than[i] & info[m]) )
	REMOVE(i,info[impindex[a][b]]);
    }
    if ( IN(i,info[m]) ) {
      if ( !(less_than[i] & info[k]) )
	REMOVE(i,info[impindex[c][d]]);
    }
  }
}


/**********************************************************************/

/*
*	GROUP 2 (2-refutations)
*/

void pretest_prefix()
{
  int i, j, k;

  FORALL(i) FORALL(j) if ( !( F_N && i < N[j] ))
    for ( k = siz; k > j; k-- ) if ( ord[j][k] )
      if ( !(Greater_than[j] & Less_than[k]) )
	affix_case( i, j, i, k );
}


void pretest_suffix()
{
  int i, j, k;

  FORALL(i) FORALL(j) if ( !( F_N && i < N[j] ))
    for ( k = 0; k < i; k++ ) if ( ord[k][i] )
      if ( !(Greater_than[k] & Less_than[i]) )
	affix_case( i, j, k, j );
}


void test_S4axiom()
{
  int a, x, y;

  FORALL(x)
    FORALL(y) if ( !ord[x][y] )
      FORALL(a)
	new_two_ref( boxindex[a], x, boxindex[x], y );
}


void test_S5axiom()
{
  int a, x, y;

  FORALL(a)
    FORALL(x) if ( !ord[a][x] )
      FORALL(y)
	new_two_ref( boxindex[N[a]], y, boxindex[N[y]], x );
}


void test_Daxiom()
{
  int a, x, y;

  FORALL(x)
    FORALL(y) if ( !ord[x][N[y]] )
      FORALL(a)
	new_two_ref( boxindex[a], x, boxindex[N[a]], y );
}


void test_NK()
{
  int a, b, x, y;

  FORALL(a)
    FORALL(x) if ( !ord[a][x] )
      FORALL(b)
	FORALL(y)
	new_two_ref( boxindex[b], y, impindex[y][a], x );
}



/**********************************************************************/

/*
*	GROUP 3 (many-refutations)
*
*	The general form of the axiom test is
*		for each assignment of values to the variables
*			if the axiom fails
*				call Ref with each cell used
*				return false
*			endif
*		endfor
*		return true
*/


boolean test_mslat(trs T)
{
  int a, b, c;

  FORaLL(b)
    for ( c = siz-1; c > b; c-- )
      if ( !ord[b][c] )
	FORaLL(a) 
	  if ( K[C[a][b]][C[a][c]] != C[a][K[b][c]] ) {
	    Ref( impindex[a][b], T );
	    Ref( impindex[a][c], T );
	    Ref( impindex[a][K[b][c]], T );
	    return false;
	  }
  return true;
}



boolean test_jslat(trs T)
{
  int a, b, c;

  FORaLL(a)
    for ( b = siz-1; b > a; b-- )
      if ( !ord[a][b] )
	FORaLL(c) 
	  if ( K[C[a][c]][C[b][c]] != C[A[a][b]][c] ) {
	    Ref( impindex[a][c], T );
	    Ref( impindex[b][c], T );
	    Ref( impindex[A[a][b]][c], T );
	    return false;
	  }
  return true;
}
	


boolean Btest(trs T)
{
  int a, b, c;

  FORaLL(a) FORaLL(b) FORaLL(c)
    if ( !ord[C[a][b]] [C[C[c][a]][C[c][b]]] ) {
      Ref(impindex[a][b], T);
      Ref(impindex[c][a], T);
      Ref(impindex[c][b], T);
      Ref(impindex[C[c][a]][C[c][b]], T);
      return false;
    }
  return true;
}

/**********************************/

boolean B2test(trs T)
{
  int a, b, c;

  FORaLL(a) FORaLL(b) FORaLL(c)
    if ( !ord[C[a][b]] [C[C[b][c]][C[a][c]]] ) {
      Ref(impindex[a][b], T);
      Ref(impindex[b][c], T);
      Ref(impindex[a][c], T);
      Ref(impindex[C[b][c]][C[a][c]], T);
      return false;
    }
  return true;
}

/**********************************/

boolean Stest(trs T)
{
  int a, b, c;

  FORaLL(a) FORaLL(b) FORaLL(c)
    if ( !ord[C[a][C[b][c]]] [C[C[a][b]][C[a][c]]] ) {
      Ref(impindex[a][b], T);
      Ref(impindex[b][c], T);
      Ref(impindex[a][c], T);
      Ref(impindex[a][C[b][c]], T);
      Ref(impindex[C[a][b]][C[a][c]], T);
      return false;
    }
  return true;
}

/*********************************/

boolean Ctest(trs T)
{
  int a, b, c;

  FORaLL(a) FORaLL(c) if ( a < c )
    FORaLL(b)
      if ( C[c][C[a][b]] != C[a][C[c][b]] ) {
	Ref(impindex[a][b], T);
	Ref(impindex[c][b], T);
	Ref(impindex[a][C[c][b]], T);
	Ref(impindex[c][C[a][b]], T);
	return false;
      }
  return true;
}

/*********************************/

boolean WBtest(trs T)
{
  int a, b, c;

  FORaLL(a) FORaLL(b) FORaLL(c)
    if ( !ord[K[C[a][b]][C[b][c]]] [C[a][c]] ) {
      Ref(impindex[a][b], T);
      Ref(impindex[b][c], T);
      Ref(impindex[a][c], T);
      return false;
    }
  return true;
}

/*********************************/

boolean NecImpDist(trs T)
{
  int a, b;

  FORaLL(a) FORaLL(b)
    if ( !ord[box[C[a][b]]] [C[box[a]][box[b]]] ) {
      Ref(impindex[a][b], T);
      Ref(impindex[box[a]][box[b]], T);
      Ref(boxindex[a], T);
      Ref(boxindex[b], T);
      Ref(boxindex[C[a][b]], T);
      return false;
    }
  return true;
}

/*********************************/

boolean NecAdj(trs T)
{
  int a, b;

  FORaLL(a) FORaLL(b)
    if ( !ord[K[box[a]][box[b]]] [box[K[a][b]]] ) {
      Ref(boxindex[a], T);
      Ref(boxindex[b], T);
      Ref(boxindex[K[a][b]], T);
      return false;
    }
  return true;
}

/*********************************/

boolean NecW(trs T)
{
  int a, b;

  FORaLL(a) FORaLL(b)
    if ( !ord[C[box[a]][C[box[a]][b]]] [C[box[a]][b]] ) {
      Ref(boxindex[a], T);
      Ref(impindex[box[a]][b], T);
      Ref(impindex[box[a]][C[box[a]][b]], T);
      return false;
    }
  return true;
}

/*********************************/
