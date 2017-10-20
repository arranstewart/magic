#include "RM.h"
#include "hmi.h"


/*
 * Is there a homomorphic image of m1 in m2?
 */

int homo(MATRIX *m1, MATRIX *m2)
{
  int h[SZ];
  int x,y;

  /*
   * First check that the fragments agree
   */
  for (x=NEG; x<FRAGMAX; x++)
    if (m1->fragment[x] != m2->fragment[x])
      return 0;
  FORALLCON(m1,x);
  FORALLCON(m2,y);
  if (x != y )
    return 0;

  /*
   * Now initialise to "undefined" and ask for a solution
   */
  for (x=0; x<SZ; x++)
    h[x] = -1;
  return homomorphism(m1,m2,h);
}


/*
 * Recursively generate a homomorphism if there is one
 */

int homomorphism(MATRIX *m1, MATRIX *m2, int h[])
{
  int x,y;
  int localh[SZ];

  copy_h(m1,h,localh);
  FORALL(m1,x)
    if (h[x] < 0) {
      FORALL(m2,y) {
	copy_h(m1,h,localh);
	localh[x] = y;
	if (propagated(m1,m2,localh) &&
	    homomorphism(m1,m2,localh)) {
	  copy_h(m1,localh,h);
	  return 1;
	}
      }
      return 0;
    }
  copy_h(m1,localh,h);
  return 1;
}


/*
 * Is there a non-trivial homomorphic image of m1 in m2?
 */

int nt_homo(MATRIX *m1, MATRIX *m2)
{
  int h[SZ];
  int x,y;

  /*
   * First check that the fragments agree
   */
  for (x=NEG; x<FRAGMAX; x++)
    if (m1->fragment[x] != m2->fragment[x])
      return 0;
  FORALLCON(m1,x);
  FORALLCON(m2,y);
  if (x != y )
    return 0;

  /*
   * Now initialise to "undefined" and ask for a solution
   */
  for (x=0; x<SZ; x++)
    h[x] = -1;
  return nt_homomorphism(m1,m2,h);
}


/*
 * Recursively generate a non-trivial homomorphism if there is one
 */

int nt_homomorphism(MATRIX *m1, MATRIX *m2, int h[])
{
  int x,y;
  int localh[SZ];

  copy_h(m1,h,localh);
  FORALL(m1,x)
    if (h[x] < 0) {
      FORALL(m2,y) {
	copy_h(m1,h,localh);
	localh[x] = y;
	if (propagated(m1,m2,localh) &&
	    nt_homomorphism(m1,m2,localh)) {
	  copy_h(m1,localh,h);
	  return 1;
	}
      }
      return 0;
    }
  FORALL(m1,x)
    FORALL(m1,y)
    if (x < y && localh[x] != localh[y]) {
      copy_h(m1,localh,h);
      return 1;
    }
  return 0;
}


/*
 * Is m1 [isomorphic to] a subalgebra of m2?
 */

int embedding(MATRIX *m1, MATRIX *m2)
{
  int h[SZ];
  int x,y;

  /*
   * First check that the fragments agree and the sizes are OK
   */
  for (x=NEG; x<FRAGMAX; x++)
    if (m1->fragment[x] != m2->fragment[x])
      return 0;
  FORALLCON(m1,x);
  FORALLCON(m2,y);
  if (x != y )
    return 0;
  if (m1->siz > m2->siz)
    return 0;

  /*
   * Now initialise to "undefined" and ask for a solution
   */
  for (x=0; x<SZ; x++)
    h[x] = -1;
  return injected_into(m1,m2,h);
}


/*
 * Recursively generate an injective homomorphism if there is one
 */

int injected_into(MATRIX *m1, MATRIX *m2, int h[])
{
  int x,y;
  int localh[SZ];

  copy_h(m1,h,localh);
  FORALL(m1,x)
    if (h[x] < 0) {
      FORALL(m2,y) {
	copy_h(m1,h,localh);
	localh[x] = y;
	if (propagated(m1,m2,localh) &&
	    injected_into(m1,m2,localh)) {
	  copy_h(m1,localh,h);
	  return 1;
	}
      }
      return 0;
    }
  FORALL(m1,x) FORALL(m1,y)
    if (x < y && localh[x] == localh[y])
      return 0;
  copy_h(m1,localh,h);
  return 1;
}


/*
 * Is there a (surjective) homomorphism from m1 onto m2?
 */

int epimorphic_image(MATRIX *m1, MATRIX *m2)
{
  int h[SZ];
  int x,y;

  /*
   * First check that the fragments agree and the sizes are OK
   */
  for (x=NEG; x<FRAGMAX; x++)
    if (m1->fragment[x] != m2->fragment[x])
      return 0;
  FORALLCON(m1,x);
  FORALLCON(m2,y);
  if (x != y )
    return 0;
  if (m1->siz < m2->siz)
    return 0;

  /*
   * Now initialise to "undefined" and ask for a solution
   */
  for (x=0; x<SZ; x++)
    h[x] = -1;
  return mapped_onto(m1,m2,h);
}


/*
 * Recursively generate a surjective homomorphism if there is one
 */

int mapped_onto(MATRIX *m1, MATRIX *m2, int h[])
{
  int x,y;
  int invh[SZ];
  int localh[SZ];

  copy_h(m1,h,localh);
  FORALL(m1,x)
    if (h[x] < 0) {
      FORALL(m2,y) {
	copy_h(m1,h,localh);
	localh[x] = y;
	if (propagated(m1,m2,localh) &&
	    mapped_onto(m1,m2,localh)) {
	  copy_h(m1,localh,h);
	  return 1;
	}
      }
      return 0;
    }
  FORALL(m2,y)
    invh[y] = -1;
  FORALL(m1,x)
    invh[localh[x]] = x;
  FORALL(m2,y)
    if (invh[y] < 0)
      return 0;

  copy_h(m1,localh,h);
  return 1;
}


void copy_h(MATRIX *m, int source[], int dest[])
{
  int x;

  FORALL(m,x)
    dest[x] = source[x];
}


int propagated(MATRIX *m1, MATRIX *m2, int h[])
{
  int localh[SZ];
  int x;

  for (x=0; x<SZ; x++)
    localh[x] = h[x];
  if (m1->fragment[NEG] &&
      !monprop(m1,m1->neg,m2->neg,localh))
    return 0;
  if (m1->fragment[BOX] &&
      !monprop(m1,m1->box,m2->box,localh))
    return 0;
  FORALLCON(m1,x)
    if (m1->adicity[x] == 1 &&
	!monprop(m1,m1->monadic[x],m2->monadic[x],localh))
      return 0;
  if (!dyprop(m1,m1->C,m2->C,localh))
    return 0;
  if (m1->fragment[FUS] &&
      !dyprop(m1,m1->fus,m2->fus,localh))
    return 0;
  if (m1->fragment[LAT]) {
    if (!dyprop(m1,m1->K,m2->K,localh))
      return 0;
    if (!dyprop(m1,m1->A,m2->A,localh))
      return 0;
  }
  FORALLCON(m1,x)
    if (m1->adicity[x] == 2 &&
	!dyprop(m1,m1->dyadic[x],m2->dyadic[x],localh))
      return 0;
  FORALL(m1,x)
    h[x] = localh[x];
  return 1;
}


int monprop(MATRIX *m1, int a[], int b[], int lh[])
{
  int x;

  FORALL(m1,x)
    if (lh[x] >= 0 && lh[a[x]] != b[lh[x]]) {
      if (lh[a[x]] < 0)
	lh[a[x]] = b[lh[x]];
      else
	return 0;
    }
  return 1;
}


int dyprop(MATRIX *m1, int a[][SZ], int b[][SZ], int lh[])
{
  int x,y;

  FORALL(m1,x)
    if (lh[x] >= 0)
      FORALL(m1,y)
	if (lh[y] >= 0 && lh[a[x][y]] != b[lh[x]][lh[y]]) {
	  if (lh[a[x][x]] < 0)
	    lh[a[x][y]] = b[lh[x]][lh[y]];
	  else return 0;
	}
  return 1;
}
