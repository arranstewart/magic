/*			Header for Read_Matrices
*
*	Selectmats reads in matrix representations of algebraic
*	models for propositional logics and prints out each one
*	that satisfies a condition.  It is called with three
*	parameters, being a pointer to a MATRIX, the name of the 
*	user-defined selection function and the print mode 
*	(PRETTY, UGLY or TeX).
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



/*
*  The setup is read into MATRIX fields as follows:
*
*    siz    the greatest-numbered value.  So the
*           values are the integers 0...siz.
*
*    neg    the negation table (if present).
*
*    ord    the partial order of implication.
*
*    A, K    disjunction and conjunction tables.
*
*    designated  designation and undesignation of values.
*
*    tee    the least designated value (if defined).
*
*    eff    the negation of tee (if defined).
*
*    C      the implication table.
*
*    box    the necessity table.
*
*    fus    the fusion table (if defined).
*
*    fis    the fission table (if defined).
*
*    dcs    the symbols for defined connectives.
*
*    adicity     of the defined connectives
*
*    nulladic
*    monadic
*    dyadic      matrices for the user's connectives.
*
*    fused       boolean flag: fusion is defined.
*
*    tee_exists  boolean flag: tee is defined.
*
*    negno
*    ordno
*    desno
*    matno       Offset numbers of the current setup.
*
*    oknegno
*    okordno
*    okdesno
*    okmatno     Offset numbers of the latest good setup.
*
*    fragment    connectives defined
*
*
*  In addition, the constant SZ is defined as the greatest 
*  possible number of values.  That is, siz < SZ always.
*
*  The universal quantifier FORALL is defined for use in the
*  host program if desired.  FORALLCON is also provided to run 
*  through the user-defined primitives in their change order.
*
*  The data type WFF is exactly as in MaGIC, for input and
*  testing of formulas.  The parser may be linked in.
*
*  The array my_values and the string my_string are provided
*  for communication of the results of tests etc to the 
*  printout routine.  They are initialised to a series of 
*  -1s and to the null string respectively.  These values 
*  should be used for the null communication.
*
*  Several other MATRIX fields are used to control the 
*  environment, mostly to determine printing options.
*
*    cmax          number of user-added primitives
*    total_got     number of matrices read in
*    total_put     number which have passes the test
*    singlematrix  treat as though the only matrix
*
*  Those are all integers.  There are a couple of defined
*  enumeration types, PRINTMODE and PRINTCOUNTER.  These are 
*  the types of the parameters printmode and printcountermode
*  passed to printmat.  The former of these determines whether
*  output is PRETTY, UGLY or TeX, while the latter determines
*  whether the numbering of matrices is to be incremental 
*  within the output only or whether the numbers remain as on
*  input.
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SZ 16
#define FMAX 256
#define CMAX 32
#define VALUEMAX 10

#define FORALL(m,x) for ( x = 0; x <= m->siz; x++ )
#define FOREACH(m,x) for ( x = m->siz; x >= 0; x-- )
#define FORALLCON(m,x) for ( x = 0; x < m->cmax; x++)

typedef enum { NONE, PRETTY, UGLY, TeX } PRINTMODE;
typedef enum { SELECTED, UNSELECTED } COUNTERMODE;
enum { NEG, TEE, TOP, BOT, FUS, LAT, BOX, FRAGMAX } ;

typedef struct well_formed_formula {
  char *sym;      /* The main symbol              */
  int lsub;
  int rsub;       /* Offsets of the subformulas   */
  int *mtx;       /* Start of the relevant matrix */
  int *lv;
  int *rv;        /* To values of subformulas     */
  int val;        /* Currently assigned value     */
} WFF;


/*
* Now for the big one: this is the matrix structure.
*/

typedef struct {
  WFF form[FMAX];
  WFF *tx;

  int siz;
  int neg[SZ];
  int ord[SZ][SZ];
  int designated[SZ];
  int tee;
  int eff;
  int A[SZ][SZ];
  int K[SZ][SZ];
  int C[SZ][SZ];
  int fus[SZ][SZ];
  int fis[SZ][SZ];
  int box[SZ];
  int diamond[SZ];
  int adicity[CMAX];
  int nulladic[CMAX];
  int monadic[CMAX][SZ];
  int dyadic[CMAX][SZ][SZ];
  int negno;
  int oknegno;
  int ordno;
  int okordno;
  int desno; 
  int okdesno;
  int matno;
  int okmatno;
  int boxno;
  int okboxno;
  int matplus[CMAX];
  int okmatplus[CMAX];
  int fused;
  int tee_exists;
  int cmax;
  int total_got;
  int total_put;
  int fragment[FRAGMAX];
  int my_values[VALUEMAX];

  char my_string[256];
  char dcs[CMAX][16];
  char symbols[3][15];
} MATRIX;



FILE *fopen();

/*
* Finally, here are the function prototypes for RM.c
*/

void selectmats(int (*selector)(), PRINTMODE printmode);
void relatemats(int (*selector)(), char *filename, char *relname);
int True(MATRIX *m);
void increment_ok(MATRIX *m);
MATRIX* mat_initial();
void Abort(char *s, int x);

/*
* These are the prototypes for RM_input.c
*/
void read_header(FILE *f, MATRIX *m);
int newsiz(FILE *f, MATRIX *m);
int newneg(FILE *f, MATRIX *m);
int neword(FILE *f, MATRIX *m);
int newdes(FILE *f, MATRIX *m);
int newC(FILE *f, MATRIX *m);
int newbox(FILE *f, MATRIX *m);
int newcon(FILE *f, MATRIX *m, int x);
int newcase(FILE *f, MATRIX *m);
int gotsiz(FILE *f, MATRIX *m);
int gotneg(FILE *f, MATRIX *m);
int gotord(FILE *f, MATRIX *m);
int gotdes(FILE *f, MATRIX *m);
int gotaro(FILE *f, MATRIX *m);
int gotbox(FILE *f, MATRIX *m);
int gotcon(FILE *f, MATRIX *m, int x);


/*
* These are the prototypes for RM_output.c
*/

void displaymat(MATRIX *m, char *legend);
void printmat(MATRIX *m, PRINTMODE printmode,
	      COUNTERMODE countermode, FILE *f);
void write_header(MATRIX *m);
void pu_print(MATRIX *m, int x, FILE *f);
void uu_print(MATRIX *m, int x);
void pbox_print( MATRIX *m, FILE *f );
void ubox_print(MATRIX *m);
void pC_print(MATRIX *m, FILE *f);
void uC_print(MATRIX *m);
void pdesprint(MATRIX *m, FILE *f);
void udesprint(MATRIX *m);
void pordprint(MATRIX *m, FILE *f);
void uordprint(MATRIX *m);
void pnegprint(MATRIX *m, FILE *f);
void unegprint(MATRIX *m);
void psizprint(MATRIX *m, FILE *f);
void usizprint(MATRIX *m);
void neg_number(MATRIX *m, FILE *f);
void ord_number(MATRIX *m, FILE *f);
void des_number(MATRIX *m, FILE *f);
void C_number(MATRIX *m, FILE *f);
void box_number(MATRIX *m, FILE *f);
void u_number(MATRIX *m, int x, FILE *f);
void nullprint(MATRIX *m, char *s, int x, FILE *f);
void unullprint(MATRIX *m, int x);
void monprint(MATRIX *m, char *s, char c, int arr[], FILE *f);
void umonprint(MATRIX *m, int arr[]);
void dyprint(MATRIX *m, char *s1, char c, int arr[][SZ],
	     char *s2, char d, int darr[][SZ], FILE *f);
void udyprint(MATRIX *m, int arr[][SZ]);
void tex_header();
void tex_mystuff(MATRIX *m);
void texchar(char c);
void tex_uprint(MATRIX *m, int x);
void tex_boxprint(MATRIX *m);
void tex_Cprint(MATRIX *m);
void tex_desprint(MATRIX *m);
void tex_ordprint(MATRIX *m);
void tex_negprint(MATRIX *m);
void tex_sizprint(MATRIX *m);
void tex_nullprint(MATRIX *m, char *s, int x);
void tex_monprint(MATRIX *m, char *s, char c, int arr[]);
void tex_dyprint(MATRIX *m, char *s1, char c, int arr[][SZ],
		 char *s2, char d, int darr[][SZ]);
void tex_dymatprint(MATRIX *m, int offset, char *s, char c, int arr[][SZ]);
void print_related(MATRIX *m1, MATRIX *m2, char *relname);
