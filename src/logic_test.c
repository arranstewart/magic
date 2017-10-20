/*
*			logic_test.c		V2.1 (May 1993)
*
*	This is the "test" module for MaGIC.  Good_matrix is 
*	called from transref, and the rest of this file contains
*	only what depends on it (with the exception of functions
*	for removing isomorphs and for printing matrices, which
*	are in other files).
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



/*
*	The logic of Good_matrix, schematically, is as follows.
*
*		Translate the communication vector info into
*		testable data structures.
*		If these fail to satisfy some postulate
*			Report the refutation just found to the
*			search controller.
*			return FALSE.
*		endif
*		If the badguy (if any) fails
*			If the structure is not isomorphic
*			to any already generated
*				Print the matrix.
*			endif
*		endif
*		return TRUE.
*
*	The pre-defined axioms, including those defining the logic,
*	are tested first.  Then any user-defined ones are tested.
*
*	Before that, however, check to see that the break conditions
*	are not met.
*/

boolean Good_matrix(unsigned info[], trs T)
{
	int ti;  

	if ( theJob->maxmat && good == theJob->maxmat) tr_par.done = true;
	if ( tr_par.done ) return true;

	tot++; 
	vect_into_C(info);
  
	for ( ti = AxNull; ti < AXMAX; ti++ )
	if ( taxiom[ti] && TL[ti].many_test )
        if ( !(*(TL[ti].many_test))(T) )	return false;
	if ( !fus_test(T) )			return false;
	if ( !axtest(T) )			return false;
	if ( theJob->failure && !got_a_fail ) {
		no_ref( T );			return false;
	}
	if ( isomorphic(istak,T) )		return true;
	mat_print();
	return true;
}





/*
*	Translate the communication vector into testable data structures.
*/

void vect_into_C(unsigned info[])
{
	register int	
		i, j;
	int	k;

	FORALL(i) FORALL(j) C[i][j] = info[impindex[i][j]];
	if ( theJob->f_nec )
	FORALL(i) box[i] = info[boxindex[i]];
	for ( k = 0; theJob->dcs[k]; k++ )
	if ( theJob->defcon[k] == PRIMITIVE )
	switch( theJob->adicity[k] ) {
		case 0:	nulladic[k] = theJob->form[VMAX+4+k].val
			= info[ucc0[k]];
			break;
		case 1:	FORALL(i) monadic[k][i] = info[ucc1[k][i]];
			break;
		case 2:	FORALL(i) FORALL(j)
			dyadic[k][i][j] = info[ucc2[k][i][j]];
	}
	for (i = 0; i < tr_par.vlength; i++) thisvector[i] = info[i];
	if ( theJob->f_n && theJob->f_nec )
		FORALL(i) diamond[i] = N[box[N[i]]];
}





/*
*	Where fusion is stipulated to exist but is not definable,
*	it is necessary to test for it.  This tends to give large
*	refutations, so it is done as late as possible.
*
*	Defining fus[a][b] as the least x such that ord[a][C[b][x]],
*	check that this exists and is also least in the relation ord.
*/

boolean fus_test(trs T)
{ int i,j,k,m;

	if ( !theJob->f_fus ) return true;
	if ( theJob->axiom[AxC] && theJob->f_n ) {
		FORALL(i) FORALL(j) fus[i][j] = N[C[i][N[j]]]; 
		return true;
	}

	FORaLL(i) FORaLL(j) {
		FORALL(fus[i][j]) 
		if ( ord[i][C[j][fus[i][j]]] ) break;
		if ( fus[i][j] > siz ) {
			FORALL(k) Ref( impindex[j][k], T );
			return false;
		}
		for ( k = fus[i][j]+1; k <= siz; k++ )
		if ( ord[i][C[j][k]] && !ord[fus[i][j]][k] ) {
			for ( m = 0; m <= fus[i][j]; m++ )
			Ref( impindex[j][m], T );
			Ref( impindex[j][k], T );
			return false;
		}
	}
	return true;
}





/*
*	Eventually it is necessary to test any user-defined axioms
*	and rules there may be.  This is much slower than testing
*	pre-defined ones.
*
*	The test given here is the stupidest imaginable, simply
*	making all assignments of values to variables and working
*	out the consequent values of all subformulas in each case.
*	It works fairly well, however, except when there are many
*	variables in use.  Then its stupidity trips it up badly, so
*	there is a case for using a more sophisticated algorithm 
*	where the task is large.
*/

boolean axtest(trs T)
{
	WFF *w;
	int i, j;

	got_a_fail = 0;
	if ( !**(theJob->croot) && !theJob->failure ) return true;
	if ( *(theJob->defcon) ) setcon();
	for ( i = 0; i < CMAX; i++ ) {
		theJob->form[atom[0][i]].val = 0;
		theJob->form[atom[1][i]].val = 0;
	}
	theJob->form[0].val = 0;
	for ( i = 1; i <= Vmax; i++ ) theJob->form[i].val = siz;

WORK:	for ( w = theJob->form+VMAX+4+(CMAX*3); w != tx; w++ )
	w->val = *(w->mtx + *(w->lv)*SZ + *(w->rv));

	for ( i = 0; theJob->croot[i][0]; i++ ) if ( kost[i] > 1 ) {
		if ( theJob->proot[i][0] != TRIVIAL ) {
			for ( j = 0; theJob->proot[i][j]; j++ )
			if ( !desig[theJob->form[theJob->proot[i][j]].val] )
				goto AX_OK;
		}
		if ( theJob->croot[i][0] != ABSURD ) {
			for ( j = 0; theJob->croot[i][j]; j++ )
			if ( desig[theJob->form[theJob->croot[i][j]].val] )
				goto AX_OK;
		}
		for ( j = 0; theJob->proot[i][j]; j++ )
		set_used( theJob->proot[i][j], T, true );
		for ( j = 0; theJob->croot[i][j]; j++ )
		set_used( theJob->croot[i][j], T, true );
		return false;
AX_OK:		;
	}
	if ( theJob->failure && !desig[theJob->form[theJob->failure].val] ) {
		got_a_fail = 1;
		for ( i = 1; i <= Vmax; i++ ) 
		badvalue[i] = rvu[i]? theJob->form[i].val: SZ;
	}

	for ( i = 1; i <= Vmax; i++ ) 
	if ( vu[i] )  {       
		if ( theJob->form[i].val ) {
			theJob->form[i].val--; 
			goto WORK;
		}
		theJob->form[i].val = siz;
	}
       
	return true;
}





/*
*	Find the appeals to changeable values in subformula #x.
*	Topper is a flag saying whether this is the outermost
*	level (so that implications at that level can be read
*	as appeals to ord).
*/

void set_used(int x, trs T, boolean topper)
{
	int	i;
	WFF	*wf;

	if ( x <= 0 ) return;
	wf = theJob->form+x;

	set_used( wf->lsub, T, false );
	set_used( wf->rsub, T, false );

	if ( !strcmp(wf->sym->s,"->") && !topper ) 
		Ref( impindex[*(wf->lv)][*(wf->rv)], T );
	else if ( !strcmp(wf->sym->s,"!") )
		Ref( boxindex[*(wf->rv)], T );
	else if ( !strcmp(wf->sym->s,"?") )
		Ref( boxindex[N[*(wf->rv)]], T );
	else if ( !strcmp(wf->sym->s,"o") ) {
		if ( theJob->f_n && valid[theJob->logic][AxC] )
			Ref( impindex[*(wf->lv)][N[*(wf->rv)]], T ); 
		else for ( i = 0; i <= wf->val; i++ ) 
			Ref( impindex[*(wf->rv)][i], T );
	}
	else for ( i = 0; theJob->dcs[i]; i++ ) 
	if ( theJob->dcs[i] == wf->sym ) {
		if ( theJob->defcon[i] == PRIMITIVE )
		switch( theJob->adicity[i] ) {
			case 0:	Ref( ucc0[i], T );
				break;
			case 1:	Ref( ucc1[i][*(wf->rv)], T );
				break;
			case 2:	Ref( ucc2[i][*(wf->lv)][*(wf->rv)], T );
				break;
		}
		else {
			if ( theJob->adicity[i] == 2 ) {
				theJob->form[atom[0][i]].val
				= *(wf->lv);
				theJob->form[atom[1][i]].val
				= *(wf->rv);
			}
			else if ( theJob->adicity[i] == 1 ) 
			theJob->form[atom[0][i]].val = *(wf->rv);
			set_used(theJob->defcon[i], T, false);
		}
		break;
	}
	if ( wf->rsub )
	wf->val = *(wf->mtx + ((*(wf->lv))*SZ + *(wf->rv)));
}

 



/*
*	The routine to derive matrices for defined connectives
*	calls the recursive getval as required.
*/

void setcon()
{
	int	i, j, k, m;

	for ( i = 0; theJob->defcon[i]; i++ )
	if ( theJob->defcon[i] != PRIMITIVE )
	switch(theJob->adicity[i]) {
		case 0:
		nulladic[i] = theJob->form[VMAX+4+i].val = 
			getval(theJob->defcon[i]); 
		break;
		case 1:
		FORALL(j) {
			for ( m = 0; m < CMAX; m++ )
			theJob->form[atom[0][m]].val = j;
			monadic[i][j] = getval(theJob->defcon[i]);
		}
		break;
		case 2:
		FORALL(j) {
			for  (m = 0; m < CMAX; m++ )
			theJob->form[atom[0][m]].val = j;
			FORALL(k) {
				for ( m = 0; m < CMAX; m++ )
				theJob->form[atom[1][m]].val = k;
				dyadic[i][j][k] = getval(theJob->defcon[i]);
			}
		}
	}
}





/*
*	Recursively generate the value of a subformula.
*/

int getval(int y)
{
	WFF *x;

	x = theJob->form + y;

	if ( x->rsub < 1 )
		return x->val;
	if ( x->lsub < 1 )
		return *(x->mtx + getval(x->rsub));
	return *(x->mtx + SZ*getval(x->lsub) + getval(x->rsub));
}
