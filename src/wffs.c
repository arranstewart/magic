/*
*			wffs.c			V2.1 (May 1993)
*
*	This contains the routines from MaGIC (other than the
*	parser) which deal with formulas.
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

#define oldsymbol(s) newsymbol(s,0,0)
#define found_symbol(s) (this_symbol(s)? true: false)



/*
*	This grabs, initialises and returns the symbol with the
*	given string, inserting it in the first unoccupied symbol
*	structure if need be. It is added to the end of the given 
*	symbol lists if these are non-null.
*
*	If the symbol already exists, no change is made to the 
*	symbol lists, even if they are non-null.
*/

symb newsymbol( char *string, symb symbol_list1[], symb symbol_list2[] )
{
  int i;
  symb sy = 0;

  for ( i = 0; i < SYMBOLMAX; i++ )
    if ( !strcmp(theJob->symtable[i].s,string) )
      return (theJob->symtable + i);

  for ( i = 0; i < SYMBOLMAX; i++ )
    if ( !theJob->symtable[i].s[0] ) {
      sy = theJob->symtable + i;
      strcpy(sy->s, string);
      sy->next = theJob->symtab;
      sy->last = 0;
      if ( theJob->symtab ) theJob->symtab->last = sy;
      theJob->symtab = sy;
      break;
    }
  if ( symbol_list1 ) {
    for ( i = 0; symbol_list1[i]; i++ ) ;
    symbol_list1[i] = sy;
    symbol_list1[i+1] = 0;
  }
  if ( symbol_list2 ) {
    for ( i = 0; symbol_list2[i]; i++ ) ;
    symbol_list2[i] = sy;
    symbol_list2[i+1] = 0;
  }
  return sy;
}





/*
*	Wff_initial initialises theJob->form etc.
*	It is called from dialog.c as part of setting 
*	up a new job.
*/
 
void wff_initial()
{
  int i;
  symb dummy_symbol;

  dummy_symbol = oldsymbol("no_connective");

  for ( i = 0; i < FMAX; i++ ) {
    theJob->form[i].lsub = theJob->form[i].rsub = 0;
    if ( i == 0 ) theJob->form[i].sym = dummy_symbol;
    else if ( i == 1 ) theJob->form[i].sym = oldsymbol("a");
    else if ( i == 2 ) theJob->form[i].sym = oldsymbol("b");
    else if ( i < VMAX ) theJob->form[i].sym = oldsymbol(".");
    else if ( i < VMAX+4+CMAX ) theJob->form[i].sym = oldsymbol(" ");
    else theJob->form[i].sym = 0;
  }
  theJob->form[VMAX].sym = oldsymbol("t");
  theJob->form[VMAX+1].sym = oldsymbol("f");
  theJob->form[VMAX+2].sym = oldsymbol("T");
  theJob->form[VMAX+3].sym = oldsymbol("F");
  
  for ( i = 0; i < CMAX; i++ ) {
    atom[0][i] = VMAX+4+CMAX+i;
    theJob->form[atom[0][i]].sym = oldsymbol("a");
    atom[1][i] = VMAX+4+(2*CMAX)+i;
    theJob->form[atom[1][i]].sym = oldsymbol("b");
  }
}







/*
*	Got_formula calls infml to read in a formula if possible.
*	If the user asks for help this is given, otherwise the 
*	boolean return value is 1 for success, 0 for failure.
*
*	The parameters x, y and yy are passed on to infml.  The
*	string s states what type of formula ("axiom", etc) is 
*	sought.
*/

boolean got_formula(int x, int y, int yy, char *s)
{ 
  for(;;)
    switch( infml(x,y,yy) ) {
    case -1:
      return(0);
    case 0:
      return(1);
    case 1:
      if ( xdialog ) return(0);
      printf("\n H)elp, R)epeat %s or N)either?   ", s);
      switch(readin("hnr")) {
      case 'n':
	return(0);
      case 'h':
	help(x+1);
	break;
      case 'r':
	printf("\n Definition:   ");
	fflush(stdout);
      }
    }
}





/*
*  Infml calls for and processes a formula from stdin.  The two
*  parameters are: x, the destination (IN_CONC = axiom or
*  conclusion, IN_PREM = premise, IN_BGUY = badguy, IN_DEFN =
*  definition); y, the offset for cases other than x = IN_BGUY; yy,
*  the second offset for cases x = IN_CONC and x = IN_PREM.
*
*  Return value is 0 if successful in reading a formula and getting
*  it parsed, 1 if an error is encountered, -1 if the null formula
*  is input.
*
*  The first action after input is to render the formula hygenic,
*  checking for bad characters at the same time.  A list of the
*  symbols is compiled from the input string.  By default, each
*  string of characters surrounded by white space (or parentheses)
*  is tried against the symbol table. If it is not found, initial
*  segments are tried.
*/

int infml(input_case x, int y, int yy)
{
  int i, k, wf;
  symb fml[SLEN*2];
  char longs[SLEN];

  if ( x != IN_DEFN && !xdialog ) printf("\n Enter formula (or RETURN):  ");
  fflush(stdout);
  fgets(answer,SLEN,stdin);

  k = 0;
  while ( next_symbol(longs) )
    if ( !seek_symbol(longs,fml,&k) ) {
      EP;
      printf("\n Illegal input: unrecognised symbol %s\n\n ", longs);
      return 1;
    }
  if ( !k ) return -1;
  fml[k] = 0;

  if ( x == IN_DEFN ) {
    if ( k == 1 ) {
      EP; printf("\n ERROR: definition is too short!\n\n");
      return 1;
    }
    if ( theJob->adicity[y] ) {
      for ( i = 0; i < k; i++ )
	if ( !strchr(fml[i]->s,'a') ) break;
      if ( i == k ) {
	EP; printf("\n ERROR: variable \"a\" does not ");
	printf("occur in the definition\n\n");
	return 1;
      }
    }
    if ( theJob->adicity[y] == 2 ) {
      for ( i = 0; i < k; i++ )
	if ( !strchr(fml[i]->s,'b') ) break;
      if ( i == k ) {
	EP; printf("\n ERROR: variable \"b\" does not ");
	printf("occur in the definition\n\n");
	return 1;
      }
    }
  }

  wf = parse(fml);
  if ( !wf ) return 1;
  if ( x == IN_CONC )
    theJob->croot[y][yy] = wf;
  else if ( x == IN_PREM )
    theJob->proot[y][yy] = wf;
  else if ( x == IN_BGUY )
    theJob->failure = wf;
  else {
    fix_atoms(y,wf);
    theJob->defcon[y] = wf;
  }
  return 0;
}



/*
*	Get the first space-delimited sequence of nonspace characters 
*	from answer[], truncate answer[] to the following portion and
*	return true. If there is no such sequence, return false.
*/

boolean next_symbol( char longs[] )
{
  int i, j;

  for ( i = 0; answer[i] && isspace(answer[i]); i++ );
  if ( !answer[i] ) return false;
  j = i;
  do {
    longs[i-j] = answer[i];
    i++;
  }
  while ( answer[i-1] && !isspace(answer[i-1]) ); i--;
  if ( answer[i] ) longs[i-j] = 0;
  for ( j = 0; answer[j]; j++ ) answer[j] = answer[j+i];
  return true;
}



/*
*	String occurs in the current symbol table.
*/

symb this_symbol( char *string )
{
  symb sy;

  for ( sy = theJob->symtab; sy; sy = sy->next )
    if ( !strcmp(sy->s,string) ) return sy;
  return 0;
}



/*
*	The "long string" longs may contain exactly a meaningful symbol,
*	in which case this is placed in fml[*k] and *k incremented.
*	Otherwise, the longest meaningful initial substring is placed
*	in fml[*k], *k incremented and the remainder of longs treated 
*	recursively. The value returned is true on success, false if no
*	meaningful initial string is found. 
*/

boolean seek_symbol( char longs[], symb fml[], int *k )
{
  char shorts[64], *cp;
  int i, j;

  if ( !*longs ) return true;
  if ( found_symbol(longs) ) {
    fml[(*k)++] = this_symbol(longs);
    return true;
  }

  if ( (cp = strchr(longs,'(')) && (j = cp - longs) ) {
    for ( i = 0; i < j; i++ ) shorts[i] = longs[i];
    shorts[i] = 0;
    if ( !seek_symbol(shorts, fml, k) ) return false;
    for ( i = 0; longs[i]; i++ ) longs[i] = longs[i+j];
  }		
  else if ( (cp = strchr(longs,')')) && (j = cp-longs) ) {
    for ( i = 0; i < j; i++ ) shorts[i] = longs[i];
    shorts[i] = 0;
    if ( !seek_symbol(shorts, fml, k) ) return false;
    for ( i = 0; longs[i]; i++ ) longs[i] = longs[i+j];
  }		

  if ( strchr(")(",*longs) ) {
    *shorts = *longs;
    shorts[1] = 0;
    fml[(*k)++] = oldsymbol(shorts);
    return seek_symbol(longs+1, fml, k);
  }

  strcpy( shorts, longs );
  for ( i = strlen(longs)-1; i; i-- ) {
    shorts[i] = 0;
    if ( found_symbol(shorts) ) {
      fml[(*k)++] = this_symbol(shorts);
      return seek_symbol(longs+i, fml, k);
    }
  }

  if ( isalpha(*longs) ) {
		*shorts = *longs;
		shorts[1] = 0;
		fml[(*k)++] = oldsymbol(shorts);
		return seek_symbol(longs+1, fml, k);
  }

  return false;
}




/*
*	Part of infml is to "fix the atoms".  This means change the
*	occurrences of 'a' and 'b' in the definition of defined 
*	connective #y to atom[0][y] and atom[1][y] respectively.
*	The other parameter, dy, points to the subformula within 
*	which to search for 'a' and 'b'.  Fix_atoms is recursive.
*/

void fix_atoms(int y, int wf)
{
  WFF *dy;

  if ( !wf ) return;
  dy = theJob->form + wf;

  if ( dy->lsub ) {
    if ( !strcmp(theJob->form[dy->lsub].sym->s,"a") ) 
      dy->lsub = atom[0][y];
    else if ( !strcmp(theJob->form[dy->lsub].sym->s,"b") ) 
      dy->lsub = atom[1][y];
    else fix_atoms(y,dy->lsub);
  }
  
  if ( dy->rsub ) {
    if ( !strcmp(theJob->form[dy->rsub].sym->s,"a") ) 
      dy->rsub = atom[0][y];
    else if ( !strcmp(theJob->form[dy->rsub].sym->s,"b") ) 
      dy->rsub = atom[1][y];
    else fix_atoms(y,dy->rsub);
  }
}





/*
*	Outfml is a very simple deparser which writes the formula
*	rooted at p, a subformula of that rooted at q, to stream f.
*
*	Local variable c is set to p->sym just for brevity.
*/

void outformula(int p, int q, FILE *f, varmode vm)
{
  symb c;

  if ( p <= 0 ) skipout("Attempt to print nonsense formula",SKIP);

  c = theJob->form[p].sym;

  if ( symbol_listed(2,c) ) {
    if ( p != q ) fprintf( f, "(" );
    outformula(theJob->form[p].lsub,q,f,vm);
    fprintf( f, " %s ", c->s );
    outformula(theJob->form[p].rsub,q,f,vm);
    if ( p != q ) fprintf( f, ")" );
  }
  else if ( symbol_listed(1,c) ) {
    fprintf( f, "%s", c->s );
    if ( strlen(c->s) > 1 ) fprintf( f, " " );
    outformula(theJob->form[p].rsub,q,f,vm);
  }
  else {
    if ( p <= Vmax && vm == VALS )
      fprintf( f, "%d", theJob->form[p].val );
    else fprintf( f, "%s", c->s );
  }
}



/*
*	Is_in returns 1 (true) if c occurs in the formula *w 
*	and 0 (false) if it does not.
*/

boolean is_in(symb s, int w)
{
  if ( w <= 0 ) return false;
  if ( theJob->form[w].sym == s ) return true;
  if ( is_in( s, theJob->form[w].lsub )) return true;
  return is_in(s, theJob->form[w].rsub);
}




/*
*       It is sometimes necessary to purge the formula list of any
*       appeal to a given symbol.  This is done especially when a
*       connective has just been deleted.
*
*	The first move is to propagate the bad symbol up the tree:
*	anything with a bad subformula is also bad.
*/

void purge( symb badsym )
{
  int i, j, k, m;

  for ( k = 1; theJob->form[k].sym; k++ ) {
    if ( theJob->form[theJob->form[k].lsub].sym == badsym )
      theJob->form[k].sym = badsym;
    if ( theJob->form[theJob->form[k].rsub].sym == badsym )
      theJob->form[k].sym = badsym;
  }

  j = 1;
  for ( i = 1; theJob->form[i].sym; i++ )
    if ( theJob->form[i].sym == badsym ) {
      if ( j < i ) j = i;
      while ( theJob->form[++j].sym == badsym ) ;
      if ( ! theJob->form[j].sym ) {
	do {
	  theJob->form[i].sym = 0;
	  theJob->form[i].lsub = 0;
	  theJob->form[i].rsub = 0;
	}
	while ( ++i <= j );
	return;
      }
      theJob->form[i].sym = theJob->form[j].sym;
      theJob->form[i].lsub = theJob->form[j].lsub;
      theJob->form[i].rsub = theJob->form[j].rsub;
      theJob->form[j].sym = badsym;

      for ( k = j+1; theJob->form[k].sym; k++ ) {
	if ( theJob->form[k].lsub == j )
	  theJob->form[k].lsub = i;
	if ( theJob->form[k].rsub == j )
	  theJob->form[k].rsub = i;
      }
      if ( theJob->failure == j ) theJob->failure = i;
      for ( k = 0; theJob->croot[k][0]; k++ )
	for ( m = 0; m < RTMAX; m++ ) {
	  if ( theJob->proot[k][m] == j )
	    theJob->proot[k][m] = i;
	  if ( theJob->croot[k][m] == j )
	    theJob->croot[k][m] = i;
	}
      for ( k = 0; theJob->dcs[k]; k++ )
	if ( theJob->defcon[k] == j ) theJob->defcon[k] = i;
    }
}
