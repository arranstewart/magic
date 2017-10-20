/*			Input.c			March 1991
*
*	Functions to read matrices for MaGIC postprocessor.
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




void read_header(FILE *f, MATRIX *m)
{
  int i, j = 0;

  for (i=0; i<FRAGMAX; i++)
    fscanf(f,"%d",m->fragment+i);
  fscanf(f,"%d",&(m->cmax));
  FORALLCON(m,i) {
    fscanf(f,"%d",m->adicity+i);
    do
      fscanf(f,"%c",m->dcs[i]);
    while (isspace(m->dcs[i][0]));
    do
      fscanf(f,"%c",m->dcs[i]+(++j));
    while (!isspace(m->dcs[i][j]));
    m->dcs[i][j] = 0;
  }
}




int newsiz(FILE *f, MATRIX *m)
{
  return(gotsiz(f,m)? (m->fragment[NEG]? newneg(f,m): neword(f,m)): 0);
}

int newneg(FILE *f, MATRIX *m)
{
  return(gotneg(f,m)? neword(f,m): newsiz(f,m));
}

int neword(FILE *f, MATRIX *m)
{
  return(gotord(f,m)? newdes(f,m):
	 (m->fragment[NEG]? newneg(f,m): newsiz(f,m)));
}

int newdes(FILE *f, MATRIX *m)
{
  return(gotdes(f,m)? newC(f,m): neword(f,m));
}

int newC(FILE *f, MATRIX *m)
{
  return(gotaro(f,m)? (m->fragment[BOX]? newbox(f,m): newcon(f,m,0)):
	 newdes(f,m));
}

int newbox(FILE *f, MATRIX *m)
{
  return(gotbox(f,m)? newcon(f,m,0): newC(f,m));
}

int newcon(FILE *f, MATRIX *m, int x)
{
  if (x >= m->cmax) return 1;
  return(gotcon(f,m,x)? newcon(f,m,x+1): 
	 (x? newcon(f,m,x-1): 
	  (m->fragment[BOX]? newbox(f,m): newC(f,m))) );
}


int newcase(FILE *f, MATRIX *m)
{
  if (m->cmax)
    return newcon(f,m,m->cmax-1);
  if (m->fragment[BOX] )
    return newbox(f,m);
  return newC(f,m);
}




int gotsiz(FILE *f, MATRIX *m)
{
  fscanf(f,"%d",&(m->siz));
  m->negno = m->ordno = 0;
  m->oknegno = m->okordno = 0;
  return (m->siz < SZ && m->siz > 0);
}


int gotneg(FILE *f, MATRIX *m)
{
  int i;

  fscanf(f,"%d",m->neg);
  if (*(m->neg) < 0)
    return 0;
  m->negno++; m->ordno = 0;
  m->okordno = 0;
  for (i=1; i<=m->siz; i++ )
    fscanf(f,"%d", m->neg+i);
  return 1;
}


int gotord(FILE *f, MATRIX *m)
{
  int i,j;

  fscanf(f,"%d",*(m->ord));
  if (**(m->ord) < 0)
    return 0;
  m->ordno++; m->desno = 0;
  m->okdesno = 0;
  FORALL(m,i) FORALL(m,j) if (i||j)
    fscanf(f,"%d",m->ord[i]+j);
  FORALL(m,i) FORALL(m,j) {
    FORALL(m,m->A[i][j])
      if (m->ord[i][m->A[i][j]] && m->ord[j][m->A[i][j]])
	break;
    FOREACH(m,m->K[i][j])
      if (m->ord[m->K[i][j]][i] && m->ord[m->K[i][j]][j])
	break;
  }
  return 1;
}


int gotdes(FILE *f, MATRIX *m)
{
  int i;

  fscanf(f,"%d",m->designated);
  if (*(m->designated) < 0)
    return 0;
  FORALL(m,i)
    if (i)
      fscanf(f,"%d",m->designated+i);
  FORALL(m,m->tee)
    if ( m->designated[m->tee] )
      break;
  m->tee_exists = 1;
  FORALL(m,i)
    if (m->designated[i] && !m->ord[m->tee][i])
      m->tee_exists = 0;
  m->desno++; m->matno = 0;
  m->okmatno = 0;
  m->eff = m->neg[m->tee];
  return 1;
}


int gotaro(FILE *f, MATRIX *m)
{
  int i, j, k;

  fscanf(f,"%d",*(m->C));
  if (**(m->C) < 0)
    return 0;
  m->matno++;
  if (m->fragment[BOX]) {
    m->boxno = 0;
    m->okboxno = 0;
  }
  else {
    *(m->matplus) = 0;
    *(m->okmatplus) = 0;
  }
  FORALL(m,i) FORALL(m,j)
    if (i||j)
      fscanf(f,"%d",m->C[i]+j);
  m->fused = 1;
  FORALL(m,i) FORALL(m,j) {
    FORALL(m,m->fus[i][j])
      if (m->ord[i][m->C[j][m->fus[i][j]]])
	break;
    if (m->fus[i][j] > m->siz)
      m->fused = 0;
  }
  if (m->fused) {
    FORALL(m,i) FORALL(m,j) FORALL(m,k)
      if (m->ord[m->fus[i][j]][k] && !m->ord[i][m->C[j][k]])
	m->fused = 0;
    if (m->ord[i][m->C[j][k]] && !m->ord[m->fus[i][j]][k])
      m->fused = 0;
  }
  if (m->fragment[NEG])
    FORALL(m,i) FORALL(m,j)
      m->fis[i][j] = m->C[m->neg[i]][j];
  return 1;
}


int gotbox(FILE *f, MATRIX *m)
{
  int i;

  fscanf(f,"%d",m->box);
  if (*(m->box) < 0)
    return 0;
  m->boxno++;
  *(m->matplus) = 0;
  *(m->okmatplus) = 0;
  for (i=1; i<=m->siz; i++)
    fscanf(f,"%d",m->box+i);
  return 1;	
}


int gotcon(FILE *f, MATRIX *m, int x)
{
  int i, j, k;

  fscanf(f,"%d",&k);
  if (k < 0)
    return 0;
  m->matplus[x]++;
  m->matplus[x+1] = 0;
  m->okmatplus[x+1] = 0;
  switch(m->adicity[x]) {
  case 0:
    m->nulladic[x] = k;
    break;
  case 1:
    m->monadic[x][0] = k;
    FORALL(m,i) if (i)
      fscanf(f,"%d",m->monadic[x]+i);
    break;
  case 2:
    m->dyadic[x][0][0] = k;
    FORALL(m,i) FORALL(m,j)
      if ( i||j )
	fscanf(f,"%d",m->dyadic[x][i]+j);
  }
  return 1;
}
