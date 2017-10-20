/*
*				Mglob.h
*
*	This file contains the declarations of global variables for
*	MaGIC. Note that TR.h contains its own globals.
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



GLOBAL JOB *theJob;		/* Run specification		*/

GLOBAL TRIN tr_par;		/* vntr job specification	*/

GLOBAL FILE
	*outfil,
	*fopen();

GLOBAL int
	nulladic[CMAX],		/* Defined sential constants	*/
	monadic[CMAX][SZ],	/* Defined connectives		*/
	dyadic[CMAX][SZ][SZ],	/* Ditto			*/
	vu[VMAX],
	rvu[VMAX],		/* Variables used in wffs	*/
	badvalue[VMAX],		/* Refuting assignment		*/
	kost[TMAX],		/* Matrix lookups for axioms	*/
	siz,			/* Highest value		*/
	infil,			/* File descriptor for input	*/
	input_bit,		/* Offset of next bit in input	*/
	start_time,		/* Clock reading    		*/
	stop_time,		/* Clock reading    		*/
	tot,			/* Total matrices tested	*/
	isoms,	isoms2,		/* Isomorphs omitted		*/
	begin_timer,
	end_timer,		/* For timing of serial version	*/
	Sizmax,			/* Local version of sizmax	*/
	Vmax,			/* Local version of VMAX	*/
	zero;			/* Dummy integer		*/

GLOBAL boolean
	xdialog,		/* Running from xmagic		*/
	noclear,		/* No system calls  		*/
	filing,			/* Output file open		*/
	default_fragment[LOGMAX][PCMAX];	/* Connectives	*/

GLOBAL char
	buffa,			/* 8 bits of input data		*/
	answer[SLEN],		/* For input of axioms etc	*/
	thisvector[V_LENGTH],	/* Current matrix stretched out	*/
	thatvector[V_LENGTH],	/* Last good matrix ditto	*/
	isolist[ISLMAX];	/* Blanks for isomorphisms	*/

GLOBAL WFF
	*tx;			/* Last active subformula	*/

GLOBAL ISM istak[ISOMAX];	/* Spare records for morphisms	*/

GLOBAL PRM *perm;		/* List of permutations		*/

GLOBAL int
	C[SZ][SZ],		/* The implication matrix	*/
	ord[SZ][SZ],		/* The partial order table	*/
	K[SZ][SZ],		/* The conjunction matrix	*/
	A[SZ][SZ],		/* The disjunction matrix	*/
	fus[SZ][SZ],		/* The fusion matrix		*/
	N[SZ],			/* The negation matrix		*/
	box[SZ],		/* The necessity matrix		*/
	diamond[SZ],		/* The possibility matrix	*/
	desig[SZ],		/* Designated values		*/
	maximal[SZ],		/* Maximal guys under ord	*/
	atom[2][CMAX],		/* 'a' and 'b' for definitions	*/
	impindex[SZ][SZ],	/* Info cell for C[x][y]	*/
	boxindex[SZ],		/* Info cell for box[x]		*/
	ucc0[CMAX],		/* Info cell for nulladic[x]	*/
	ucc1[CMAX][SZ],		/* Info cell for monadic[x][y]	*/
	ucc2[CMAX][SZ][SZ];	/* And dyadic[x][y][z]		*/

GLOBAL int
	good,		/* Matrices accepted		*/
	negno,		/* Negation #, this size	*/
	ordno,		/* Order #, this negation	*/
	desno,		/* Des #, this order		*/
	matno,		/* Matrix #, this des		*/
	boxno,		/* Box #, this matrix		*/
	matplus[CMAX],	/* Matno for user's connectives	*/
	matsum,		/* Matrices this des choice	*/
	des,		/* Least designated value	*/
	undes,		/* Greatest undesignated value	*/
	firstchange,	/* First cell changed this test	*/
	firstarrow,	/* First cell of -> matrix	*/
	firstbox,	/* First cell of ! matrix	*/
	got_a_fail,	/* Badguy fails in matrix	*/
	F_N,		/* f_n and contraposition	*/
	afx;		/* Affixing is in force		*/

GLOBAL unsigned
	greater_than[SZ],	/* {y: ord[x][y]}	*/
	Greater_than[SZ],	/* {y: ord[x][y],x!=y}	*/
	less_than[SZ],		/* {y: ord[y][x]}	*/
	Less_than[SZ];		/* less_than[x] \ x	*/

GLOBAL L_DEFAULT
	default_orders[LOGMAX];	/* Defaults for logics	*/
