/*			Output.c			March 1991
*
*	These are the functions for ugly, pretty and TeX output of
*	matrices.  They are called from RM.c which is used by all 
*	MaGIC post-processing programs.
*/



	/****************************************************************
	*								*
	*			    MaGIC 2.0				*
	*								*
	*	    (C) 1991 Australian National University		*
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


static int pnegno;
static int pordno;
static int pdesno;
static int pmatno;
static int pboxno;
static int pmatplus[CMAX];
static int singlematrix;


void displaymat(MATRIX *m, char *legend)
{
  FILE *f;

  f = fopen("/dev/tty","w");
  singlematrix = 1;
  system( "clear > /dev/tty" );
  printmat( m, PRETTY, UNSELECTED, f );
  fprintf( f, "\n\n %s", legend );
  singlematrix = 0;
  fclose(f);
}



void printmat(MATRIX *m, PRINTMODE printmode, COUNTERMODE countermode, FILE *f)
{
  int i;

  if ( countermode == SELECTED ) {
    pnegno = m->oknegno;
    pordno = m->okordno;
    pdesno = m->okdesno;
    pmatno = m->okmatno;
    pboxno = m->okboxno;
    FORALLCON(m,i)
      pmatplus[i] = m->okmatplus[i];
  }
  else if ( countermode == UNSELECTED ) {
    pnegno = m->negno;
    pordno = m->ordno;
    pdesno = m->desno;
    pmatno = m->matno;
    pboxno = m->boxno;
    FORALLCON(m,i)
      pmatplus[i] = m->matplus[i];
  }
  else Abort("Unrecognised print_counter mode: %d", countermode);

  switch (printmode) {
  case PRETTY:
    pu_print( m, m->cmax-1, f );
    if ( m->my_values[0] != -1 || m->my_string[0] ) {
      fprintf( f, "\n %s", m->my_string );
      for ( i = 0; m->my_values[i] != -1; i++ )
	fprintf( f, " %d", m->my_values[i] );
      fprintf( f, "\n" );
    }
    return;
  case UGLY:
    if ( m->total_put == 1 ) write_header( m );
    uu_print( m, m->cmax-1 );
    return;
  case TeX:
    if ( m->total_put == 1 ) tex_header();
    tex_uprint( m, m->cmax-1 );
    if ( m->my_values[0] != -1 || m->my_string[0] )
      tex_mystuff( m );
    return;
  case NONE:
    return;
  }
  Abort( "Unrecognised print mode: %d", printmode );
}



void write_header(MATRIX *m)
{
  int i;

  for ( i = 0; i < FRAGMAX; i++ )
    printf( " %d", m->fragment[i] );
  printf( "\n %d", m->cmax );
  FORALLCON(m,i)
    printf( " %d %s", m->adicity[i], m->dcs[i] );
  printf( "\n");
}



void pu_print(MATRIX *m, int x, FILE *f)
{
  if ( x < 0 ) {
    if ( m->fragment[BOX] ) pbox_print( m, f );
    else pC_print( m, f );
  }
  else {
    if ( pmatplus[x] == 1 || singlematrix )
      pu_print( m, x-1, f );
    fprintf( f, "\n\n %s table ", m->dcs[x] );
    u_number( m, x, f );
    switch( m->adicity[x] ) {
    case 0:
      nullprint( m, m->dcs[x], m->nulladic[x], f );
      break;
    case 1:
      monprint( m, m->dcs[x], 0, m->monadic[x], f );
      break;
    case 2:
      dyprint( m, m->dcs[x], 0, m->dyadic[x], "", 0, 0, f );
    }
  }
}


void uu_print(MATRIX *m, int x)
{
  if ( x < 0 ) {
    if ( m->fragment[BOX] ) ubox_print( m );
    else uC_print( m );
  }
  else {
    if ( pmatplus[x] == 1 || singlematrix ) {
      if ( m->total_put > 1 ) printf( " -1\n" );
      uu_print( m, x-1 );
    }
    switch( m->adicity[x] ) {
    case 0: unullprint( m, m->nulladic[x] );
      break;
    case 1: umonprint( m, m->monadic[x] );
      break;
    case 2: udyprint( m, m->dyadic[x] );
    }
  }
}



void pbox_print( MATRIX *m, FILE *f )
{
  if ( pboxno == 1 || singlematrix ) pC_print( m, f );
  fprintf( f, "\n\n Necessity " );
  box_number( m, f );
  monprint( m, "!", 0, m->box, f );
}



void ubox_print(MATRIX *m)
{
  if ( pboxno == 1 || singlematrix ) {
    if ( m->total_put > 1 ) printf( " -1\n");
    uC_print( m );
  }
  umonprint( m, m->box );
}



void pC_print(MATRIX *m, FILE *f)
{
  if ( pmatno == 1 || singlematrix ) pdesprint( m, f );
  fprintf( f, "\n\n Implication " );
  C_number( m, f );
  if ( m->fused )
    dyprint( m, "->", 0, m->C, "o", 0, m->fus, f );
  else
    dyprint( m, "->", 0, m->C, "", 0, 0, f );
}



void uC_print(MATRIX *m)
{
  if ( pmatno == 1 || singlematrix ) {
    if ( m->total_put > 1 ) printf( " -1\n");
    udesprint( m );
  }
  udyprint( m, m->C );
}



void pdesprint(MATRIX *m, FILE *f)
{
  if ( pdesno == 1 || singlematrix ) pordprint( m, f );
  fprintf( f, "\n\n\n Choice of truths " );
  des_number( m, f );
  monprint( m, "True", '-', m->designated, f );
}



void udesprint(MATRIX *m)
{
  if ( pdesno == 1 || singlematrix ) {
    if ( m->total_put > 1 ) printf( " -1\n");
    uordprint( m );
  }
  umonprint( m, m->designated );
}


void pordprint(MATRIX *m, FILE *f)
{
  if ( pordno == 1 || singlematrix ) {
    if ( m->fragment[NEG] ) pnegprint( m, f );
    else psizprint( m, f );
  }
  fprintf( f, "\n\n\n Order table " );
  ord_number( m, f );
  dyprint( m, "_\b<", '-', m->ord, "", 0, 0, f );
}


void uordprint(MATRIX *m)
{
  if ( pordno == 1 || singlematrix ) {
    if ( m->total_put > 1 ) printf( " -1\n");
    if ( m->fragment[NEG] ) unegprint( m );
    else usizprint( m );
  }
  udyprint( m, m->ord );
}



void pnegprint(MATRIX *m, FILE *f)
{
  if ( pnegno == 1 || singlematrix ) psizprint( m, f );
  fprintf( f, "\n\n\n Negation table " );
  neg_number( m, f );
  monprint( m, "~", 0, m->neg, f );
}



void unegprint(MATRIX *m)
{
  if ( pnegno == 1 || singlematrix ) {
    if ( m->total_put > 1 ) printf( " -1\n");
    usizprint( m );
  }
  umonprint( m, m->neg );
}



void psizprint(MATRIX *m, FILE *f)
{
  fprintf( f, "\n\n\n Size: %d\n", m->siz+1 );
}



void usizprint(MATRIX *m)
{
  printf( " %d\n", m->siz );
}



void neg_number(MATRIX *m, FILE *f)
{
  fprintf( f, "%d.%d", m->siz+1, pnegno );
}


void ord_number(MATRIX *m, FILE *f)
{
  if ( !m->fragment[NEG] ) fprintf( f, "%d", m->siz+1 );
  else neg_number( m, f );
  fprintf( f, ".%d", pordno );
}


void des_number(MATRIX *m, FILE *f)
{
  ord_number( m, f );
  fprintf( f, ".%d", pdesno );

}


void C_number(MATRIX *m, FILE *f)
{
  des_number( m, f );
  fprintf( f, ".%d", pmatno );
}


void box_number(MATRIX *m, FILE *f)
{
  C_number( m, f );
  fprintf( f, ".%d", pboxno );
}


void u_number(MATRIX *m, int x, FILE *f)
{
  if ( !x ) {
    if ( m->fragment[BOX] ) box_number( m, f );
    else C_number( m, f );
  }
  else u_number( m, x-1, f );
  fprintf( f, ".%d", pmatplus[x] );
}




void nullprint(MATRIX *m, char *s, int x, FILE *f)
{
  fprintf( f, "   %s = %d\n", s, x );
}




void unullprint(MATRIX *m, int x)
{
  printf( "%d\n", x );
}



void monprint(MATRIX *m, char *s, char c, int arr[], FILE *f)
{
  int i, offset;

  fprintf( f, "\n\n     " );
  for ( offset = 0; s[offset]; offset++ ) fprintf( f, " " );
  if ( offset > 1 ) fprintf(f, "  ");
  fprintf( f, "a |" );
  FORALL(m,i) fprintf( f, " %x", i );
  fprintf( f, "\n     " );
  for ( i = 0; i < offset; i++ ) fprintf( f, "-" );
  if ( offset > 1 ) fprintf( f, "--" );
  fprintf( f, "--+" );
  FORALL(m,i) fprintf( f, "--" );
  fprintf( f, "\n     " );
  if ( offset > 1 ) fprintf( f, "%s(a) |", s );
  else fprintf( f, "%sa |", s );
  FORALL(m,i)
    if ( c == '-' ) fprintf( f, " %c", arr[i]? '+': '-' );
    else fprintf( f, " %x", arr[i] );
  fprintf( f, "\n" );
}



void umonprint(MATRIX *m, int arr[])
{
  int i;

  FORALL(m,i)
    printf( " %d", arr[i] );
  printf( "\n" );
}



void dyprint(MATRIX *m, char *s1, char c, int arr[][SZ], 
		char *s2, char d, int darr[][SZ], FILE *f)
{
  int i, j, offset = 0;
  
  for ( i = 0; s1[i]; i++ )
    if ( s1[i] == '\b' ) offset--; else offset++;
  fprintf( f, "\n\n      %s |", s1 );
  FORALL(m,i) fprintf( f, " %x", i );
  if ( *s2 ) {
    fprintf( f, "%15s |", s2 );
    FORALL(m,i) fprintf( f, " %x", i );
  }
  fprintf( f, "\n     " );
  for ( i = 1; i < offset; i++ ) fprintf( f, "-" );
  fprintf( f, "---+" );
  FORALL(m,i) fprintf( f, "--" );
  if ( *s2 ) {
    for ( i = 14; i; i-- )
      fprintf( f, (i > strlen(s2)? " ": "-") ); 
    fprintf( f, "--+" );
    FORALL(m,i) fprintf( f, "--" );
  }
  FORALL(m,i) {
    fprintf( f, "\n      " );
    for ( j = 1; j < offset; j++ ) fprintf( f, " " );
    fprintf( f, "%x |", i );
    FORALL(m,j)
      if ( c == '-' )
	fprintf( f, " %c", arr[i][j]? '+': '-' );
      else fprintf( f, " %x", arr[i][j] );
    if ( *s2 ) {
      fprintf( f, "%15x |", i );
      FORALL(m,j)
	if ( d == '-' )
	  fprintf( f, " %c", darr[i][j]? '+': '-' );
	else fprintf( f, " %x", darr[i][j] );
    }
  }
  fprintf( f, "\n" );
}



void udyprint(MATRIX *m, int arr[][SZ])
{
  int i, j;

  FORALL(m,i) FORALL(m,j)
    printf( " %d", arr[i][j] );
  printf( "\n" );
}



void tex_header()
{
  printf( "\\documentstyle{article}\n");
  printf( "\\oddsidemargin=15mm\n");
  printf( "\\evensidemargin=15mm\n");
  printf( "\\textwidth=160mm\n");
  printf( "\\topmargin=0mm\n");
  printf( "\\textheight=250mm\n");
  printf( "\\begin{document}\n\n");
  printf( "\\pagestyle{empty}\n");
  printf( "\\newcommand{\\C}{\\rightarrow}\n");
  printf( "\\newcommand{\\N}{\\neg}\n");
  printf( "\\newcommand{\\A}{\\mbox{$\\,${\\footnotesize");
  printf( " $\\vee$}$\\,$}}\n");
  printf( "\\newcommand{\\fs}{\\circ}\n");
  printf( "\\newcommand{\\hugeskip}{\\vspace{10mm}}\n");
  printf( "\\setlength{\\tabcolsep}{1.5mm}\n\n");
}



void tex_mystuff(MATRIX *m)
{
  int i;

  printf( "\n%s", m->my_string);
  for ( i = 0; m->my_values[i] != -1; i++ )
    printf( " \\ %d", m->my_values[i]);
}



void texchar(char c)
{
  switch(c) {
  case '&':
  case '%':
  case '$':
  case '#':
  case 'v':
  case '^':
  case '_':
  case '{':
  case '}': printf( "\\%c", c ); return;
  case '~': printf( "$\\N$" ); return;
  case '>': printf( "$\\C$" ); return;
  case '<': printf( "$\\leq$" ); return;
  case 'o': printf( "$\\fs$" ); return;
  case '-': printf( "$-$" ); return;
  case '!': printf( "$\\Box$" ); return;
  case '?': printf( "$\\Diamond$" ); return;
  }
  putchar( c );
}



void texstring(char *s)
{
  do texchar( *s );
  while ( *s++ );
}



void tex_uprint(MATRIX *m, int x)
{
  if ( x < 0 ) {
    if ( m->fragment[BOX] ) tex_boxprint( m );
    else tex_Cprint( m );
  }
  else {
    if ( pmatplus[x] == 1 || singlematrix )
      tex_uprint( m, x-1 );
    printf(
	   "\n\n\\hugeskip\\noindent\\parbox[t]{160mm}{");
    texstring( m->dcs[x] );
    printf( " table " );
    u_number( m, x, stdout );
    switch( m->adicity[x] ) {
    case 0: 
      tex_nullprint( m, m->dcs[x], m->nulladic[x] );
      break;
    case 1:
      tex_monprint( m, m->dcs[x], ' ', m->monadic[x] );
      break;
    case 2:
      tex_dyprint( m, m->dcs[x], 0, m->dyadic[x], "", 0, 0 );
    }
    printf( "}" );  
  }
}




void tex_boxprint(MATRIX *m)
{
  if ( pboxno == 1 || singlematrix ) tex_Cprint( m );
  printf( 
	 "\n\n\\hugeskip\\noindent\\parbox[t]{160mm}{Necessity " );
  box_number( m, stdout );
  tex_monprint( m, "!", ' ', m->box );
  printf( "}" );
}



void tex_Cprint(MATRIX *m)
{
  if ( pmatno == 1 || singlematrix ) tex_desprint( m );
  printf( 
	 "\n\n\\hugeskip\\noindent\\parbox[t]{160mm}{Implication " );
  C_number( m, stdout );
  if ( m->fused )
    tex_dyprint( m, ">", 0, m->C, "o", 0, m->fus );
  else
    tex_dyprint( m, ">", 0, m->C, "", 0, 0 );
  printf( "}" );
}



void tex_desprint(MATRIX *m)
{
  if ( pdesno == 1 || singlematrix ) tex_ordprint( m );
  printf("\n\n\\hugeskip\\noindent\\parbox[t]{160mm}{Choice of truths " );
  des_number( m, stdout );
  tex_monprint( m, "True", '-', m->designated );
  printf( "}" );
}



void tex_ordprint(MATRIX *m)
{
  if ( pordno == 1 || singlematrix ) {
    if ( m->fragment[NEG] ) tex_negprint( m );
    else tex_sizprint( m );
  }
  printf("\n\n\\hugeskip\\noindent\\parbox[t]{160mm}{Order table " );
  ord_number( m, stdout );
  tex_dyprint( m, "<", '-', m->ord, "", 0, 0 );
  printf( "}" );
}



void tex_negprint(MATRIX *m)
{
  if ( pnegno == 1 || singlematrix ) tex_sizprint( m );
  printf("\n\n\\hugeskip\\noindent\\parbox[t]{160mm}{Negation table " );
  neg_number( m, stdout );
  tex_monprint( m, "~", ' ', m->neg );
  printf( "}" );
}



void tex_sizprint(MATRIX *m)
{
  if ( m->total_put > 1 ) printf( "\n\n\\newpage");
  printf( "\\noindent Size: %d", m->siz+1 );
}



void tex_nullprint(MATRIX *m, char *s, int x)
{
  printf( " \\ ");
  texstring( s );
  printf( " = %d", x );
}




void tex_monprint(MATRIX *m, char *s, char c, int arr[])
{
  int i;

  printf("\\\\[3mm]\n\\hspace*{5mm}\\begin{tabular}{r|" );
  FORALL(m,i) putchar( 'c' );
  printf( "}\na &");
  FORALL(m,i) {
    printf( " %x ", i );
    if ( i < m->siz ) putchar( '&' );
  }
  printf( "\\\\\\hline\n");
  texstring( s );
  printf("(a)");
  printf( " & ");
  FORALL(m,i) {
    if ( c == '-' )
      texchar( arr[i]? '+': '-' );
    else printf( " %x ", arr[i] );
    if ( i < m->siz ) putchar( '&' );
    else printf( "\\\\\n");
  }
  printf( "\\end{tabular}");
}




void tex_dyprint(MATRIX *m, char *s1, char c, int arr[][SZ],
			char *s2, char d, int darr[][SZ])
{
  printf( "\\\\[3mm]\n" );
  tex_dymatprint( m, 5, s1, c, arr );
  if ( *s2 ) tex_dymatprint( m, 10, s2, d, darr);
}




void tex_dymatprint(MATRIX *m, int offset, char *s, char c, int arr[][SZ])
{
  int i, j;

  printf( "\\hspace*{%dmm}\\begin{tabular}{r|", offset );
  FORALL(m,i) putchar( 'c' );
  printf( "}\n");
  texstring( s );
  printf( " &");
  FORALL(m,i) {
    printf( " %x ", i );
		if ( i < m->siz ) putchar( '&' );
  }
  printf( "\\\\\\hline\n");
  
  FORALL(m,i) {
    printf( "%x &", i );
    FORALL(m,j) {
      if ( c == '-' )
	texchar( arr[i][j]? '+': '-' );
      else printf( " %x ", arr[i][j] );
      if ( j < m->siz ) putchar( '&' );
      else printf( "\\\\\n");
    }
  }
  printf( "\\end{tabular}");
}



void print_related(MATRIX *m1, MATRIX *m2, char *relname)
{
  printf("%s m%d m%d\n", relname, m1->total_got, m2->total_got);
}
