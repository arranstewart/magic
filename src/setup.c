/*
*			setup.c			V2.1 (May 1993)
*
*	These functions have nothing in common except that they
*	are called near the outer levels of MaGIC and don't really
*	fit into any other category.
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



#include <sys/types.h>      // needed for gettimeofday data types
#include <sys/time.h>       // because we use gettimeofday for timing
#include <sys/stat.h>       // nobody knows why this is included
#include <fcntl.h>          // because we use "open" to read data file

#include "MaGIC.h"




/*
*	Subf_set completes the contents of theJob->form.
*	It also sets up vu[] and kost[].
*/

void subf_set()
{
  int i, j;

  for ( i = 0; theJob->form[i].sym; i++ ) {
    theJob->form[i].val = 0;
    theJob->form[i].mtx = 0;
    theJob->form[i].lv =
      &(theJob->form[theJob->form[i].lsub].val);
    theJob->form[i].rv =
      &(theJob->form[theJob->form[i].rsub].val);
    for ( j = 0; theJob->dcs[j]; j++ )
      if ( theJob->dcs[j] == theJob->form[i].sym ) {
	switch( theJob->adicity[j] ) {
	case 0:
	  theJob->form[i].mtx = nulladic+j;
	  break;
	case 1:
	  theJob->form[i].mtx = monadic[j];
	  break;
	case 2:
	  theJob->form[i].mtx = *(dyadic[j]); 
	}
	break;
      }
    if ( !theJob->form[i].mtx && 
	 !strcmp(theJob->form[i].sym->s,"->") )
      theJob->form[i].mtx = *C;
    else if ( !theJob->form[i].sym->s[1] && !theJob->form[i].mtx )
      switch( theJob->form[i].sym->s[0] ) {
      case '~':
	theJob->form[i].mtx = N;
	break;
      case '!':
	theJob->form[i].mtx = box;
	break;
      case '?':
	theJob->form[i].mtx = diamond;
	break;
      case 'v':
	theJob->form[i].mtx = *A;
	break;
      case '&':
	theJob->form[i].mtx = *K;
	break;
      case 'o':
	theJob->form[i].mtx = *fus;
      }
  }
  tx = theJob->form+i;

  for ( i = 0; i < VMAX; i++ ) vu[i] = rvu[i] = 0;
  Vmax = 0;
  if ( theJob->failure ) { 
    set_u(vu,theJob->failure);
    set_u(rvu,theJob->failure);
  }
  for ( i = 0; theJob->croot[i][0]; i++ ) {
    kost[i] = 0;
    for ( j = 0; theJob->croot[i][j]; j++ ) {
      set_u(vu,theJob->croot[i][j]);
      kost[i] += worstcase(theJob->croot[i][j],0);
    }
    for ( j = 0; theJob->proot[i][j]; j++ ) {
      set_u(vu,theJob->proot[i][j]);
      kost[i] += worstcase(theJob->proot[i][j],0);
    }
  }
}





/*
*	Worstcase calculates the maximum number of lookups of
*	changeable matrices in the assignment of a value to the
*	formula rooted at x.  This is 0 if that formula is a variable,
*	a pre-defined constant or null.  Otherwise it is the worstcases
*	of its left and right subformulas plus the cost of its main 
*	connective.  This cost is 0 for ~, & and v, 1 for -> and any
*	user-given primitives, the worstcase of the definition for any
*	user-defined connective and a number depending on the logic and
*	the matrix size in the case of o.
*/

int worstcase(int x, int ntop)
{
  int i, wcsum;
  WFF *wf;

  if ( x < 1 ) return 0;

  wf = theJob->form+x;

  if ( is_var(wf->sym) ) return 0;

  wcsum = worstcase(wf->lsub,1) + worstcase(wf->rsub,1);

  if ( !strcmp(wf->sym->s,"->") ) return (wcsum + ntop);
  if ( !wf->sym->s[1] ) switch( wf->sym->s[0] ) {
  case '&':
  case 'v':
  case '~':
  case 't':
  case 'f':
  case 'T':
  case 'F':	return wcsum;
  case '!':	return (wcsum + 1);
  case '?':	return (wcsum + 1);
  case 'o':
    if ( F_N && theJob->axiom[AxC] )
      return (wcsum + 1);
    return (wcsum + siz + 1);
  }

  for ( i = 0; theJob->dcs[i]; i++ )
    if ( theJob->dcs[i] == wf->sym ) {
      if ( theJob->defcon[i] == PRIMITIVE )
	return (wcsum + 1);
      return (wcsum + worstcase(theJob->defcon[i],1));
    }
  /*
    This case should never happen!
  */
  skipout("Unrecognised symbol in a formula",SKIP);
  return 0;
}




/*
*	Record the variables used in the formula rooted at x by
*	setting the array arr.
*/

void set_u(int arr[], int x)
{
  if ( x < 1 ) return;
  if ( x < VMAX ) {
    *(arr+x) = 1;
    if ( x > Vmax ) Vmax = x;
  }
  else {
    set_u(arr,theJob->form[x].lsub);
    set_u(arr,theJob->form[x].rsub);
  }
}




/*
*	Here is a clock function which ought to work even in
*	the worst of times.
*/

void CLoCK(int *timer)
{
  struct timeval tp;
  struct timezone tzp;

#ifdef HASTIMES
  times(&time_buffer);
#endif
  gettimeofday(&tp,&tzp);
  *timer = tp.tv_sec & 0xffff;
  *timer *= TICK;
  *timer += (tp.tv_usec / (1000000/TICK));
}



/*
*	This is called from got_ord in logic_io.c to set up the 
*	index from the structures representing the models to the
*	vector to be used by transref. At the same time, it labels 
*	`forbidden' any cells at which backtracking is not allowed.
*/

void set_up_cc()
{
  int i, j, k;
  boolean fb;

  tr_par.vlength = 0;
  for ( i = 0; theJob->dcs[i]; i++ ) ;
  while ( i ) {
    if ( theJob->defcon[--i] == PRIMITIVE ) {
      fb = theJob->concut[i];
      switch ( theJob->adicity[i] ) {
      case 0:
	ucc0[i] = tr_par.vlength++;
	tr_par.forbidden[ucc0[i]] = fb;				
	break;
      case 1:
	FORALL(j) {
	ucc1[i][j] = tr_par.vlength++;
	tr_par.forbidden[ucc1[i][j]] = fb;
	}
	break;
      case 2:
	FORALL(j) FORALL(k) {
	  ucc2[i][j][k] = tr_par.vlength++;
	  tr_par.forbidden[ucc2[i][j][k]] = fb;
	}
      }
    }
  }
  if ( theJob->f_nec ) {
    firstbox = tr_par.vlength;
    FORALL(i) {
      boxindex[i] = tr_par.vlength++;
      tr_par.forbidden[boxindex[i]] = false;
    }
  }
  firstarrow = tr_par.vlength;
  FORALL(i) FORALL(j)
    if ( !( F_N && i < N[j] )) {
      impindex[i][j] = tr_par.vlength++;
      tr_par.forbidden[impindex[i][j]] = false;
      if ( F_N ) impindex[N[j]][N[i]] = impindex[i][j];
    }
  if ( tr_par.vlength > V_LENGTH ) {
    printf(" %d > %d\n", tr_par.vlength, V_LENGTH);
    skipout( "V_LENGTH is too small for this job", SKIP );
  }
}



/*
*	This is called immediately before transref.
*/
 
void job_start()
{
  char infil_name[100];

  set_orders(infil_name);
  if ( (infil = open(infil_name,0)) < 0 )
    skipout("Failed to open the data file",SKIP);
  input_bit = 8;
  if ( *(theJob->outfil_name) && !filing ) {
    outfil = fopen(theJob->outfil_name,"w");
    filing = 1;
  }
  if ( theJob->fil_out == PRETTY || theJob->fil_out == SUMMARY )
    disp(outfil);
  else if ( theJob->fil_out == UGLY )
    uglydisp(outfil);
  if ( theJob->tty_out == UGLY )
    uglydisp(stdout);
  
  logic_axioms(1);

  CLoCK(&start_time);
#ifdef HASTIMES
  begin_timer = time_buffer.tms_utime;
#endif

  good = tot = isoms = isoms2 = 0;
  siz = 0;
  set_up_trin();
}




/*
*	This, on the other hand, is not.
*/

void job_stop(boolean batch)
{
  int i;

  CLoCK( &stop_time );
#ifdef HASTIMES
  end_timer = time_buffer.tms_utime;
#endif

  if ( theJob->tty_out == UGLY ) {
    printf(" -1\n");
    for ( i = 0; theJob->dcs[i]; i++ )
      if ( theJob->defcon[i] == PRIMITIVE ) printf(" -1\n");
  }
  if (theJob->fil_out == UGLY ) {
    fprintf(outfil, " -1\n");
    for ( i = 0; theJob->dcs[i]; i++ )
      if ( theJob->defcon[i] == PRIMITIVE )
	fprintf(outfil, " -1\n");
  }
  if ( !batch ) {
    stats_print();
    paws();
  }
  if ( filing ) {
    fclose(outfil);
    filing = 0;
  }

  if ( infil ) {
    close(infil);
    infil = 0;
  }
  tr_par.done = true;
  logic_axioms(false);
}





/*
*	Set_orders works out the input file name and extends the
*	string passed to it with that name.
*/

void set_orders(char s[])
{
  strcpy( s, theJob->data_dir );

  if ( theJob->totord )
    strcat(s,(theJob->f_n? "tn": "to"));
  else if ( theJob->f_lat ) {
    if ( !theJob->distrib ) 
      strcat(s,(theJob->f_n? "ln": "l"));
    else if ( theJob->axiom[AxBA] || 
	      ( theJob->logic==S4 && theJob->f_n ))
      strcat(s,"ba");
    else	strcat(s,(theJob->f_n? "dln": "dl"));
  }
  else {
    if ( theJob->f_n && theJob->f_t )
      strcat(s,"pont");
    else if ( theJob->f_n )
      strcat(s,"pon");
    else if ( theJob->f_t )
      strcat(s,"pot");
    else	strcat(s,"po");
  }
  
  sprintf( s+strlen(s), ".%d", Sizmax );
}




/*
*	This routine just prints the header for ugly output.
*/

void uglydisp(FILE *f)
{
  int i, j;

  fprintf( f, " %d", theJob->f_n );
  fprintf( f, " %d", theJob->f_t );
  fprintf( f, " %d", theJob->f_T );
  fprintf( f, " %d", theJob->f_F );
  fprintf( f, " %d", theJob->f_fus );
  fprintf( f, " %d", theJob->f_lat );
  fprintf( f, " %d", theJob->f_nec );
  j = 0;
  for ( i = 0; theJob->dcs[i]; i++ )
    if ( theJob->defcon[i] == PRIMITIVE ) j++;
  fprintf( f, " %d", j );
  for ( i = 0; theJob->dcs[i]; i++ )
    if ( theJob->defcon[i] == PRIMITIVE )
      fprintf( f, " %d %s", theJob->adicity[i], theJob->dcs[i]->s );
  fprintf( f, "\n" );
}


/*
*	The job parameters are communicated to transref via a structure
*	tr_par which must be set up before each call. Note that 
*	tr_par.forbidden which records functions on which there is a 
*	"cut" is set elsewhere.
*/

void set_up_trin()
{
  tr_par.Test = Good_matrix;
  tr_par.Onerefs = set_poss;
  tr_par.Smallrefs = find_twos;
  tr_par.batches = 10;
  tr_par.extra_batches = EXTRA_DEFAULT;
  tr_par.maxref = 6;
  tr_par.maxbak = 0;
  tr_par.verb = 0;
  tr_par.Nosecs = false;
  tr_par.topsoff = true;
  tr_par.report_breaks = false;
  tr_par.done = false;
}


/*
*	These stubs are just to maintain compatibility with the vntr used
*	by FINDER. They are not called.
*/

void report_branches(trin T) {}
void ref_size_report() {}

