/*
*			getjob.c		V2.1 (May 1993)
*
*	This file contains most of the functions called from dialog.c
*	which fill in the various fields of theJob for MaGICal 
*	purposes.  The functions called immediately following menu
*	selections come first, then the subsidiary ones.  Note that 
*	most of the formula-handling functions are in separate files.
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
*	Response to menu option (a).  This causes the axioms to be
*	listed on the screen with numbers and asks for a selection.
*	Note that "user-defined axiom" gets a number bigger than 
*	any of the listed axioms.
*
*	Listed axioms are marked as selected just by setting the
*	appropriate flags.  User defined ones have to be read in by
*	the function got_formula.
*/

void add_axioms()
{
  int i, j;
  int axint;
  AXIOM ax;
  AXIOM local_ax[AXMAX+2];

  j = 1;
  if ( !xdialog ) {
    for ( ax = AxNull+1; ax < AXMAX; ax++ )
      if ( !valid[theJob->logic][ax] && !theJob->axiom[ax] ) {
	local_ax[j] = ax;
	printf("\n %-5d%s", j++, ax_string[ax]);
      }
    printf("\n\n %-5d< User-defined axiom >\n", j);
    printf(" %-5d< User-defined rule >", j+1);
    local_ax[j] = AXMAX;
    local_ax[j+1] = AXMAX+1;
    printf("\n\n Select from the above by typing the numbers:  ");
    fflush(stdout);
  }
  else for ( ax = AxNull; ax < AXMAX+2; ax++ ) local_ax[ax] = ax;
  fgets(answer,SLEN,stdin);
  for ( i = 0; answer[i]; i++ )
    if ( !isdigit(answer[i]) ) answer[i] = ' ';

  for ( i = 0; answer[i]; i++ )
    if ( !i || (answer[i-1] == ' ' && answer[i] != ' ') ) {
      sscanf( answer+i, "%d", &axint );
      ax = (AXIOM)axint;
      if ( ax > AxNull && ax < AXMAX+2 )
	add_one_axiom( local_ax[ax] );
    }
}



void add_one_axiom( AXIOM select )
{
  int i, j;

  if ( select < AXMAX ) theJob->axiom[select] = true;
  else {
    for ( i = 0; theJob->croot[i][0] && i<TMAX; i++ ) ;
    if  (i == TMAX ) {
      EP;
      printf("\n\n Maximum number of user-defined axioms/rules is %d", TMAX);
      paws();
      return;
    }
    if ( select == AXMAX ) {
      got_formula( 0, i, 0, "axiom" );
      theJob->proot[i][0] = TRIVIAL;
    }
    else {
      if ( !xdialog ) {
	printf("\n Type the premise(s) of the ");
	printf("rule, one to a line.\n When there");
	printf(" are no more to come, just hit ");
	printf("RETURN.\n");
      }
      for ( j = 0; j < RTMAX; j++ ) {
	if ( !got_formula( 1, i, j, "premise" ))
	  break;
      }
      if ( !xdialog ) {
	printf("\n\n Now the conclusion(s) of the");
	printf(" rule, one to a line.\n When there");
	printf(" are no more to come, just hit ");
	printf("RETURN.\n");
      }
      for ( j = 0; j < RTMAX; j++ ) {
	if ( !got_formula( 0, i, j, "conclusion" ))
	  break;
      }
      if ( !theJob->croot[i][0] )
	theJob->croot[i][0] = ABSURD;
      if ( !theJob->proot[i][0] )
	theJob->proot[i][0] = TRIVIAL;
      if ( theJob->proot[i][0] == TRIVIAL
	   && theJob->croot[i][0] == ABSURD )
	theJob->proot[i][0] = theJob->croot[i][0] = 0;
    }
  }
}





/*
*	Response to menu option (b).
*	Take the user definition of a formula to go in failure.
*/

void bad_guy()
{ 
  if ( !xdialog ) puts("");
  got_formula( 2, 0, 0, "formula" );
}





/*
*	Response to menu option (c).
*	Take the user's specification of a new operator, find its
*	adicity and parse its definition if any.  Undefined 
*	connectives are allowed: their notional definitions are
*	set to the constant PRIMITIVE.
*/

void connective()
{
  int offset;
  char newcon[SLEN];
  symb sy;

  for ( offset = 0; theJob->defcon[offset] && offset<CMAX; offset++ ) ;
  if ( offset == CMAX ) {
    EP; printf("\n\n Maximum number of definitions is %d", CMAX);
    paws();
    return;
  }

  if ( !xdialog ) {
    printf("\n\n Type the new connective:    ");
    fflush(stdout); fgets(newcon,SLEN,stdin);
    nospace(newcon);
  }
  for ( sy = theJob->symtab; sy; sy = sy->next )
    if ( !strcmp(newcon,sy->s) ) {
      EP; printf("\n No fair: \"%s\" already means something.",newcon);
      paws();
      return;
    }

  if ( !xdialog ) {
    printf("\n What is the adicity of %s?   ", newcon);
    fflush(stdout); scanf("%d", theJob->adicity+offset); READLN;
  }
  if ( theJob->adicity[offset]<0 || theJob->adicity[offset]>2 ) {
    printf("\n No fair: adicity must be 0, 1 or 2.");
    paws();
    return;
  }

  if ( !xdialog )
    printf("\n D)efined or u)ndefined?     ");
  if ( readin("du") == 'u' ) {
    theJob->defcon[offset] = PRIMITIVE;
    if ( !xdialog )
      printf("\n Place a \"cut\" on %s? (y/n)   ", newcon);
    theJob->concut[offset] = (readin("yn") == 'y');
  }
  else {
    if ( !xdialog ) {
      if ( !theJob->adicity[offset] )
	printf("\n Define %s   ", newcon);
      else if ( theJob->adicity[offset] == 1 )
	printf("\n Define %s(a)   ", newcon);
      else	printf("\n Define a %s b   ", newcon);
      fflush(stdout);
    }
    theJob->defcon[offset] = 0;
    if ( !got_formula( IN_DEFN, offset, 0, "definition" )) return;
  }

  sy = newsymbol( newcon,
		  theJob->symbol[theJob->adicity[offset]],
		  theJob->dcs );
  if ( !theJob->adicity[offset] )
    theJob->form[VMAX+4+offset].sym = sy;
}





/*
*	Response to menu option (d).
*	The user is offered a choice of deleting an axiom from the
*	list, a user-defined axiom, a connective or the badguy.
*	Appropriate action is taken to scrub the unwanted item 
*	from the job specification.
*/

void deletion()
{
  if ( !xdialog ) {
    printf("\n Delete (a) pre-defined axiom?");
    printf("\n        (b) bad guy?");
    printf("\n        (c) connective?");
    printf("\n        (d) user-defined axiom?    ");
    fflush(stdout);
  }
  switch( readin("abcd") ) {
  case 'a':	delete_saxiom();	break;
  case 'b':	theJob->failure = 0;	break;
  case 'c':	delete_connective();	break;
  case 'd':	delete_uaxiom();
  }
}





/*
*	Menu option (e) is for "exit", so there is no response to
*	it in this file.  Fragment is the response to option (f).
*
*	There is a certain amount of detail involved in settling
*	the fragment, since some combinations of connectives allow
*	others to be defined.  The calculation of these forced 
*	extensions to the fragment is done here on the fly in 
*	order to avoid unnecessary questions.
*
*	In terse mode, all such fiddling is bypassed.
*/

void fragment()
{
  boolean old_nec;

  old_nec = theJob->f_nec;
  if ( noclear ) {
    if ( !xdialog ) {
      printf("\n\n Select which connectives you want");
      printf("\n\n ~        (y/n) ");
    }
    theJob->f_n = ( readin("yn") == 'y' );
    if ( !xdialog ) printf(" & and v  (y/n) ");
    theJob->f_lat = ( readin("yn") == 'y' );
    if ( !xdialog ) printf(" t        (y/n) ");
    theJob->f_t = ( readin("yn") == 'y' );
    if ( !xdialog ) printf(" T        (y/n) ");
    theJob->f_T = ( readin("yn") == 'y' );
    if ( !xdialog ) printf(" F        (y/n) ");
    theJob->f_F = ( readin("yn") == 'y' );
    if ( !xdialog ) printf(" o        (y/n) ");
    theJob->f_fus = ( readin("yn") == 'y' );
    if ( !xdialog ) printf(" !        (y/n) ");
    theJob->f_nec = ( readin("yn") == 'y' );
  }
  else {
    theJob->f_n = theJob->f_lat = theJob->f_t = 0;
    theJob->f_T = theJob->f_F = theJob->f_fus = 0;
    theJob->f_nec = 0;
    set_frag( false );
    if ( !theJob->f_n ) {
      printf(
	     "\n\n Do you want negation defined? (y/n)    ");
      theJob->f_n = ( readin("yn") == 'y' );
    }
    if ( !theJob->f_lat ) {
      printf("\n Do you want & and v defined? (y/n)     ");
      theJob->f_lat = ( readin("yn") == 'y' );
      if ( theJob->f_lat ) theJob->f_t = 1;
      else {
	printf("\n Do you want constant ");
	printf("t defined? (y/n)  ");
	theJob->f_t = ( readin("yn") == 'y' );
      }
    }
    if ( theJob->f_lat )
      theJob->f_T = theJob->f_F = 1;
    if ( !theJob->f_T ) {
      printf("\n Do you want constant ");
      printf("T defined? (y/n)  ");
      theJob->f_T = ( readin("yn") == 'y' );
      if ( theJob->f_T && theJob->f_n ) theJob->f_F = 1;
    }
    if ( !theJob->f_F ) {
      printf("\n Do you want constant ");
      printf("F defined? (y/n)  ");
      theJob->f_F = ( readin("yn") == 'y' );
    }
    if ( theJob->f_n && valid[theJob->logic][AxC] )
      theJob->f_fus = 1;
    if ( !theJob->f_fus ) {
      printf("\n Do you want fusion ");
      printf("o defined? (y/n)    ");
      theJob->f_fus = ( readin("yn") == 'y' );
    }
    if ( !theJob->f_nec ) {
      printf("\n Do you want necessity ");
      printf("! defined? (y/n) ");
      theJob->f_nec = (readin("yn") == 'y');
    }
  }
  if ( theJob->f_nec && !old_nec ) {
    theJob->axiom[RulNec] =
      !valid[theJob->logic][RulNec];
    theJob->axiom[AxNID] =
      !valid[theJob->logic][AxNID];
    if ( theJob->f_lat && valid[theJob->logic][AxW] )
      theJob->axiom[AxNand] =
	!valid[theJob->logic][AxNand];
  }
}
              



/*
*	Menu options (g) for "generate" and (h) for "help" do not
*	cause any action in this file.
*
*	Response to menu option (i).
*/

void input_direct()
{
  char dd[SLEN];

  if ( !xdialog ) {
    printf(" \n Current input directory: %s\n",
	   theJob->data_dir);
    printf(" \n New input directory:     ");
    fflush(stdout);
  }
  fgets(dd,SLEN,stdin);
  nospace(dd);
  if ( !*(dd) ) return;

  if ( dd[strlen(dd)-1] != '/' ) strcat( dd, "/" );
  strcpy( theJob->data_dir, dd );
} 




/*
*	Response to menu option (j).
*	This passage of dialogue gets the conditions on which 
*	the search is to be terminated.  There is nothing to 
*	explain here.
*/

void jump_condition()
{
  char	oc;

  if ( xdialog ) {
    scanf("%d", &(theJob->maxmat));  
    scanf("%d", &(theJob->sizmax));
    scanf("%d", &(theJob->maxtime));
    return; 
  }
  printf("\n Shall I stop when:");
  printf(" (a) I'm exhausted?\n");
  printf("%20c(b) time's up?\n", ' ');
  printf("%20c(c) I've found enough matrices?\n", ' ');
  printf("%20c(d) the matrices get too big?\n", ' ');
  printf("%20c(e) a combination of the above?        ", ' ');
  oc = readin("abcde");
  theJob->sizmax = Sizmax;  
  theJob->maxtime = theJob->maxmat = 0;  
  theJob->sizmax_ismax = 1;
  switch( oc ) {
  case 'c':
    printf("\n How many is enough?  "); 
    fflush(stdout);
    scanf("%d", &(theJob->maxmat));  
    if ( theJob->maxmat < 0 )
      theJob->maxmat = 0;
    break;
  case 'd':
    printf("\n How big is big enough?  ");
    fflush(stdout);
    scanf("%d", &(theJob->sizmax)); 
    if ( theJob->sizmax > 1 )
      theJob->sizmax_ismax = 0;
    break;
  case 'e':
    printf("\n How many matrices are enough?  "); 
    fflush(stdout);
    scanf("%d", &(theJob->maxmat));  
    if ( theJob->maxmat < 0 )
      theJob->maxmat = 0;
    printf("\n How big can they get?          ");
    fflush(stdout);
    scanf("%d", &(theJob->sizmax));
    if ( theJob->sizmax > 1 )
      theJob->sizmax_ismax = 0;
  case 'b':
    printf("\n How many seconds have I got?   ");
    fflush(stdout);
    scanf("%d", &(theJob->maxtime)); 
    if ( theJob->maxtime <= 0 )
      theJob->maxtime = 0;
    else if ( theJob->maxtime < 5 )
      theJob->maxtime = 5;
  }
}




/*
*	Initially and in response to menu option (l) the user is 
*	offered the choice of logics pre-defined in MaGIC.h.
*
*	Any logic name may be prefixed with 'T' if total orders are 
*	required, or 'L' if non-distributive lattices will do.
*/

void logic_choice()
{
  LOGIC lptr;

  if ( !xdialog ) {
    printf("\n\n What is your favourite logic?  ");
    fflush(stdout);
  }
  fgets(answer,SLEN,stdin);
  trim();

  for ( lptr = Null_logic+1; lptr < LOGMAX; lptr++ )
    if ( !strcmp(logic_name[lptr],answer) ) {
      set_logic( lptr );
      return;
    }

  if ( *(answer)=='L' || *(answer)=='T' || *(answer)=='D' )
    for ( lptr = Null_logic+1; lptr < LOGMAX; lptr++ )
      if ( !strcmp(logic_name[lptr],answer+1) ) {
	set_logic( lptr );
	theJob->totord = (*(answer)=='T');
	theJob->distrib = ((*(answer)=='T') || (*(answer)=='D'));
	return;
      }

  if ( *answer )
    printf("\n\n There is no such logic as \"%s\".", answer);
  printf("\n\n Logics are");
  for ( lptr = 1; lptr < LOGMAX; lptr++ )
    printf("%s%s",(lptr==1? " ":(lptr+1==LOGMAX? " and ": ", ")),
	   logic_name[lptr]);
  printf("\n Prefix \"D\" for distribution, ");
  printf("\"L\" for no distribution, ");
  printf("\"T\" for total order.");
  printf("\n\n\n H)elp, R)eselect or Q)uit?    ");
  switch( readin("hrq") ) {
  case 'q': exit(0);
  case 'h':
    help(FDL); paws();
    help(BTW); paws();
    help(LOG);
  }
  logic_choice();
}




/*
*	Set the currently selected logic to lptr.
*	Set the default order type and fragment for that logic.
*/

void set_logic( LOGIC lptr )
{
  theJob->logic = lptr;
  switch( default_orders[lptr] ) {
  case lattices:
    theJob->distrib = false;
    theJob->totord = false;
    break;
  case distributive_lattices:
    theJob->distrib = true;
    theJob->totord = false;
    break;
  case total_orders:
    theJob->distrib = true;
    theJob->totord = true;
  }
  theJob->f_n = default_fragment[lptr][n_exists];
  theJob->f_lat = default_fragment[lptr][lat_exists];
  theJob->f_fus = default_fragment[lptr][fus_exists];
  theJob->f_nec = default_fragment[lptr][nec_exists];
  theJob->f_t = theJob->f_T = theJob->f_F = theJob->f_lat;
}




/*
*	Response to menu option (m).
*	Print the version number, release date and author's ID.
*/

void print_version()
{
  printf("\n\n\n This is MaGIC Version %s, released %s.\n\n",
	 VERSION, RELEASE_DATE);
  printf(" Author and target for bug reports:\n");
  printf("\t J. Slaney\n\t Australian National University\n");
  printf("\t John.Slaney@anu.edu.au\n\n\n\n");
  paws();
}





/*
*	The sequential MaGIC makes no response to menu option (n).
*	The parallel version, however, allows the user to change 
*	the number of processes by means of this option.
*	To be honest, this feature was included mainly to provide
*	a working toy for the xmagic display panel.
*
*	The value returned is the new number of processes unless
*	we are sequential, in which case the return value is -1.
*	Note that this function does not actually create or destroy
*	processes, but merely returns the desired number.
*
*	The old convention for changing the number of processes 
*	was to type '#' followed by the number.  This is still
*	supported, although the normal call will be via menu (n).
*/

int n_of_procs()
{
#ifdef PARALLEL
  if (!(npcp = strchr(answer,'#'))) {
    printf("\n\n How many processes do you want?   ");
    fgets(npcp,10,stdin);
    nospace(npcp);
  }
  while ( *npcp && ( *npcp < '0' || *npcp > '9' )) npcp++;
  if ( !*npcp ) return(gm->nprocs);
  i = 0;
  do {
    i = i*10 + *npcp - '0';
    npcp++;
  }
  while ( *npcp >= '0' && *npcp <= '9' );
  if ( i < 2  || i > PARALLEL ) {
    printf("\n Number must be in range 2 - %d\n\n", PARALLEL);
    paws();
    return(gm->nprocs);
  }
  return(i);
#endif
  return(-1);
}





/*
*	Response to menu option (o).
*	Re-order symbols[2] to determine scope conventions, or
*	re-order theJob->dcs to determine change order.
*/

void order_change()
{
  symb primitives[CMAX];
  symb dyadics[CMAX+8];
  int i, j;
  int neworder[CMAX+8];

  j = 0;
  for ( i = 0; theJob->dcs[i]; i++ )
    if ( theJob->defcon[i] == PRIMITIVE )
      primitives[j++] = theJob->dcs[i];
  primitives[j] = 0;
  if ( j < 2 ) {
    user_order( theJob->symbol[2],
		"order of dyadic connectives", neworder );
    for ( i = 0; theJob->symbol[2][i]; i++ )
      dyadics[i] = theJob->symbol[2][neworder[i]];
    dyadics[i] = 0;
    for ( i = 0; dyadics[i]; i++ )
      theJob->symbol[2][i] = dyadics[i];
    return;
  }
  if ( !xdialog ) {
    printf("\n\n You may alter (\"s\") the scope ordering of");
    printf(" the\n dyadic connectives, or (\"c\") the change");
    printf(" order of\n the user-defined connectives.");
    printf("\n\n Which do you want to do?  (s/c):  ");
    fflush(stdout);
  }
  switch( readin("cs") ) {
  case 's':
    user_order( theJob->symbol[2],
		"order of dyadic connectives", neworder );
    for ( i = 0; theJob->symbol[2][i]; i++ )
      dyadics[i] = theJob->symbol[2][neworder[i]];
    dyadics[i] = 0;
    for ( i = 0; dyadics[i]; i++ )
      theJob->symbol[2][i] = dyadics[i];
    break;
  case 'c': 
    user_order( primitives, "change order", neworder );
    j = 0;
    for ( i = 0; theJob->dcs[i]; i++ )
      if ( theJob->defcon[i] == PRIMITIVE )
	theJob->dcs[i] = primitives[neworder[i]];
    break;
  }
}




/*
*	Shuffle symbols according to instructions.
*	The second parameter describes the order in question.
*/

void user_order(symb oldsymbols[], char *string, int neworder[])
{
  int i, j, mxs;

  for ( mxs = 0; oldsymbols[mxs]; mxs++ ) ;

  if ( !xdialog ) {
    printf("\n\n Current %s:\n\n", string);
    for ( i = 0; i < mxs; i++ ) {
      printf(" %-3d", i+1);
      for ( j = 1; j < strlen(oldsymbols[i]->s); j++ )
	putchar(' ');
    }
    puts("");
    for ( i = 0; i < mxs; i++ )
      printf(" %s  ", oldsymbols[i]->s);
    printf("\n\n Type the new order (e.g. 2 1 3 etc)\n");
  }
  for ( i = 0; i < mxs; i++ )
    scanf("%d", neworder+i);
  if ( xdialog ) return;

  for ( i = 0; i < mxs; i++ ) {
    if ( neworder[i] < 1 || neworder[i] > mxs ) neworder[i] = 0;
    for ( j = 0; j < i; j++ )
      if ( neworder[j] == neworder[i] ) neworder[i] = 0;
  }
  for ( i = 0; i < mxs; i++ ) if ( !neworder[i] ) {
    for ( j = i; j < mxs-1; j++ ) neworder[j] = neworder[j+1];
    neworder[mxs-1] = 0;
  }
  for ( i = 1; i <= mxs; i++ ) {
    for ( j = 0; j < mxs; j++ ) if ( neworder[j] == i ) break;
    if ( j == mxs ) {
      for ( j = 0; neworder[j]; j++ ) ;
      neworder[j] = i;
    }
  }
  for ( i = 0; i < mxs; i++ ) neworder[i]--;
}




/*
*	Response to menu option (p).
*	The print settings for tty and file are set independently.
*	If file output is requested, the output file name is set.
*	All of this is perfectly straightforward.
*/

void print_options()
{
  char oc;

  if ( !xdialog ) printf("\n\n"); 
  do {
    if ( !xdialog )
      printf(
	     "\n Tty:   P)retty  U)gly  S)ummary  N)one  H)elp    ");
    oc = readin("pusnh");
    theJob->tty_out = oc=='u'? UGLY: (oc=='p'? PRETTY: 
				      (oc=='s'? SUMMARY: NONE));
    if (oc=='h') help(OUT);
  }
  while ( oc == 'h' );
  do {
    if ( !xdialog )
      printf(
	     "\n File:  P)retty  U)gly  S)ummary  N)one  H)elp    ");
    oc = readin("pusnh");
    theJob->fil_out = oc=='u'? UGLY: (oc=='p'? PRETTY: 
				      (oc=='s'? SUMMARY: NONE));
    if ( oc == 'h' ) help(OUT);
  }
  while ( oc == 'h' );

  if (theJob->fil_out) {
    if ( !xdialog && *(theJob->outfil_name) ) {
      printf("\n New output file? (y/n)   ");
      if ( readin("yn") == 'n' ) return;
    }
    if ( filing ) {
      fclose(outfil);
      filing = 0;
    }
    do {
      if ( !xdialog ) {
	printf(" \n Name output file:   ");
	fflush(stdout);
      }
      fgets(theJob->outfil_name,SLEN,stdin);
      nospace(theJob->outfil_name);
    }
    while ( !*(theJob->outfil_name) );
    if ( !xdialog && (outfil = fopen(theJob->outfil_name,"r")) ) {
      fclose(outfil);
      printf("\n File \"%s\" ",theJob->outfil_name);
      printf("already exists. Overwrite it? (y/n)   ");
      fflush(stdout);
      if ( (oc = readin("yn")) == 'n' ) {
	strcpy(theJob->outfil_name,"");
	theJob->fil_out = false;
      }
    }
  }
  else strcpy(theJob->outfil_name,"");
}




/*
*	Menu option (q) for "quit" does not require action here.
*
*	Response to menu option (r).
*	This function prompts the user for a filename, opens the
*	file and reads the specification of theJob, expected to
*	have been placed there previously by function store_job.
*/

boolean read_job(char *batchfile)
{
  int i;
  char fname[SLEN];
  FILE *f;

  if ( batchfile ) {
    i = 0;
    while ( (fname[i] = batchfile[i]) ) i++;
  }
  else {
    if ( !xdialog ) {
      printf("\n\n Name the file containing the job");
      printf(" you want:  ");
      fflush(stdout);
    }
    fgets(fname,SLEN,stdin);
    nospace(fname);
  }
  if ( !(f = fopen( fname, "r" ))) {
    EP
      fprintf(stderr,"\n Can't find file %s (sorry).\n\n", fname);
    if ( !batchfile ) paws();
    return false;
  }
  injob(f);
  if ( batchfile ) set_frag( true );
  return true;
}




/*
*	Response to menu option (s).
*	Get the name of a file in which to store the job, 
*	then do it.
*/

void store_job()
{
  FILE *f;
  char fname[SLEN];

  if ( !xdialog )
    printf("\n\n Name the file where you want the job stored:  ");
  fgets(fname,SLEN,stdin);
  nospace(fname);
  if ( !(f = fopen( fname, "w" ))) {
    EP printf("\n Can't open that file (sorry).\n\n");
    paws();
    return;
  }
  outjob(f);
  fclose(f);
}





/*
*	This grabs the contents of theJob in the format in which they 
*	are dumped by outjob.  It is called by MaGIC in response to
*	menu option "r" and by xmagic to get the job description after
*	each update.
*/

int nint(FILE *f)
{
  int ni;

  fscanf(f, "%d", &ni);
  return ni;
}


void injob(FILE *f)
{
  int i, j, k;
  int nint();

  for ( i = 0; i < AXMAX; i++ )
    theJob->axiom[i] = (nint(f)? true: false);

  for ( i = 0; i < CMAX; i++ )
    theJob->adicity[i] = nint(f);

  for ( i = 0; i < TMAX; i++ )
    for ( j = 0; j < RTMAX; j++ )
      theJob->croot[i][j] = nint(f);

  for ( i = 0; i < TMAX; i++ )
    for ( j = 0; j < RTMAX; j++ )
      theJob->proot[i][j] = nint(f);

  for ( i = 0; i < CMAX; i++ )
    theJob->defcon[i] = nint(f);

  for ( i = 0; i < CMAX; i++ )
    theJob->concut[i] = (nint(f)? true: false);

  theJob->failure = nint(f);
  theJob->logic = nint(f);

  theJob->f_n = (nint(f)? true: false);
  theJob->f_lat = (nint(f)? true: false);
  theJob->f_t = (nint(f)? true: false);
  theJob->f_T = (nint(f)? true: false);
  theJob->f_F = (nint(f)? true: false);
  theJob->f_fus = (nint(f)? true: false);
  theJob->f_nec = (nint(f)? true: false);

  theJob->maxtime = nint(f);
  theJob->maxmat = nint(f);
  theJob->sizmax = nint(f);

  theJob->sizmax_ismax = (nint(f)? true: false);
  theJob->totord= (nint(f)? true: false);
  theJob->distrib = (nint(f)? true: false);

  theJob->tty_out = nint(f);
  theJob->fil_out = nint(f);
	
  i = 0;
  do theJob->data_dir[i] = nint(f);
  while ( theJob->data_dir[i++] );

  i = 0;
  do theJob->outfil_name[i] = nint(f);
  while ( theJob->outfil_name[i++] );

  i = 0;
  do logic_name[theJob->logic][i] = nint(f);
  while ( logic_name[theJob->logic][i++] );

  for ( i = 0; i < SYMBOLMAX; i++ ) {
    for ( j = 0; j < 16; j++ )
      theJob->symtable[i].s[j] = nint(f);
    if ( (k = nint(f)) >= 0 )
      theJob->symtable[i].last = theJob->symtable + k;
    else theJob->symtable[i].last = 0;
    if ( (k = nint(f)) >= 0 )
      theJob->symtable[i].next = theJob->symtable + k;
    else theJob->symtable[i].next = 0;
  }

  for ( i = 0; i < 3; i++ )
    for ( j = 0; j < CMAX+8; j++ )
      if ( (k = nint(f)) < 0 ) theJob->symbol[i][j] = 0;
      else theJob->symbol[i][j] = theJob->symtable + k;

  for ( i = 0; i < CMAX; i++ )
    if ( (j = nint(f)) < 0 ) theJob->dcs[i] = 0;
    else theJob->dcs[i] = theJob->symtable + j;

  if ( (i = nint(f)) < 0 ) theJob->symtab = 0;
  else theJob->symtab = theJob->symtable + i;

  for ( i = 0; i < FMAX; i++ ) {
    theJob->form[i].lsub = nint(f);
    theJob->form[i].rsub = nint(f);
    if ( (j = nint(f)) < 0 ) theJob->form[i].sym = 0;
    else theJob->form[i].sym = theJob->symtable + j;
  }
  fclose(f);

  F_N =
    ((theJob->f_n && valid[theJob->logic][AxFN])
     || theJob->axiom[AxFN]);
  afx =
    ((valid[theJob->logic][RulPref] && valid[theJob->logic][RulSuff])
     || (theJob->axiom[RulPref] && theJob->axiom[RulSuff]));
}



/*
*	This dumps the current contents of theJob in a just-about-human-
*	readable form to a file specified by the user.
*	The stored job can be recovered by means of read_job.
*	Outjob replaces the job display in xdialog mode.
*/

void outjob(FILE *f)
{
  int i, j, k;

  for ( i = 0; i < AXMAX; i++ ) {
    j = (int) theJob->axiom[i];
    fprintf(f, " %d", j);
  }

  fprintf(f,"\n");
  for ( i = 0; i < CMAX; i++ ) fprintf(f, " %d", theJob->adicity[i]);

  for ( i = 0; i < TMAX; i++ )
    for ( j = 0; j < RTMAX; j++ )
      fprintf(f, " %d", theJob->croot[i][j]);

  for ( i = 0; i < TMAX; i++ )
    for ( j = 0; j < RTMAX; j++ )
      fprintf(f, " %d", theJob->proot[i][j]);

  fprintf(f,"\n");
  for ( i = 0; i < CMAX; i++ ) fprintf(f, " %d", theJob->defcon[i]);

  for ( i = 0; i < CMAX; i++ ) fprintf(f, " %d", theJob->concut[i]? 1: 0);

  fprintf(f, " %d", theJob->failure);
  fprintf(f, " %d", theJob->logic);

  fprintf(f, " %d %d %d %d %d %d %d",
	  theJob->f_n, theJob->f_lat, theJob->f_t,
	  theJob->f_T, theJob->f_F, theJob->f_fus, theJob->f_nec);

  fprintf(f, " %d %d %d", 
	  theJob->maxtime, theJob->maxmat, theJob->sizmax);
  fprintf(f, " %d %d %d",
	  theJob->sizmax_ismax, theJob->totord, theJob->distrib);
  fprintf(f, " %d %d",
	  theJob->tty_out, theJob->fil_out);

  fprintf(f,"\n");
  for ( i = 0; theJob->data_dir[i]; i++ ) {
    j = theJob->data_dir[i];
    fprintf(f, " %d", j);
  }
  fprintf(f, " 0 ");

  for ( i = 0; theJob->outfil_name[i]; i++ ) {
    j = theJob->outfil_name[i];
    fprintf(f, " %d", j);
  }
  fprintf(f, " 0 ");

  for ( i = 0; logic_name[theJob->logic][i]; i++ ) {
    j = logic_name[theJob->logic][i];
    fprintf(f, " %d", j);
  }
  fprintf(f, " 0 ");

  for ( i = 0; i < SYMBOLMAX; i++ ) {
    for ( j = 0; j < 16; j++ ) {
      k = theJob->symtable[i].s[j];
      fprintf(f, " %d", k);
    }
    if ( theJob->symtable[i].last )
      k = theJob->symtable[i].last - theJob->symtable;
    else k = -1;
    fprintf(f, " %d", k);
    if ( theJob->symtable[i].next )
      k = theJob->symtable[i].next - theJob->symtable;
    else k = -1;
    fprintf(f, " %d", k);
  }

  for ( i = 0; i < 3; i++ )
    for ( j = 0; j < CMAX+8; j++ ) {
      if ( theJob->symbol[i][j] )
	k = theJob->symbol[i][j] - theJob->symtable;
      else k = -1;
      fprintf(f, " %d", k);
    }

  for ( i = 0; i < CMAX; i++ ) {
    if ( theJob->dcs[i] )
      j = theJob->dcs[i] - theJob->symtable;
    else j = -1;
    fprintf(f, " %d", j);
  }
  if ( theJob->symtab )
    i = theJob->symtab - theJob->symtable;
  else i = -1;
  fprintf(f, " %d", i);

  fprintf(f,"\n");
  for ( i = 0; i < FMAX; i++ ) {
    fprintf(f, " %d", theJob->form[i].lsub);
    fprintf(f, " %d", theJob->form[i].rsub);
    if ( theJob->form[i].sym )
      j = theJob->form[i].sym - theJob->symtable;
    else j = -1;
    fprintf(f, " %d", j);
  }
  fprintf(f,"\n");
}




/*
*	Trim converts answer to upper case and removes
*	any non-alphanumeric characters.
*/

void trim()
{
  int i, j = 0;

  for ( i = 0; answer[i]; i++ ) {
    if ( islower(answer[i]) )
      answer[i] = toupper(answer[i]);
    if ( isupper(answer[i]) || isdigit(answer[i]) )
      answer[j++] = answer[i];
  }
  while ( j < i )
    answer[j++] = '\0';
}


/*
 *  By contrast, nospace just removes any white space characters,
 *  including the line feed if any. The parameter is assumed to be
 *  a null-terminated character string. The method is maximally
 *  crude, but the strings are very short so it's OK.
 */

void nospace(char *s)
{
  int i,j;

  for ( i = 0; s[i]; i++ ) if ( isspace(s[i]) )
    for ( j = i;; j++ )
      if ( !(s[j] = s[j+1]) ) break;
}




/*
*	Delete_saxiom is just a subroutine of delete, taking 
*	care of case (a).
*/

void delete_saxiom()
{
  int i, j = 0;

  for ( i = 1; i < AXMAX; i++ ) 
    if ( theJob->axiom[i] ) {
      if ( j ) j = AXMAX;
      else j = i;
    }
  if ( !j ) return;
  if ( j < AXMAX ) theJob->axiom[j] = 0;
  else {
    if ( !xdialog ) {
      for ( i = 1; i < AXMAX; i++ ) 
	if ( theJob->axiom[i] ) {
	  printf("\n %2d  ", i);
	  print_axiom(stdout,i); 
	}
      printf("\n\n Delete which one? (if none type \"0\")   ");
      fflush(stdout);
    }
    scanf("%d", &i);  READLN;
    if ( i > 0 && i < AXMAX ) theJob->axiom[i] = 0;
  }
}




/*
*	Delete_uaxiom is just a subroutine of delete, taking 
*	care of case (d).
*/

void delete_uaxiom()
{
  int i=0, j=0, k=0;

  if ( !**(theJob->croot) ) return;
  if ( theJob->croot[1][0] && !xdialog ) {
    for ( i = 0; theJob->croot[i][0]; i++ ) {
      printf("\n %d:  ", i+1);
      for ( j = 0; theJob->proot[i][j]; j++ ) {
	if ( j ) printf(", ");
	if ( theJob->proot[i][j] != TRIVIAL )
	  outfml(theJob->proot[i][j],
		 theJob->proot[i][j],stdout);
      }
      if ( theJob->proot[i][0] != TRIVIAL 
	   || theJob->croot[i][1] )
	printf("  /  ");
      for ( j = 0; theJob->croot[i][j]; j++ ) {
	if ( j ) printf(",");
	if ( theJob->croot[i][j] != ABSURD )
	  outfml(theJob->croot[i][j],
		 theJob->croot[i][j],stdout);
      }
    }
    printf("\n\n Delete which one? (if none type \"0\")  ");
    fflush(stdout); scanf("%d", &j); READLN;
    if ( j < 1 || j > i ) return;
    j--;
  }
  else if ( xdialog ) scanf("%d", &j);
  else {
    i = 1;
    j = 0;
  }
  while ( ++j <= i ) 
    for ( k = 0; k < RTMAX; k++ ) {
      theJob->proot[j-1][k] = theJob->proot[j][k];
      theJob->croot[j-1][k] = theJob->croot[j][k];
    }
}



/*
*	Delete_connective is just a subroutine of delete, taking 
*	care of case (c).
*/

void delete_connective()
{
  int	i, j;
  int	badplace = 0;
  int	badadic;
  char	badstr[SLEN];
  symb	badcon;

  if ( !theJob->dcs[0] ) return;
  if ( xdialog ) {
    fgets(badstr,SLEN,stdin);
    nospace(badstr);
  }
  else if ( !theJob->dcs[1] ) strcpy(badstr,theJob->dcs[0]->s);
  else {
    printf("\n\n Delete which connective?   ");
    fflush(stdout);
    for (;;) {
      fgets(badstr,SLEN,stdin);
      nospace(badstr);
      if ( !*badstr ) return;
      for ( badplace = 0; theJob->dcs[badplace]; badplace++ )
	if ( !strcmp(badstr,theJob->dcs[badplace]->s) ) break;
      if ( theJob->dcs[badplace] ) break;
    }
  }
  badadic = theJob->adicity[badplace];
  badcon = theJob->dcs[badplace];

  for ( i = 0; theJob->dcs[i]; i++ )
    if ( theJob->defcon[i] != PRIMITIVE && 
	 is_in( badcon, theJob->defcon[i] ))
      return(nodelcon("connective",theJob->dcs[i],
		      theJob->defcon[i]));
  for ( i = 0; theJob->croot[i][0]; i++ ) {
    for ( j = 0; theJob->croot[i][j]; j++ )
      if ( is_in( badcon, theJob->croot[i][j] ))
	return(nodelcon("an axiom",'\0',
			theJob->croot[i][j]));
    for ( j = 0; theJob->proot[i][j]; j++ )
      if ( is_in( badcon, theJob->proot[i][j] ))
	return(nodelcon("a rule premise",'\0',
			theJob->proot[i][j]));
  }
  if ( theJob->failure && 
       is_in( badcon, theJob->failure ))
    return(nodelcon("the badguy",'\0',theJob->failure));
  
  for ( i = badplace; theJob->dcs[i]; i++ ) {
    theJob->adicity[i] = theJob->adicity[i+1];
    theJob->dcs[i] = theJob->dcs[i+1];
    theJob->defcon[i] = theJob->defcon[i+1];
    theJob->concut[i] = theJob->concut[i+1];
  }

  for ( i = 0; theJob->symbol[badadic][i] != badcon; i++ ) ;
  for ( j = i; theJob->symbol[badadic][j]; j++ )
    theJob->symbol[badadic][j] = theJob->symbol[badadic][j+1];
  purge( badcon );
  *(badcon->s) = 0;
  if ( badcon->last ) badcon->last->next = badcon->next;
  if ( badcon->next ) badcon->next->last = badcon->last;
  if ( badcon == theJob->symtab ) theJob->symtab = badcon->next;
  badcon->last = badcon->next = 0;
}



/*
*	If we bump out of delete_connective, it is for a reason,
*	communicated to the user thus:
*/

void nodelcon(char *s, symb spec, int formula)
{
  EP printf("\n\n Sorry, it's needed to specify %s", s);
  if ( spec ) printf(" %s", spec->s);
  printf(":   ");
  outfml(formula,formula,stdout);
  puts("");
  paws();
  return;
}
