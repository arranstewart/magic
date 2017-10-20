/*
*			Mtypes.h
*
*	This file is part of the header for MaGIC. It contains
*	the type definitions. The special header axioms.h has to
*	be included from here, as it uses the enumerated type 
*	boolean and defines such constants as AXMAX which are in
*	turn used here in the definitions of the big structures.
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
*	First the output codes.  This is just an enumeration.
*/

typedef enum
{
	NONE,
	PRETTY,
	UGLY,
	SUMMARY
} output_style;


/*
*	There is another enumeration of cases for formula input
*/

typedef enum
{
	IN_CONC,
	IN_PREM,
	IN_BGUY,
	IN_DEFN
} input_case;


/*
*	And one for formula output
*/

typedef enum
{
	VARS,
	VALS
} varmode;


/*
*	Then the various help codes
*/

typedef enum
{
	MEN,
	WFF1,
	WFF2,
	FDL,
	BTW,
	LOG,
	OUT,
	HELPMAX
} helpcode;


/*
*	It remains to define the structure types used.  Each 
*	"struct xxxtype" expression is abbreviated using typedef
*	to preserve sanity later on.  JOB is for communication of 
*	the major problem specification between the front end and 
*	MaGIC itself.  The others are fairly self-explanatory.
*/


typedef struct isomorph
{	char	*icv;		/* Isomorphic version of info	*/
	struct isomorph
		*left,
		*right,
		*parent;	/* Links to make a binary tree	*/
} ISM, *ism;


typedef struct symbol_list
{
	char	s[SLEN];	/* Connective as a string	*/
	struct symbol_list
		*next, *last;	/* Links for list		*/
} SYMB, *symb;


typedef struct
{	symb	sym;		/* The main symbol		*/
	int	lsub,
		rsub,		/* Offsets of the subformulas	*/
		*mtx,		/* Start of the relevant matrix	*/
		*lv,
		*rv,		/* To values of subformulas	*/ 
		val;		/* Currently assigned value	*/
} WFF;


typedef struct PerMutAtion
{
	char	h[SZ];		/* Image under homomorphism	*/
	struct PerMutAtion
		*pup;		/* Pointer to the next guy up	*/
} PRM;


typedef enum {
	lattices,
	distributive_lattices,
	total_orders
} L_DEFAULT;


typedef enum {
	n_exists,
	lat_exists,
	fus_exists,
	nec_exists,
	PCMAX	
} XF;


typedef struct
{
	int	adicity[CMAX],		/* Of defined connectives	*/
		croot[TMAX][RTMAX],	/* Roots of rule conclusions	*/
		proot[TMAX][RTMAX],	/* Roots of rule premises	*/
		defcon[CMAX],		/* Roots of defined connectives	*/
		failure,		/* Root of badguy		*/
		maxtime,		/* Maximum clock reading	*/
		maxmat,			/* Maximum good matrices	*/
		sizmax;			/* Maximum matrix size		*/
	output_style
		tty_out,
		fil_out;		/* Output formats		*/
	char	data_dir[SLEN],		/* Text name of input directory	*/
		outfil_name[SLEN];	/* Text name of output file	*/
	WFF	form[FMAX];		/* (Subformulas of) axioms etc	*/

	SYMB	symtable[SYMBOLMAX];	/* Symbol table for connectives	*/
	symb	symtab,			/* List of symbols used		*/
		symbol[3][CMAX+8],	/* Symbol table entries		*/
		dcs[CMAX+1];		/* Defined connectives		*/
	boolean	axiom[AXMAX],	/* Selected axioms (default none)	*/
		concut[CMAX],	/* Flags connectives with "cuts"	*/
		f_n, f_lat,
		f_t, f_T,
		f_F,		/* The fragment (default 1)		*/
		f_fus, f_nec,	/* The fragment (default 0)		*/
		sizmax_ismax,	/* (Boolean) Let sizmax float up	*/
		distrib,	/* Distributive lattice requested	*/
		totord;		/* Total orders requested		*/
	LOGIC	logic;			/* The system (default FD)	*/
} JOB;
