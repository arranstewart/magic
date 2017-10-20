/*
*			isom.c		V2.1 (May 1993)
*
*	This contains all the procedures pertaining to the
*	elimination of isomorphic copies from the output of
*	matrix-generating programs such as MaGIC.c.
*
*	It is assumed that the external variables siz, neg[], 
*	ord[][], des, C[][] and box[] are available and that 
*	this module is called, presumably by the accept() 
*	function, every time a new matrix is found.  If there 
*	are any isomorphic copies, the integer isoms is 
*	incremented.
*	 
*	There are two parts to all this.  First come some
*	procedures to find the acceptable permutations on the
*	current siz, neg[], ord[][] and des.  Then there are
*	the routines to be executed on discovery of a new 
*	matrix.
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


#define firstopen (tr_par.vlength - tr_par.vlength)

static boolean
	manyisoms;		/* Not all isoms have been stacked	*/

static ism
	nextisom;		/* First free isom in the list		*/

static int
	lastisovect;		/* Last unfree segment of isolist	*/

static int
	isoused;		/* Number of isoms used this case	*/

static char
	shvector[V_LENGTH];	/* The variable part of thisvector	*/



/*
*	This initialisation function just sets up perm.
*/

void perm_initial()
{
  perm = (PRM*) malloc(sizeof(PRM));
  perm->h[0] = -1;
  perm->pup = 0;
  isoused = ISOMAX;
}




/*
*	Setperm finds the automorphisms which do not disturb 
*	ord, N or des.  The bulk of it is a crude test-and-change.
*/

void setperm()
{
  int i, j;
  int p_poss[SZ][SZ],
    p_used[SZ],
    sh[SZ],
    single[SZ];
  PRM *prmptr;

  for (i=0; i<isoused; i++) {
    istak[i].icv = (i? 0: isolist);
    istak[i].left = istak[i].right = 0;
    if ( !i || i == ISOMAX-1 ) istak[i].parent = 0;
    else istak[i].parent = istak+i+1;
  }
  isoused = 1;
  nextisom = istak+1;
  for ( i = 0; i < V_LENGTH; i++ ) thisvector[i] = -1;
  manyisoms = false;
  lastisovect = 0;
  istak[0].icv[0] = SZ;

  for ( prmptr = perm; prmptr->h[0] >= 0; prmptr = prmptr->pup )
    prmptr->h[0] = -1;

  FORALL(i) FORALL(j) {
    p_poss[i][j] = (lower_than(i)==lower_than(j)) &&
      (higher_than(i)==higher_than(j));
    if ( theJob->f_n && 
	 (( N[i]==i && N[j]!=j ) || ( N[j]==j && N[i]!=i )))
      p_poss[i][j] = 0;
  }

  FORALL(i) {
    if ( theJob->f_t ) {
      if ( i != des ) 
	p_poss[des][i] = p_poss[i][des] = 0;
      if ( theJob->f_n && i != N[des] ) 
	p_poss[N[des]][i] = p_poss[i][N[des]] = 0;
    }
    else if ( desig[i] ) 
      FORALL(j) if ( !desig[j] ) 
	p_poss[i][j] = p_poss[j][i] = 0;
    p_used[i] = 1;  sh[i] = i;
    single[i] = 1;
    FORALL(j)
      if ( i != j && p_poss[i][j] )
	single[i] = 0;
  }
  goto CHANGE;	/* !!! */

PTEST:	FORALL(i) FORALL(j)
	  if ( ord[i][j] != ord[sh[i]][sh[j]] )
	    goto CHANGE;
  newperm(sh);

CHANGE:	FORALL(i)
	  if ( !(( theJob->f_n && i < N[i] ) || single[i] )) {
	    p_used[sh[i]] = 0;
	    if ( theJob->f_n )
	      p_used[sh[N[i]]] = 0;
	    for ( j = sh[i]-1; j >= 0; j-- )
	      if ( p_poss[i][j] && !p_used[j] ) {
		sh[i] = j;
		if ( theJob->f_n )
		  sh[N[i]] = N[j];
		p_used[j] = 1;
		if ( theJob->f_n )
		  p_used[N[j]] = 1;
		for (i--; i >= 0 && !(theJob->f_n && i < N[i]); i--) 
		  if ( !single[i] ) {
		    for (j = siz; j; j-- )
		      if ( p_poss[i][j] && !p_used[j] ) {
			sh[i] = j;
			if ( theJob->f_n )
			  sh[N[i]] = N[j];
			p_used[j] = 1;
			if ( theJob->f_n)
			  p_used[N[j]] = 1;
			break;
		      }
		  }
		goto PTEST;
	      }
	  }
}




/*
*	How many elements are below x?
*/

int lower_than(int x)
{
  int i;
  int j = 0;

  for ( i=0; i < x; i++ ) j += ord[i][x];
  return j;
}


/*
*	Similarly, how many are above?
*/

int higher_than(int x)
{
  int i;
  int j = 0;

  for ( i = siz; i > x; i-- ) j += ord[x][i];
  return j;
}





/*
*	Add a permutation to the perm list.
*/

void newperm(int *vec)
{
  int i;
  PRM *p;

  for ( p = perm; p->pup && p->h[0] >= 0; p = p->pup) ;
  if ( !p->pup ) {
    p->pup = (PRM*) malloc(sizeof(PRM));
    p->pup->pup = 0;
    p->pup->h[0] = -1;
  }
  FORALL(i) p->h[i] = *(vec+i);
}





/*
*	That completes the first half, concerned with generating 
*	the list of allowable permutations.  Now, assuming that 
*	list given, deal with isomorphisms between good matrices.
*
*	Isomorphic usually returns 1 if the matrix was already in the 
*	isomorphism tree below ptr, snipping it out as it does so
*	(since it is not going to be generated twice).  If not, 
*	all the isomorphic copies which lie within the search space
*	are generated and added to the tree.
*
*	If the flag "manyisoms" is set, indicating that some isomorphic
*	copies of earlier things have not been recorded because of lack
*	of space, then the copies of the current matrix, instead of 
*	being added directly to the tree, are compared with the current 
*	matrix to see whether one of them preceeds it.  If the current 
*	matrix is the first in its isomorphism class, its copies are 
*	inserted in the tree.  If not, the temporary store is emptied 
*	and 1 returned.
*
*	The flag "manyisoms" is raised if the number of stored copies
*	gets to equal ISOMAX, and lowered at the start of each search.
*/

boolean isomorphic(ism ptr, trs T)
{
  int i, j;
  boolean do_add = false;

  if ( !tr_par.vlength ) return false;
  j = 0;
  for ( i = 0; i < tr_par.vlength; i++ )
    if ( T->coinorder[i] >= firstopen )
      shvector[j++] = thisvector[i];

  for ( i = 0; i < tr_par.vlength; i++ ) {
    if ( shvector[i] < ptr->icv[i] ) {
      if ( ptr->left ) 
	return isomorphic(ptr->left,T);
      do_add = true;
      break;
    }
    if ( shvector[i] > ptr->icv[i] ) {
      if ( ptr->right ) 
	return isomorphic(ptr->right,T);
      do_add = true;
      break;
    }
  }
  if ( do_add ) {
    if ( isomorphic_anyhow(T) ) {
      isoms2++;
      return true;
    }
    add_isoms( T );
    return false;
  }
  snip(ptr);
  isoms++;
  return true;
}





/*
*	Snip removes a node from the isom tree.
*	The action is slightly different if the node is the root.
*/

void snip(ism p)
{
  ism q;
  int i;

  if ( p == istak ) {
    if ( !p->left && !p->right ) {
      *(p->icv) = SZ;
    }
    else {
      if ( p->right ) 
	for ( q = p->right; q->left; q = q->left ) ;
      else	for ( q = p->left; q->right; q = q->right ) ;
      for ( i = 0; i < tr_par.vlength; i++ )
	p->icv[i] = q->icv[i];
      snip(q);
    }
  }
  else {
    if ( !p->left )
      subst(p,p->right);
    else if ( !p->right )
      subst(p,p->left);
    else {
      for ( q = p->right; q->left; q = q->left ) ;
      for ( i = 0; i < tr_par.vlength; i++ )
	p->icv[i] = q->icv[i];
      subst(q,q->right);
    }
  }
}





/*
*	A subroutine of the above, this moves p2 into the place
*	of p1, marking p1 as defunct so that it can be re-used.
*/

void subst(ism p1, ism p2)
{
  if ( p2 )
    p2->parent = p1->parent;
  if ( p1->parent->left == p1 )
    p1->parent->left = p2;
  else	p1->parent->right = p2;
  *(p1->icv) = SZ;
  p1->parent = nextisom;
  nextisom = p1;
}




/*
*	Isomorphic_anyhow if manyisoms and this is not the first
*	in its isomorphism class.  This function and add_isoms are
*	very similar and should probably be amalgamated at some stage.
*/

boolean isomorphic_anyhow(trs T)
{
  int i, j, k;
  char ac[V_LENGTH];
  PRM *aptr;

  if ( !manyisoms ) return false;
  for ( aptr = perm; aptr->h[0] >= 0; aptr = aptr->pup ) {
    FORALL(i) FORALL(j) 
      ac[impindex[(int)aptr->h[i]][(int)aptr->h[j]]] = aptr->h[C[i][j]];
    FORALL(i)
      ac[boxindex[(int)aptr->h[i]]] = aptr->h[box[i]];
    for ( i = 0; theJob->dcs[i]; i++ )
      if ( theJob->defcon[i] == PRIMITIVE )
	switch ( theJob->adicity[i] ) {
	case 0:
	  ac[ucc0[i]] = aptr->h[nulladic[i]];
	  break;
	case 1:
	  FORALL(j)
	    ac[ucc1[i][(int)aptr->h[j]]] =
	    aptr->h[monadic[i][j]];
	  break;
	case 2:
	  FORALL(j) FORALL(k)
	    ac[ucc2[i][(int)aptr->h[j]][(int)aptr->h[k]]] =
	    aptr->h[dyadic[i][j][k]];
	}
    for ( i = firstopen; i < tr_par.vlength; i++ )
      if ( thisvector[T->inorder[i]] != ac[T->inorder[i]] ) {
	if ( thisvector[T->inorder[i]] > ac[T->inorder[i]] )
	  return true;
	break;
      }
  }
  return false;
}






/*
*	Add_isoms, called from isomorphic, generates all the non-
*	trivial automorphic copies of the matrix got by applying 
*	allowable permutations and puts them into the "isom" tree.
*/

void add_isoms(trs T)
{
  int i, j, k, same;
  char ac[V_LENGTH], shac[V_LENGTH];
  PRM *aptr;

  for ( aptr = perm;
	aptr->h[0] >= 0 && nextisom && 
	  lastisovect < ISLMAX - tr_par.vlength;
	aptr = aptr->pup ) {
    FORALL(i) FORALL(j) 
      ac[impindex[(int)aptr->h[i]][(int)aptr->h[j]]] = aptr->h[C[i][j]];
    if ( theJob->f_nec )
      FORALL(i)
	ac[boxindex[(int)aptr->h[i]]] = aptr->h[box[i]];
    for ( i = 0; theJob->dcs[i]; i++ )
      if ( theJob->defcon[i] == PRIMITIVE )
	switch ( theJob->adicity[i] ) {
	case 0:	ac[ucc0[i]] = aptr->h[nulladic[i]];
	  break;
	case 1:
	  FORALL(j)
	    ac[ucc1[i][(int)aptr->h[j]]] =
	    aptr->h[monadic[i][j]];
	  break;
	case 2:
	  FORALL(j) FORALL(k)
	    ac[ucc2[i][(int)aptr->h[j]][(int)aptr->h[k]]] =
	    aptr->h[dyadic[i][j][k]];
	}
    same = true;
    j = 0;
    for ( i = 0; i < tr_par.vlength; i++ ) 
      if ( T->coinorder[i] >= firstopen ) {
	shac[j] = ac[i];
	if ( shac[j] != shvector[j] ) same = false;
	j++;
      }
    if ( !same ) {
      if ( *((*(istak)).icv) != SZ ) {
	add_this(shac,istak);
	if ( !nextisom || lastisovect >= ISLMAX - tr_par.vlength )
	  manyisoms = true;
      }
      else
	for ( i = 0; i < tr_par.vlength; i++ )
	  (*(istak)).icv[i] = shac[i];
    }
  }
}





/*
*	If the matrix mat is not in the i_tree, put it in and return true.
*	If it is there already, return false.
*/

boolean add_this(char mat[], ism i_tree)
{
  int i;

  for ( i = 0; i < tr_par.vlength; i++ )
    if ( mat[i] < i_tree->icv[i] ) {
      if ( i_tree->left )
	return add_this(mat,i_tree->left);
      i_tree->left = tack_on(i_tree,mat);
      return true;
    }
    else if ( mat[i] > i_tree->icv[i] ) {
      if ( i_tree->right )
	return add_this(mat,i_tree->right);
      i_tree->right = tack_on(i_tree,mat);
      return true;
    }
  return false;
}





/*
*	Extend the isom tree with a new entry.  Its parent is
*	p and its matrix is mat.
*	If there is no free ism in the stack, crash gracefully.
*/

ism tack_on(ism p, char mat[])
{
  ism pi;
  int i;

  pi = nextisom;
  if ( !pi ) skipout("Isomorphism stack overflow",SKIP);
  if ( pi >= istak+isoused ) isoused++;
  nextisom = pi->parent;
  pi->left = pi->right = 0;
  pi->parent = p;
  lastisovect += tr_par.vlength;
  pi->icv = isolist+lastisovect;
  for ( i = 0; i < tr_par.vlength; i++ )
    pi->icv[i] = mat[i];
  return pi;
}
