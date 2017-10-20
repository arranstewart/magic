/*
*			mp_parse.c		V2.1 (May 1993)
*
*	The parser for MaGIC and like programs.  This accepts
*	formulas in I Block normal form (that is, with our usual
*	conventions about scope, association to the left, dots
*	in place of parentheses etc.  See works of R.K. Meyer or
*	the author for details.  Actual scope ordering is passed
*	by means of the string cn which records the available 
*	connectives with the dyadic ones in scope order.
*
*	Programs such as minlog also use this parser, so although
*	for example WFF is defined in MaGIC.h it is defined again
*	here to make life easier at some points.  The WFF structure
*	must have at least the three fields specified here, though
*	of course it may have others if it is externally defined.
*
*	Note that formula #0 is regarded as the dummy, to be 
*	returned in case of failure.
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

#define P_OK (c && c->s[0] != ')')
#define ERROR(sy,ws) { strcpy(P_ERR[0],(sy)->s); strcpy(P_ERR[1],ws); return 0; }
#define ERROR1(ws) { strcpy(P_ERR[0],""); strcpy(P_ERR[1],ws); return 0; }

static char P_ERR[2][32];





/*
*	Now for the parser.  It returns the offset of the successfully
*	parsed formula, or 0 if the parse attempt was unsuccessful.
*
*	s_parse (parse a subformula) is called to get the result.
*	Finally, if an error was recorded, it is output.
*
*	The parameters are the text to be parsed, the destination 
*	array of subformulas and the list of available connectives.
*/

int parse( symb fla[] )  
{
  int rc;

  rc = s_parse( fla );
  if ( !rc ) {
    if ( xdialog ) printf("E");
    printf("\n ERROR: \"%s\" %s",P_ERR[0],P_ERR[1]);
  }
  return rc;
}





/*
*	S_parse parses successive subformulas which, separated by 
*	dyadic connectives, make up the string.  It then calls 
*	Finish to sort out the binary parsing according to the 
*	scope conventions.
*
*	This is a one-pass parser.
*/

int s_parse( symb fla[] )
{
  int sbf[20];		/* Dyadic-separated subformulas	*/
  int pptr=0;		/* Offset of current symbol	*/
  int oldptr;		/* Previous value of pptr 	*/
  int ns = 0;		/* "Next subformula"		*/
  int i,j;
  symb dcn[19];		/* Separating dyadics		*/
  symb c;
   
  c = *fla;
  while ( P_OK ) {
    oldptr = pptr;
    while ( symbol_listed(1,(c=fla[pptr++])) ) ;
    i = pptr-1;
    if ( is_var(c) || symbol_listed(0,c) )
      sbf[ns] = Loc(c, 0, 0);
    else if ( c->s[0] == '(' || c->s[0] == '.' ) {
      if ( !(sbf[ns] = s_parse( fla+pptr)) ) return 0;
      j = Match(fla+pptr-1);
      if ( !j ) return 0;
      pptr += j-1;
    }
    else	ERROR(fla[pptr-1]," can't begin a subformula");
    if ( symbol_listed(1,fla[oldptr]) )
      while ( i > oldptr ) 
	sbf[ns] = Loc(fla[--i], 0, sbf[ns]);
    c = fla[pptr];
    if ( P_OK ) {
      if ( symbol_listed(2,c) )
	dcn[ns++] = c;
      else ERROR(fla[pptr]," found where dyadic connective expected");
      c = fla[++pptr];
      if ( !P_OK)
	ERROR(fla[pptr]," found where subformula expected to start");
    }
  }
  return Finish(sbf, dcn, ns);
}






/*
*	Return the offset of the right parenthesis matching the
*	left one at start.  Remember a dot is a left parenthesis
*	whose imaginary right mate is as far to the right as is
*	reasonable.
*/

int Match( symb start[] )
{
  int i;

  for ( i = 1; start[i] && start[i]->s[0] != ')'; i++ )
    if ( start[i]->s[0] == '(' )
      i += Match(start+i)-1;
  if ( start[0]->s[0] == '.' )
    return i;
  if ( !start[i] )
    ERROR1("Unmatched left parenthesis");
  return i+1;
}





/*
*	Given a formula A1 * .... * An where each Ai is a formula
*	and each * is some dyadic connective, turn it into a binary
*	wff tree as advertised in the definition of a formula.
*
*	Array subf holds the offsets of the main subformulas. The
*	corresponding connectives are in conn, while totl is the 
*	number of subformulas.
*/

int Finish( int subf[], symb conn[], int totl )
{
  int i, m = 0;
  int sk;

  if ( !totl ) return *subf;
  sk = 0;
  for (i = 0; i < totl; i++)
    if ( symbol_position(2,conn[i]) >= sk) {
      m = i;
      sk = symbol_position(2,conn[i]);
    }
  i = m+1;
  return Loc(conn[m],
	     Finish(subf, conn, m),
	     Finish(subf+i, conn+i, totl-i)
	     );
}





/*
*	Locate the given formula in the stack of those already
*	parsed and return its offset.  If it is not already there
*	then add it and return its offset.
*
*	Note that in order to force variables to the bottom of the
*	stack of subformulas, there may be a section reserved for 
*	them with symbols initialised to '?'.  If not, never mind.
*/

int Loc( symb mn, int lft, int rgt )
{
  int i;

  for (i = 0; theJob->form[i].sym; i++)
    if ( theJob->form[i].sym == mn && theJob->form[i].lsub == lft
	 && theJob->form[i].rsub == rgt )
      return i;

    else if ( is_var(mn) && theJob->form[i].sym->s[0] == '.' )
      break;

  theJob->form[i].sym = mn; 
  theJob->form[i].lsub = lft;
  theJob->form[i].rsub = rgt;  
  return i;
}





/*
*	A variable is any lower-case letter that is not a connective.
*/

boolean is_var(symb x)
{
  int i;

  for ( i = 0; i < 3; i++ )
    if ( symbol_listed(i,x) ) return 0;
  return (isalpha(x->s[0]) && !(x->s[1]));
}




/*
*	Return the offset of symbol s in array theJob->symbol[x].
*/

int symbol_position( int x, symb s )
{
	
  int i;

  for ( i = 0; theJob->symbol[x][i]; i++ )
    if ( theJob->symbol[x][i] == s ) return i;
  return -1;
}
