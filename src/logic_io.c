/*
*			logic_io.c		V2.1 (May 1993)
*
*	This file contains the routines for reading in setups
*	and for making the appropriate initialisations.  It 
*	also contains the procedures for printing any matrices
*	found.
*/



	/****************************************************************
	*								*
	*			    MaGIC 2.1				*
	*								*
	*	    (C) 1992 Australian National University		*
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
*	First the semi-trivial function cluster to control the loop
*/

int newsiz()
{
  N[0] = 0;
  return( got_siz()? (theJob->f_n? newneg(): neword()): 0 );
}

int newneg()
{
  return( got_neg()? neword(): newsiz() );
}

int neword()
{
  return( got_ord()? newdes(): (theJob->f_n? newneg(): newsiz()) );
}

int newdes()
{
  return( got_des()? 1: neword() );
}




/*
*	Sep is a subroutine to print the separator "-1" if the
*	chosen format is UGLY.  The parameter is the counter for
*	the appropriate level.
*/

void sep(int *x)
{
  if ( *x ) {
    if ( theJob->tty_out==UGLY ) printf(" -1\n");
    if ( theJob->fil_out==UGLY ) fprintf(outfil, " -1\n");
    *x = 0;
  }
}



/*
*	Next_bit returns 1 or 0 depending on the next bit in the
*	input file. This is to allow very compressed data to be used.
*/

int next_bit()
{
  if ( input_bit == 8 ) {
    if ( read(infil,&buffa,1) < 1 ) return 0;
    input_bit = 0;
  }
  if ( buffa & ((char)1 << input_bit++) ) return 1;
  return 0;
}





/*
*	Return the new size, or 0 if there isn't one.
*	The values of T and F are incidentally set at this stage.
*/

int got_siz()
{ 
  if ( theJob->f_n )
    sep( &negno );
  else	sep( &ordno );
  if ( next_bit() ) siz++; else siz = 0;
  if ( tr_par.done || siz>=theJob->sizmax ) siz = 0;
  if ( theJob->f_T ) theJob->form[VMAX+2].val = siz;
  if ( theJob->f_F ) theJob->form[VMAX+3].val = 0;
  return siz;
}




/*
*	Set the next negation table.
*	Return 1 if successful, 0 if not.
*/

boolean got_neg()
{
  int i;

  sep(&ordno);
  if ( tr_par.done || !next_bit() ) return false;
  if ( N[0] ) {
    FORaLL(i)
      if ( N[i] > i ) break;
    N[N[i]] = N[i];
    N[i] = i;
  }
  else FORALL(i) N[i] = siz-i;
  return true;
}





/*
*	Read the next order table.
*	Return 1 if successful, 0 if not.
*	At this stage, decide which elements are maximal, define
*	lattice meet and join if they exist, and set up cc, the
*	index to the communication vector of transref.
*/

boolean got_ord()
{
  int i, j, k;

  sep( &desno ); 
  if ( tr_par.done || !next_bit() ) return false;

  FORALL(i)  FORALL(j)
    if ( i < j ) ord[i][j] = next_bit();
    else ord[i][j] = (i == j);

  FORALL(i) {
    maximal[i] = 1;
    for ( j = i+1; j <=siz; j++ ) maximal[i] *= !ord[i][j];
  }
  if ( theJob->f_lat ) {
    FORALL(i) {
      for ( j = i; j <= siz; j++ ) {
	for (k = j; !(ord[i][k] && ord[j][k]); k++) ;
	A[i][j] = A[j][i] = k;
	for (k = i; !(ord[k][i] && ord[k][j]); k--) ;
	K[i][j] = K[j][i] = k;
      }
    }
  }

  set_up_cc();
  return true;
}




/*
*	Read the next choice of designated values, or of least
*	designated value if t is defined.
*	Return 1 on success, 0 on failure.
*
*	The set of designated (true) values can be calculated
*	here, and the values of t and f filled in.
*/

boolean got_des()
{ 
  int i;

  if ( matsum ) {
    if ( theJob->tty_out == SUMMARY ) {
      printf("     %d matri%s", 
	     matsum, (matsum>1? "ces": "x"));
      fflush(stdout);
    }
    if ( theJob->fil_out == SUMMARY ) 
      fprintf(outfil, "     %d matri%s",
	      matsum, (matsum>1? "ces": "x"));
    matsum = 0;
  }
  sep( &matno );

  if ( tr_par.done || !next_bit() ) return false;
  FORALL(i) desig[i] = next_bit();

  for ( des = 0; !desig[des]; des++ ) ;
  for ( undes = siz; desig[undes]; undes-- ) ;
  if ( theJob->f_t ) {
    theJob->form[VMAX].val = des;
    if ( theJob->f_n ) theJob->form[VMAX+1].val = N[des];
  }
  return true;
}




/*
======================================================================
*/


/*
*  That's the end of the input routines.  Now for the output ones.
*
*  Mat_print is called from the tester with each good matrix.
*/
  
void mat_print()
{
  int i, j;
  boolean b;

  firstchange = -1;
  for ( i = 0; i < tr_par.vlength; i++ ) {
    if ( thisvector[i] != thatvector[i] ) firstchange = i;
    thatvector[i] = thisvector[i];
  }
  if ( !matno ) firstchange = tr_par.vlength;
  if ( firstchange < 0 ) skipout("Same matrix found twice",SKIP);

  j = -1;
  for ( i = 0; theJob->dcs[i]; i++ );
  for (--i; i >= 0; i--)
    if ( theJob->defcon[i] == PRIMITIVE ) {
      switch( theJob->adicity[i] ) {
      case 0:
	newmatplus( j, ucc0[i] );
	break;
      case 1:
	newmatplus( j, ucc1[i][0] );
	break;
      case 2:
	newmatplus( j, ucc2[i][0][0] );
      }
      j = i;
    }
  if ( theJob->f_nec ) {
    newmatplus( j, firstbox );
    if ( firstchange >= firstarrow ) sep(&boxno);
  }
  else newmatplus( j, firstarrow );

  b = true;
  for ( i = 0;  theJob->dcs[i]; i++ );
  for ( --i;  i>=0; i-- )
    if ( theJob->defcon[i] == PRIMITIVE )
      if ( matplus[i]++ ) {
	b = false;
	break;
      }
  if ( b && theJob->f_nec )
    if ( boxno++ ) b = false;
  if ( b )
    if ( !matno++ )
      if ( !desno++ )
	if ( !ordno++ )
	  if ( theJob->f_n )
	    negno++;

  good++;
  matsum++;
  if ( theJob->tty_out ) printup(stdout,theJob->tty_out);
  if ( theJob->fil_out ) printup(outfil,theJob->fil_out);
}




/*
*	Essentially a macro for the above:
*/

void newmatplus(int x, int y)
{
  if ( firstchange >= y && x >= 0 ) sep(matplus+x);
}





/*
*	Printing the new size is part of printing the new negation
*	table or, if negation is not defined, the new order table.
*	The parameter x is the output mode, and f the destination.
*/

void siz_print(FILE *f, output_style x)
{
  switch(x) {
  case UGLY:
    fprintf(f, " %d\n", siz);
    break;
  case PRETTY:
    fprintf(f, "\n\n\n\n Size: %d\n", siz+1);
    break;
  case SUMMARY:
    fprintf(f, "\n\n\n Size: %d", siz+1);
  case NONE:
    break;
  }
}




/*
*	Printing the new negation table is part of printing the new 
*	order table.
*	The parameter x is the output mode, and f the destination.
*/

void neg_print(FILE *f, output_style x)
{
  int i;

  if ( negno == 1 ) siz_print(f,x);
  switch(x) {
  case UGLY:
    FORALL(i) fprintf(f, " %d", N[i]);
    fprintf(f, "\n");
    break;
  case PRETTY:
    fprintf(f, "\n\n Negation table ");
    pretty_negno(f);
    fprintf(f, "\n\n         a |");
    FORALL(i) fprintf(f, " %x", i);
    fprintf(f, "\n        ---+");
    FORALL(i) fprintf(f, "--");  
    fprintf(f, "\n        ~a |");
    FORALL(i) fprintf(f, " %x", N[i]);
    fprintf(f, "\n");
    break;
  case SUMMARY:
    fprintf(f, "\n\n    Negation ");
    pretty_negno(f);
  case NONE:
    break;
  }
}




/*
*	Printing the new order table is part of printing the new 
*	choice of designated values.
*	The parameter x is the output mode, and f the destination.
*/

void ord_print(FILE *f, output_style x)
{
  int i, j;

  if ( ordno == 1 ) {
    if ( theJob->f_n ) neg_print(f,x);
    else siz_print(f,x);
  }
  switch(x) {
  case NONE:
    break;
  case UGLY:
    FORALL(i) FORALL(j) 
      fprintf(f, " %d", ord[i][j]); 
    fprintf(f, "\n");
    break;
  case PRETTY:
    fprintf(f, "\n\n Order ");
    pretty_ordno(f);
    fprintf(f, "\n\n         < |");
    FORALL(i) fprintf(f, " %x", i);
    fprintf(f, "\n         --+");
    FORALL(i) fprintf(f, "--");
    FORALL(i)
      {	fprintf(f, "\n%10x |", i);
      FORALL(j) 
	fprintf(f, " %c", (ord[i][j]? '+': '-'));
      }
    fprintf(f, "\n");
    break;
  case SUMMARY:
    fprintf(f, "\n\n       Order ");
    pretty_ordno(f);
  }
}



/*
*	Printing the new choice of designated values is part of 
*	printing the new implication matrix.
*	The parameter x is the output mode, and f the destination.
*
*	If t is defined only it is printed.  Otherwise the true
*	values are listed.  In ugly format, the boolean vector 
*	true (1 if value is designated, 0 if not) is printed.
*	Note that this convention for ugly output is different 
*	from that of MaGIC version 1.1 and causes incompatibility
*	with post-processing software.
*/

void des_print(FILE *f, output_style x)
{ 
  int i;

  if ( desno == 1 ) ord_print(f,x);
  switch(x) {
  case NONE:
    break;
  case UGLY:
    FORALL(i) fprintf(f, " %d", desig[i]); 
    fprintf(f, "\n" );
    break;
  case PRETTY:
    if ( theJob->f_t ) {
      fprintf(f, "\n\n Choice ");
      pretty_desno(f);
      fprintf(f, " of t:  %x\n", des);
    }
    else {
      fprintf(f, "\n\n Choice ");
      pretty_desno(f);
      fprintf(f, " of truths:  ");
      FORALL(des) if ( desig[des] )
	fprintf(f, " %x", des);
      fprintf(f, "\n");
    }
    break;
  case SUMMARY:
    if ( theJob->f_t )
      fprintf(f, "\n\n          t = %x", des);
    else {
      fprintf(f, "\n\n          truth =");
      FORALL(des) if ( desig[des] )
	fprintf(f, " %x", des);
    }
  }
}




/*
*	Printing the new implication matrix is part of printing
*	the new algebra.
*	The parameter x is the output mode, and f the destination.
*/

void C_print(FILE *f, output_style x)
{
  int i, j;

  if ( matno == 1 ) des_print(f,x);
  switch(x) {
  case SUMMARY:
  case NONE:
    return;
  case UGLY:
    FORALL(i) FORALL(j) fprintf(f, " %d", C[i][j]);
    break;
  case PRETTY:
    fprintf(f, "\n\n Implication matrix ");
    pretty_matno(f);

    fprintf(f, "\n\n        -> |");
    FORALL(i) fprintf(f, " %x", i);
    fprintf(f, "\n        ---+");
    FORALL(i) fprintf(f, "--");
    FORALL(i) {
      fprintf(f, "\n%10x |", i);
      FORALL(j) fprintf(f, " %x", C[i][j]);
    }
    fprintf(f, "\n");
  }
}




/*
*	Printing a new ! table may be part of printup.
*	The parameter x is the output mode, and f the destination.
*/

void box_print(FILE *f, output_style x)
{
  int i;

  switch(x) {
  case UGLY:
    FORALL(i) fprintf(f, " %d", box[i]);
    fprintf(f, "\n");
    break;
  case PRETTY:
    fprintf(f, "\n ! matrix ");
    pretty_boxno( f );
    fprintf(f, "\n\n         a |");
    FORALL(i) fprintf(f, " %x", i);
    fprintf(f, "\n        ---+");
    FORALL(i) fprintf(f, "--");  
    fprintf(f, "\n        !a |");
    FORALL(i) fprintf(f, " %x", box[i]);
    if ( theJob->f_n ) {
      fprintf(f, "\n        ?a |");
      FORALL(i) fprintf(f, " %x", diamond[i]);
    }
    fprintf(f, "\n");
    break;
  case SUMMARY:
  case NONE:
    return;
  }
}




/*
*	Printing a nulladic user connective may be part of printup.
*	The parameter x is the output mode, and f the destination.
*/

void u_print0(FILE *f, output_style x, int y)
{ 
  switch(x) {
  case UGLY:
    fprintf(f, " %d\n", nulladic[y]);
    break;
  case PRETTY:
    fprintf(f, "\n Choice ");
    pretty_umat( f, y );
    fprintf(f, " of %s:  %x\n", 
	    theJob->dcs[y]->s, nulladic[y]);
    break;
  case SUMMARY:
  case NONE:
    return;
  }
}





/*
*	Printing a monadic user connective may be part of printup.
*	The parameter x is the output mode, and f the destination.
*/

void u_print1(FILE *f, output_style x, int y)
{
  int i;

  switch(x) {
  case UGLY:
    FORALL(i) fprintf(f, " %d", monadic[y][i]);
    fprintf(f, "\n");
    break;
  case PRETTY:
    fprintf(f, "\n %s matrix ", theJob->dcs[y]->s);
    pretty_umat( f, y );
    fprintf(f, "\n\n          ");
    for ( i = 0; i < strlen(theJob->dcs[y]->s); i++ )
      printf(" ");
    fprintf(f, "a |");
    FORALL(i) fprintf(f, " %x", i);
    fprintf(f, "\n        ----");
    for ( i = 0; i < strlen(theJob->dcs[y]->s); i++ )
      printf("-");
    fprintf(f, "+");
    FORALL(i) fprintf(f, "--");  
    fprintf(f, "\n        %s(a) |", theJob->dcs[y]->s);
    FORALL(i) fprintf(f, " %x", monadic[y][i]);
    fprintf(f, "\n");
    break;
  case SUMMARY:
  case NONE:
    return;
  }
}

			

/*
*	Printing a dyadic user connective may be part of printup.
*	The parameter x is the output mode, and f the destination.
*/

void u_print2(FILE *f, output_style x, int y)
{
  int i, j, k;

  switch(x) {
  case NONE:
  case SUMMARY:
    return;
  case UGLY:
    FORALL(i) FORALL(j)
      fprintf(f, " %d", dyadic[y][i][j]);
    break;
  case PRETTY:
    fprintf(f, "\n %s matrix ", theJob->dcs[y]->s);
    pretty_umat( f, y );
    fprintf(f, "\n\n                   %s |", 
	    theJob->dcs[y]->s);
    FORALL(i) fprintf(f, " %x", i);
    fprintf(f, "\n                  --");
    for ( i = 0; i < strlen(theJob->dcs[y]->s); i++ )
      printf("-");
    fprintf(f, "+");
    FORALL(i) fprintf(f, "--");
    FORALL(i) {
      fprintf(f, "\n");
      for ( k = 0; k < strlen(theJob->dcs[y]->s); k++ )
	printf(" ");
      fprintf(f, "%19x |", i);
      FORALL(j)
	fprintf(f, " %x", dyadic[y][i][j]);
    }
    fprintf(f, "\n");
  }
}





/*
*	This is the printup routine called with every good matrix.
*	The parameter x is the output mode, and f the destination.
*/

void printup(FILE *f, output_style x)
{
  int i;

  if ( firstchange >= firstarrow ) C_print(f,x);
  if ( theJob->f_nec && firstchange >= firstbox ) box_print(f,x);
  for ( i = 0; theJob->dcs[i]; i++ )
    if ( theJob->defcon[i] == PRIMITIVE )
      switch( theJob->adicity[i] ) {
      case 0:
	if ( firstchange >= ucc0[i] )
	u_print0(f,x,i);
      break;
      case 1:
	if (firstchange >= ucc1[i][0] )
	u_print1(f,x,i);
	break;
      case 2:
	if (firstchange >= ucc2[i][0][0] )
	  u_print2(f,x,i);
      }
  if ( x == PRETTY ) {
    if ( theJob->failure ) fail_print(f);
    fprintf(f, "\n");
    fflush(f);
  }
}





/*
*	This is the series of functions for pretty-printing the
*	matrix numbers.
*/

void pretty_size(FILE *f)
{
  fprintf( f, "%d", siz+1 );
}


void pretty_negno(FILE *f)
{
  pretty_size(f);
  fprintf( f, ".%d", negno );
}


void pretty_ordno(FILE *f)
{
  if ( theJob->f_n )
    pretty_negno(f);
  else	pretty_size(f);
  fprintf( f, ".%d", ordno );
}


void pretty_desno(FILE *f)
{
  pretty_ordno(f);
  fprintf( f, ".%d", desno );
}



void pretty_matno(FILE *f)
{
  pretty_desno(f);
  fprintf( f, ".%d", matno );
}



void pretty_boxno(FILE *f)
{
  pretty_matno(f);
  if ( theJob->f_nec ) fprintf( f, ".%d", boxno );
}


/*
 * DOES NOT LOOK RIGHT AT ALL
 *
void pretty_umat(FILE *f, int x)
{
  int i;

  for ( i = x-1; i >= 0; i-- )
    if ( theJob->defcon[i] == PRIMITIVE ) {
      pretty_umat( f, i );
      break;
    }
  if ( i < 0 ) pretty_matno(f);
  fprintf( f, ".%d", matplus[x] );
}
 *
*/

void pretty_umat(FILE *f, int x)
{
  if ( x < 0 ) pretty_boxno(f);
  else {
    pretty_umat(f,x-1);
    if ( theJob->defcon[x] == PRIMITIVE )
      fprintf( f, ".%d", matplus[x] );
  }
}



/*
*	The assignment of values failing the bad guy is appended to 
*	each structure recorded in pretty format.
*/

void fail_print(FILE *f)
{
  insert_badvalues( theJob->failure );
  fprintf(f,"\n Failure: ");
  outformula( theJob->failure, theJob->failure, f, VALS );
  fprintf(f,"\n");
}


void insert_badvalues( int offset )
{
  if ( !offset ) return;
  insert_badvalues( theJob->form[offset].lsub );
  insert_badvalues( theJob->form[offset].rsub );
  if ( offset <= Vmax ) theJob->form[offset].val = badvalue[offset];
}





/*
*	At the end of a job some runtime statistics are sent to
*	stdout.  Note that this is not done in batch mode.
*/

void stats_print()
{
  int tim;

  CLoCK(&tim);
  printf("\n\n\n\n\n Matrices generated by MaGIC:  ");
  fflush(stdout);
  system("date");
  printf("\n\n Good ones found:   %d\n", good);
  printf(" Bad ones tested:   %d\n", tot-(good+isoms+isoms2));
  printf(" Isomorphs omitted: %d", isoms);
  if ( isoms2 ) printf(" + %d", isoms2);
#ifdef HASTIMES
  end_timer = time_buffer.tms_utime;
  printf("\n Time (cpu):        %1.2f seconds",
	 (end_timer - begin_timer)/(1.0*CLK_TCK));
#endif
  printf("\n Time (wall clock): %1.2f seconds\n\n\n", 
	 (tim - start_time)/(TICK*1.0));
}
