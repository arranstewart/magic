/*
*			dialog.c		V2.1 (May 1993)
*
*	The main interactive part of MaGIC.c.  This module contains 
*	some of the procedures to be executed in response to menu 
*	selections, most of the others being in getjob.c.
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
*	The dialog function gets the job specifications from the
*	user, setting up theJob accordingly.  It returns 1 if 
*	option 'g' is selected from the menu, 0 if 'e' or 'q' is 
*	selected and n > 1 to change (in the parallel case) to n 
*	processes.
*/
 
int dialog(boolean batch, char *batchfile)
{
  if ( batch ) return read_job( batchfile );

  for(;;) switch( menu() ) {
  case 'a': add_axioms();		break;
  case 'b': bad_guy();		break;
  case 'c': connective();		break;
  case 'd': deletion();		break;
  case 'e':
  case 'q':		return 0;
  case 'f': fragment();		break;
  case 'g':		return 1;
  case 'h': help(0); paws();	break;
  case 'i': input_direct();	break;
  case 'j': jump_condition();	break;
  case 'k': job_defaults(batch);	break;
  case 'l': logic_choice();	break;
  case 'm': print_version();	break;
  case 'n':
  case '#':		return n_of_procs();
  case 'o': order_change();	break;
  case 'p': print_options();	break;
  case 'r': read_job( 0 );	break;
  case 's': store_job();	
  }
}




/*
*	Menu first ensures that the fragment is consistent with the
*	selected logic, axioms and badguy.  Then it displays the 
*	currently selected job and the menu.  Finally it takes the
*	next user choice of menu selection and returns the character
*	code for the next action.
*/

char menu()
{
  char readin();

  set_frag( true );
  if ( xdialog ) {
    printf("J\n");
    outjob( stdout );
  }
  else display();
  return readin("abcdefghijklmnopqrst#");
}
    



/*
*	Readin returns the first character from stdin which matches 
*	one in its parameter, converting to lower case as required.
*	It then discards all characters up to and including the next 
*	line feed.  It is used mainly to get menu selections.
*/

char readin(char *str)
{
  char ch;
  int i;

  fflush(stdout);
  for(;;) {
    fgets(answer,SLEN,stdin);
    ch = 0;
    for ( i = 0; answer[i]; i++ ) {
      if ( isupper(ch = answer[i]) ) ch = tolower(ch);
      if ( strchr(str,ch) ) return ch;
    }
  }
}




/*
*	"Pause".
*/

void paws()
{
  if ( xdialog ) fflush(stdout);
  else {
    printf("\n\n Now type RETURN to continue.........");
    READLN;
  }
}




/*
*	Job_defaults is the initialisation procedure.  It gets the
*	initial choice of system and sets theJob to its defaults.
*/

void job_defaults( boolean batch )
{
  int	i, j;

  zero = 0;

  for ( i = 0; i < SYMBOLMAX; i++ ) {
    strcpy( theJob->symtable[i].s, "" );
    theJob->symtable[i].last = theJob->symtable[i].next = 0;
  }
  for ( i = 0; i < 3; i++ )
    for ( j = 0; j < CMAX+8; j++ )
      theJob->symbol[i][j] = 0;
  theJob->symtab = 0;

  newsymbol( "(", 0, 0 );
  newsymbol( ")", 0, 0 );
  newsymbol( ".", 0, 0 );
  newsymbol( "a", 0, 0 );
  newsymbol( "b", 0, 0 );
  newsymbol( "t", theJob->symbol[0], 0 );
  newsymbol( "f", theJob->symbol[0], 0 );
  newsymbol( "T", theJob->symbol[0], 0 );
  newsymbol( "F", theJob->symbol[0], 0 );
  newsymbol( "~", theJob->symbol[1], 0 );
  newsymbol( "!", theJob->symbol[1], 0 );
  newsymbol( "?", theJob->symbol[1], 0 );
  newsymbol( "o", theJob->symbol[2], 0 );
  newsymbol( "&", theJob->symbol[2], 0 );
  newsymbol( "v", theJob->symbol[2], 0 );
  newsymbol( "->", theJob->symbol[2], 0 );

  strcpy( theJob->data_dir, DATA_DIR );

  for ( i = 0; i <= CMAX; i++ ) {
    theJob->dcs[i] = 0;
    if ( i < CMAX ) {
      theJob->defcon[i] = 0;
      theJob->concut[i] = false;
    }
  }

  for ( i = 0; i < TMAX; i++ )
    for ( j = 0; j < RTMAX; j++ )
      theJob->croot[i][j] = theJob->proot[i][j] = 0;
  theJob->failure = 0;

  for ( i = 1; i < AXMAX; i++ )
    theJob->axiom[i] = 0;

  theJob->tty_out = PRETTY;
  theJob->fil_out = NONE;  
  theJob->outfil_name[0] = '\0';

  theJob->maxmat = 0;
  theJob->maxtime = 0;

  Sizmax = theJob->sizmax = SZ;
  theJob->sizmax_ismax = true;

  wff_initial();

  if ( !batch ) {
    if ( !noclear ) {
#ifdef __CYGWIN__
      puts("\033[2J");
#else
      system("clear");
#endif
    }
    theJob->logic = FD;
    if ( !xdialog ) {
      printf("\n This is MaGIC %s, finding matrices for", VERSION);
      printf(" your favourite logic.\n");
      printf(" Matrices come in all sizes up to %dx%d.\n",
	     SZ,SZ);
      logic_choice();
    }
  }
}




/*
*	Set_frag makes the selected fragment include all the used
*	connectives and sets Sizmax accordingly.
*/

void set_frag( boolean set_sizmax )
{
  int i, j;

  for ( i = 0; i < AXMAX; i++ ) if ( theJob->axiom[i] ) {
    if ( strchr(ax_string[i],'~') || strchr(ax_string[i],'f')
	 || strchr(ax_string[i],'?') )
      theJob->f_n = 1;
    if ( strchr(ax_string[i],'t') || strchr(ax_string[i],'f') ) 
      theJob->f_t = 1;
    if ( strchr(ax_string[i],'&') || strchr(ax_string[i],'v') )
      theJob->f_lat = 1;
    if ( strchr(ax_string[i],'!') || strchr(ax_string[i],'?') )
      theJob->f_nec = 1;
    if ( i == AxK || i == AxK2 || i == AxTF )
      theJob->f_T = theJob->f_t = 1;
  }
  if ( theJob->f_n && valid[theJob->logic][AxC] )
    theJob->f_fus = 1;

  for ( i = 0; theJob->croot[i][0]; i++ ) {
    for ( j = 0; theJob->croot[i][j]; j++ )
      if ( theJob->croot[i][j] != ABSURD )
	check_frag(theJob->croot[i][j]);
    for ( j = 0; theJob->proot[i][j]; j++ )
      if ( theJob->proot[i][j] != TRIVIAL )
	check_frag(theJob->proot[i][j]);
  }
  if ( theJob->failure )
    check_frag(theJob->failure);
  for ( i = 0; theJob->dcs[i]; i++ )
    if ( theJob->defcon[i] != PRIMITIVE ) 
      check_frag(theJob->defcon[i]);

  if ( theJob->totord )
    theJob->f_lat = 1;
  if ( theJob->f_lat )
    theJob->f_t = theJob->f_T = theJob->f_F = 1;
  if (( theJob->f_F || theJob->f_T ) && theJob->f_n ) 
    theJob->f_F = theJob->f_T = 1;

  F_N =
    ((theJob->f_n && valid[theJob->logic][AxFN])
     || theJob->axiom[AxFN]);
  afx =
    ((valid[theJob->logic][RulPref] && valid[theJob->logic][RulSuff])
     || (theJob->axiom[RulPref] && theJob->axiom[RulSuff]));

  if ( set_sizmax ) {
    if ( !(theJob->f_lat || theJob->f_n) )
      Sizmax = theJob->f_t?  S_pot: S_pO;
    else if ( !theJob->f_lat )
      Sizmax = theJob->f_t?  S_pont: S_poN;
    else if ( !theJob->distrib )
      Sizmax = theJob->f_n? S_ln: S_lat;
    else if ( !theJob->totord )
      Sizmax = ((theJob->f_n && theJob->logic==S4)
		|| theJob->axiom[AxBA])? S_ba:
      (theJob->f_n? S_dln: S_dlat);
    else	Sizmax = theJob->f_n? S_Ton: S_to;

    if ( theJob->sizmax > Sizmax || theJob->sizmax_ismax ) 
      theJob->sizmax = Sizmax;
  }
}




/*
*	This function adds to the fragment any connectives which
*	occur in the given formula.  It attends to the main 
*	connective only, calling itself recursively to deal with
*	the subformulae.
*/

void check_frag(int x)
{
  if (!x) return;
  if (!theJob->form[x].sym->s[1]) {
    switch(theJob->form[x].sym->s[0]) {
    case '~': theJob->f_n = 1; break;
    case 'f': theJob->f_n = 1;
    case 't': theJob->f_t = 1; break;
    case 'T': theJob->f_T = 1; break;
    case 'F': theJob->f_F = 1; break;
    case 'o': theJob->f_fus = 1; break;
    case '?': theJob->f_n = 1;
    case '!': theJob->f_nec = 1; break;
    case '&':
    case 'v': theJob->f_lat = 1;
    }
  }
  check_frag(theJob->form[x].lsub);
  check_frag(theJob->form[x].rsub);
}




/*
*	Print_axiom just writes out axiom #x on stream f.
*/

void print_axiom(FILE *f, AXIOM x)
{
  fprintf(f, "%s", ax_string[x]);
}




/*
*	Display shows the current choices and the menu.
*/

void display()
{
  if ( !noclear ) {
#ifdef __CYGWIN__
    puts("\033[2J");
#else
    system("clear");
#endif
  }
  disp(stdout);
  printf("    A)xiom    B)adguy      C)onnective   D)elete\n");
  printf("    E)xit     F)ragment    G)enerate     H)elp\n"); 
  printf("    I)O       J)ump        K)ill         L)ogic\n");
  printf("    M)aGIC    N)o. Procs   O)rder        P)rint Opts\n");
  printf("    Q)uit     R)ead        S)tore                       ");
}




/*
*	Disp is the function to print an account of the current
*	contents of theJob.  It is to have verbose and terse modes.
*/

void disp(FILE *f)
{
  AXIOM ax;
  int i, j;
  boolean b;

#ifdef PARALLEL
  fprintf(f, "\n Parallel MaGIC running %d out of %d processes\n", 
	  gm->nprocs, PARALLEL);
#endif

  if ( theJob->totord && default_orders[theJob->logic] != total_orders )
    fprintf(f, "\n Logic:%8cT%s", ' ', logic_name[theJob->logic]);
  else if ( theJob->distrib &&
	    default_orders[theJob->logic] != distributive_lattices )
    fprintf(f, "\n Logic:%8cD%s", ' ', logic_name[theJob->logic]);
  else if ( !theJob->distrib &&
	    default_orders[theJob->logic] != lattices )
    fprintf(f, "\n Logic:%8cL%s", ' ', logic_name[theJob->logic]);
  else fprintf(f, "\n Logic:%8c%s", ' ', logic_name[theJob->logic]);

  b = false;
  for ( ax = AxNull+1; ax < AXMAX; ax++ ) if ( theJob->axiom[ax] ) {
    if ( !b ) {
      b = true;
      fprintf(f, "\n\n Plus:         ");
    }
    else fprintf(f, "\n               ");
    print_axiom(f,ax);
  }

  for ( i = 0; theJob->croot[i][0]; i++ ) {
    if ( i ) fprintf(f,   "\n               "); 
    else     fprintf(f, "\n\n Extra:        ");
    for ( j = 0; theJob->proot[i][j]; j++ ) {
      if ( j ) fprintf(f, ", ");
      if ( theJob->proot[i][j] != TRIVIAL )
	outfml(theJob->proot[i][j],
	       theJob->proot[i][j],f);
    }
    if ( theJob->proot[i][0] != TRIVIAL ) fprintf(f, "  /  ");
    for ( j = 0; theJob->croot[i][j]; j++ ) {
      if ( j ) fprintf(f, ", ");
      if ( theJob->croot[i][j] != ABSURD )
	outfml(theJob->croot[i][j],
	       theJob->croot[i][j],f);
    }
  }

  fprintf(f, "\n\n Fragment:     ->");
  if ( theJob->f_lat ) fprintf(f, ", &, v");
  if ( theJob->f_n ) fprintf(f, ", ~");
  if ( theJob->f_fus ) fprintf(f, ", o");
  if ( theJob->f_nec ) {
    fprintf(f, ", !");
    if ( theJob->f_n ) fprintf(f, ", ?");
  }
  if ( theJob->f_t ) {
    fprintf(f, ", t"); 
    if ( theJob->f_n ) fprintf(f, ", f");
  }
  if ( theJob->f_T ) fprintf(f, ", T");
  if ( theJob->f_F ) fprintf(f, ", F");

  for ( i = 0; theJob->defcon[i]; i++ ) {
    if ( i ) fprintf(f,   "\n             ");
    else if ( !theJob->defcon[1] ) fprintf(f, "\n\n Definition: ");
    else     fprintf(f, "\n\n Definitions:");
    if ( !theJob->adicity[i] ) 
      fprintf(f, "  %s        ", theJob->dcs[i]->s);
    else if ( theJob->adicity[i] == 1 ) {
      if ( theJob->dcs[i]->s[1] )
	fprintf(f, "  %s a       ", theJob->dcs[i]->s);
      else fprintf(f, "  %sa       ", theJob->dcs[i]->s);
    }
    else	fprintf(f, "  a %s b    ", theJob->dcs[i]->s);
    if ( theJob->defcon[i] == PRIMITIVE ) {
      fprintf(f, "Primitive");
      if ( theJob->concut[i] ) fprintf(f, "  (cut)");
    }
    else outfml(theJob->defcon[i], theJob->defcon[i],f);
  }

  if ( theJob->failure ) {
    fprintf(f, "\n\n Fail:         ");
    outfml(theJob->failure, theJob->failure, f);
  }

  fprintf(f, "\n\n TTY output:   %s\n File output:  %s",
	  (theJob->tty_out==NONE? "none": 
	   (theJob->tty_out==UGLY? "ugly": 
	    (theJob->tty_out==PRETTY? "pretty": "summary"))), 
	  (theJob->fil_out==NONE? "none": 
	   (theJob->fil_out==UGLY? "ugly": 
	    (theJob->fil_out==PRETTY? "pretty": "summary"))));
  if ( theJob->fil_out ) 
    fprintf(f,"\n Output file:  \"%s\"", theJob->outfil_name);

  fprintf(f, "\n\n Search concludes ");
  if (theJob->maxtime)
    fprintf(f, "after %d seconds\n or ", theJob->maxtime);
  if (theJob->maxmat)
    fprintf(f, "when %d matri%s found\n or ", 
	    theJob->maxmat, (theJob->maxmat==1? "x": "ces"));
  fprintf(f, "when size %d finished.\n\n\n", theJob->sizmax);
}





/*
*	The Help facility is extremely primitive.  It simply transfers
*	the contents of a short file to stdout.  The parameter codes
*	the place in the program from which the Help call was made.
*/

void help(helpcode x)
{ 
  switch( x ) {
  case MEN:
    put_out("MEN");
    break;
  case WFF1: 
  case WFF2:
    put_out("WFF");
    break;
  case FDL:
    put_out("FDL");
    break;
  case BTW:
    put_out("BTW");
    break;
  case LOG:
    put_out("LOG");
    break;
  case OUT:
    put_out("OUT");
    break;
  case HELPMAX:
    break;
  }
}




/*
*	This is essentially a macro for Help (above).
*	It prepends the data directory name, appends ".show" 
*	and does the file transfer.
*
*	If we are in terse mode, nothing happens because the
*	front end xmagic has its own Help routine.
*/

void put_out(char *f_nm)
{
  FILE *f1;
  char s[100];

  if ( !noclear ) {
#ifdef __CYGWIN__
    puts( "\033[2J" );
    strcpy(s,DATA_DIR);
    strcat(s,f_nm);
    strcat(s,".show");
    if ( (f1 = fopen(s,"r")) == NULL ) {
      printf("Cannot open %s.",s);
    }
    else {
      while (fgets(s,80,f1) != NULL) {
        printf("%s",s);
      }
      fclose(f1);
    }
#else
    sprintf(s,"clear; more %s%s.show", DATA_DIR, f_nm);
    system( s );
#endif
  }
}
