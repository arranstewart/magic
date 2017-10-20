/*
*			logic_set.c		V2.1 (May 1993)
*
*	The procedures going with set_poss() called from the 
*	search.  Also pre_set() called from job_initial().
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


static int vuloc[VMAX];         /* Local version of vu		*/
static int phi;                 /* The changeable subformula	*/
static int phicc;               /* Vector cell of phi		*/
static int phival;              /* The value of formula phi	*/




/*
*	Pre_set checks that such axioms as Excluded Middle are
*	satisfied, returning 0 (false) if not.  These principles
*	are the ones that can be checked without generating any
*	implication matrix, or even setting up a search space, 
*	so the search is avoided if they fail.
*/

boolean pre_set()
{
  int i,j;

  FORALL(i) {
    greater_than[i] = less_than[i] = 0;
    FORALL(j) {
      if ( ord[i][j] ) ADDTO(j,greater_than[i]);
      if ( ord[j][i] ) ADDTO(j,less_than[i]);
    }
    Greater_than[i] = (greater_than[i] & ~(1 << i));
    Less_than[i] = (less_than[i] & ~(1 << i));
  }

  for ( i = 1; i < AXMAX; i++ )
    if ( theJob->axiom[i] && TL[i].zero_test )
      if (!(*(TL[i].zero_test))()) return false;

  if ( theJob->f_T )
    FORALL(i)
      if ( !ord[i][siz] ) return false;
  if ( theJob->f_F )
    FORALL(i)
      if ( !ord[0][i] )	return false;

  for ( i = 0; theJob->croot[i][0]; i++ )
    if ( ! kost[i] )
      if ( ! utest(i,0) ) return false;

  return true;
}





/*
*	Utest returns true if the user-defined axiom or rule #x
*	is valid (preserves designation for all assignments to the
*	variables it involves).  It is assumed that changeable 
*	connectives are not involved.
*/

boolean utest(int x, unsigned info[])
{
  int i, j;

  if ( kost[x] > 1 ) return true;

  for ( i = 1; i <= Vmax; i++ ) {
    theJob->form[i].val = 0;
    vuloc[i] = 0;
  }
  phi = 0;
  phival = 0;
  FORALL(i) FORALL(j) C[i][j] = (ord[i][j]? des: undes);
  for ( i = 0; theJob->croot[x][i]; i++ )
    set_vuloc( theJob->croot[x][i], theJob->croot[x][i] );
  for ( i = 0; theJob->proot[x][i]; i++ )
    set_vuloc( theJob->proot[x][i], theJob->proot[x][i] );
  do	if ( badcase( x, info )) {
    if ( kost[x] ) REMOVE(phival,info[phicc]);
    else return false;
  }
  while	( anothercase(x) );
  return true;
}





/*
*	The following few functions implement the testing of zero
*	and one refutations in a rather crude but acceptable way.
*	First we record the variables used locally.
*/

void set_vuloc(int r, int rr)
{
  int	i;

  if ( r < 0 ) return;
  if ( r < VMAX ) vuloc[r] = 1;
  else {
    set_vuloc(theJob->form[r].lsub,rr);
    set_vuloc(theJob->form[r].rsub,rr);
    if ( r != rr && !strcmp(theJob->form[r].sym->s,"->") )
      phi = r;
    else if ( r != rr && !strcmp(theJob->form[r].sym->s,"o") )
      phi = r;
    else if ( !strcmp(theJob->form[r].sym->s,"!") )
      phi = r;
    else for ( i = 0; theJob->dcs[i]; i++ )
      if ( theJob->form[r].sym == theJob->dcs[i] ) {
	if ( theJob->defcon[i] == PRIMITIVE )
	  phi = r;
	else set_vuloc(theJob->defcon[i],rr);
      }
  }
}


/*
*	This tests the rule for one assignment of values to its
*	constituent atoms.  Note the returned value is 1 for a
*	failure of the rule and 0 for success.
*/

boolean badcase(int x, unsigned info[])
{
  int i;

  for ( i = 0; theJob->proot[x][i] > 0; i++ )
    if ( !desig[eval( theJob->proot[x][i], info )] ) return false;
  for ( i = 0; theJob->croot[x][i] > 0; i++ )
    if ( desig[eval( theJob->croot[x][i], info )] ) return false;
  return true;
}


/*
*	Eval returns the current value of the subformula rooted
*	at r.  This is not a fast way of testing, but it will do.
*	Note that C is set up so that true cases get designated 
*	values and false cases undesignated ones.
*/

int eval(int r, unsigned z[])
{
  int dr, Ls, Rs;
  symb c;

  if ( r < 1 ) skipout("Attempt to test rubbish",SKIP);
  c = theJob->form[r].sym;
  if ( is_var(c) ) return theJob->form[r].val;

  Ls = theJob->form[r].lsub;
  Rs = theJob->form[r].rsub;
  if ( !strcmp(c->s,"->") ) {
    if ( r == phi ) {
      phicc = impindex[eval(Ls,z)][eval(Rs,z)];
      while ( phival < siz && !IN(phival,z[phicc]) )
	phival++;
      return phival;
    }
    return C[eval(Ls,z)][eval(Rs,z)];
  }
  if ( !c->s[1] ) switch( c->s[0] ) {
  case 'T':
    return siz;
  case 'F':
    return 0;
  case 't':
    return des;
  case 'f':
    return N[des];
  case '&':
    return K[eval(Ls,z)][eval(Rs,z)];
  case 'v':
    return A[eval(Ls,z)][eval(Rs,z)];
  case 'o':
    if ( r == phi ) {
      phicc = impindex[eval(Ls,z)][N[eval(Rs,z)]];
      while ( phival < siz && !IN(phival,z[phicc]) )
	phival++;
      return N[phival];
    }
    return N[C[eval(Ls,z)][N[eval(Rs,z)]]];
  case '~':
    return N[eval(Rs,z)];
  case '!':
    if ( r == phi ) {
      phicc = boxindex[eval(Rs,z)];
      while ( phival < siz && 
	      !IN(phival,z[phicc]) )
	phival++;
      return phival;
    }
    return box[eval(Rs,z)];
  }
  for ( dr = 0; theJob->dcs[dr] != theJob->form[r].sym; dr++ ) ;
  if ( r == phi ) {
    switch( theJob->adicity[dr] ) {
    case 0:
      phicc = ucc0[dr];
      break;
    case 1:
      phicc = ucc1[dr][eval(Rs,z)];
      break;
    case 2:
      phicc = ucc2[dr][eval(Ls,z)][eval(Rs,z)];
    }
    while ( phival < siz && !IN(phival,z[phicc]) )
      phival++;
    return phival;
  }
  else {
    if ( theJob->adicity[dr] == 1 )
      theJob->form[atom[0][dr]].val = eval(Rs,z);
    else if ( theJob->adicity[dr] == 2 ) {
      theJob->form[atom[0][dr]].val = eval(Ls,z);
      theJob->form[atom[1][dr]].val = eval(Rs,z);
    }
    return eval( theJob->defcon[dr], z );
  }
}



int anothercase(int x)
{
  int i;

  if ( kost[x] ) {
    if ( phival == siz ) phival = 0;
    else return ++phival;
  }
  for ( i = 1; i <= Vmax; i++ ) if ( vuloc[i] ) {
    if ( theJob->form[i].val == siz ) theJob->form[i].val = 0;
    else return ++(theJob->form[i].val);
  }
  return 0;
}






/*
*	Set_poss is called from transref when a search space is
*	being initialised.  The vector info will contain (as bit
*	vectors) supersets of the possible values for each cell.
*	Set_poss may take values out but should on no account 
*	put values in.  Since transref is allowed to split up
*	the seasrch space and search bits of it separately, this
*	function should not be based on any assumption about what
*	values will be present as possibilities.
*
*	The first values to be pruned away are those which are 
*	larger than siz, the maximum value.  The arrow matrix C
*	should also have only designated (true) values in cells
*	a -> b where ord[a][b], and only undesignated values in
*	cells a -> b where not ord[a][b].
*
*	Logic_poss trims off more values according to the 
*	postulates of the chosen logic and any pre-defined axioms
*	which have been selected.
*
*	In affixing logics (all except FD), affixing and the 
*	existence of fusion force more values out.
*/

boolean set_poss(unsigned info[], trs T)
{
  int i, j, k, pr;
  unsigned mask;

  pr = 0;
  for ( i = 0; theJob->dcs[i]; i++ ) ;
  while ( i ) {
    if ( theJob->defcon[--i] == PRIMITIVE ) {
      switch ( theJob->adicity[i] ) {
      case 0:	co_priority( ucc0[i], pr, T );		
	break;
      case 1:
	FORALL(j) 
	  co_priority( ucc1[i][j], pr, T );
	break;
      case 2:
	FORALL(j) FORALL(k)
	  co_priority( ucc2[i][j][k], pr, T );
      }
      pr++;
    }
  }
  if ( theJob->f_nec )
    FORALL(i) co_priority( boxindex[i], pr, T );
  FORALL(i) FORALL(j) co_priority( impindex[i][j], pr+1, T );

  mask = 0;
  FORALL(i) ADDTO(i,mask);
  for ( i = 0; i < tr_par.vlength; i++ ) info[i] &= mask;

  FORALL(i) FORALL(j) FORALL(k)
    if IFF( (ord[i][j]),(!desig[k]) )
	    REMOVE(k,info[impindex[i][j]]);

  if ( !logic_poss(info) ) return 0;
  if ( theJob->f_fus ) fusion(info);
	
  for ( i = 0; theJob->croot[i][0]; i++ )
    if ( kost[i] == 1 ) utest( i, info );

  return 1; 
}






/*
*	Fusion requires that if m is any maximal element then any 
*	a->m is also maximal.
*/

void fusion(unsigned info[])
{
  int m, x, a;

  FORALL(m) if ( maximal[m] )
    FORALL(x) if ( !maximal[x] )
      FORALL(a)
	REMOVE(x,info[impindex[a][m]]);
}






/*
*	If permutation is selected, wherever ord[a][ b->c ] we must
*	have ord[b][ a->c ].  Hence if ord[a][x] for ALL or NONE of
*	the values for b->c, then we can remove from the values for
*	a->c any y such that !ord[b][y] or ord[b][y] respectively.
*/

boolean permutable(int a, int b, int c, unsigned info[])
{
  int k, m;

  k = impindex[a][c];  m = impindex[b][c];

  if ( (greater_than[a] & info[m]) == info[m] )
    info[k] &= greater_than[b];
  if ( !(greater_than[a] & info[m]) )
    info[k] &= ~greater_than[b];
  if ( (greater_than[b] & info[k]) == info[k] )
    info[m] &= greater_than[a];
  if ( !(greater_than[b] & info[k]) )
    info[m] &= ~greater_than[a];

  return( info[k] != 0 && info[m] != 0 );
}









/*
*	This function calls the above cases as required, according
*	to the axioms selected.  Some additional slightly messy 
*	features of C-type logics are additionally implemented.
*/

boolean logic_poss(unsigned info[])
{
  int i, j, k;

  if ( theJob->logic==R )
    TaT(info);
  if ( (theJob->logic==RW || theJob->logic==LIN)
       && theJob->f_n && theJob->f_lat )
    RWX(info);

  for ( i = 1; i < AXMAX; i++)
    if ( theJob->axiom[i] && TL[i].one_test )
      (*(TL[i].one_test))(info);

  if ( afx && ( theJob->axiom[AxC] || theJob->axiom[AxC2] ))
    FORALL(i) FORALL(j) if ( i < j )
      FORALL(k) 
	if ( !permutable( i, j, k, info ) )
	  return false;
  return (*info != 0);
}





/*
*	The axioms valid in the chosen logic are set or unset
*	here according to whether the parameter x is 1 or 0.
*/

#define TUNSET(axm) taxiom[axm] = 0

void logic_axioms(boolean x)
{
  AXIOM ax;

  if ( theJob->f_n && theJob->logic != FD )
    theJob->axiom[AxFN] = x;

  for ( ax = AxNull; ax < AXMAX; ax++ )
    if ( valid[theJob->logic][ax] )
      theJob->axiom[ax] = x;

  for ( ax = AxNull; ax < AXMAX; ax++ ) {
    if ( strchr(ax_string[ax],'~') && !theJob->f_n )
      theJob->axiom[ax] = 0;
    if ( strchr(ax_string[ax],'&') && !theJob->f_lat )
      theJob->axiom[ax] = 0;
    if ( strchr(ax_string[ax],'v') && !theJob->f_lat )
      theJob->axiom[ax] = 0;
    if ( strchr(ax_string[ax],'o') && !theJob->f_fus )
      theJob->axiom[ax] = 0;
    if ( strchr(ax_string[ax],'!') && !theJob->f_nec )
      theJob->axiom[ax] = 0;
    if ( strchr(ax_string[ax],'t') && !theJob->f_t )
      theJob->axiom[ax] = 0;
    if ( strchr(ax_string[ax],'f') && !(theJob->f_t && theJob->f_n) )
      theJob->axiom[ax] = 0;
    if ( strchr(ax_string[ax],'T') && !theJob->f_T )
      theJob->axiom[ax] = 0;
    if ( strchr(ax_string[ax],'F') && !theJob->f_F )
      theJob->axiom[ax] = 0;
  }

  if (x) efficient_logic_set();
}




/*
*	The above sets all the axioms valid in a particular logic
*	- for example, so that they are not printed.  For the search
*	however, it is much more efficient only to test a few of them
*	as selected by this function.
*/

void efficient_logic_set()
{
  AXIOM ax;

  for ( ax = AxNull; ax < AXMAX; ax++ )
    taxiom[ax] = theJob->axiom[ax];

  switch(theJob->logic) {
  case Null_logic:
  case FD:
  case B:
  case DW:
  case TW:
  case EW:
  case T:
  case LOGMAX:
    break;
  case R:
  case RW: 
  case LIN: 
  case CK:
    taxiom[AxB2] = false;
  case S4: 
  case E:
    taxiom[AxWB] = false;
    taxiom[AxB] = false;
    break;
  }

  if ( theJob->axiom[AxC] )
    taxiom[AxC2] = true;
}
