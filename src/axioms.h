/*
*				axioms.h		May 1993
*
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
*	First define the logics and axioms
*/

typedef enum {
	Null_logic,
	FD,
	B,
	DW,
	TW,
	EW,
	RW,
	LIN,
	T,
	E,
	R,
	CK,
	S4,
	LOGMAX
} LOGIC;



typedef enum {
	AxNull,
	RulPref,
	RulSuff,
	AxKcomp,
	AxAcomp,
	AxX,
	AxBA,
	AxSBA,
	AxW2,
	AxK,
	AxK2,
	AxM,
	AxRED,
	RulC3,
	AxCt,
	Axat,
	AxTF,
	AxB,
	AxB2,
	AxS,
	AxC2,
	AxW,
	AxC,
	AxWB,
	AxFN,
	RulNec,
	AxNec,
	AxD,
	Ax4,
	Ax5,
	AxNID,
	AxNand,
	AxNW,
	AxNK,
	AxNKI,
	AXMAX
} AXIOM;


/*
*	There now follow the maximum sizes for the various
*	logics and fragments.
*/

typedef struct {
	boolean (*zero_test)();
	void (*one_test)();
	void (*two_test)();
	void (*three_test)();
	boolean (*many_test)();
} testlist;



GLOBAL char
	logic_name[LOGMAX][8],	/* Text names of known logics	*/
	ax_string[AXMAX][64];	/* Text forms of known axioms	*/

GLOBAL boolean
	taxiom[AXMAX],		/* Tested axioms		*/
	valid[LOGMAX][AXMAX];	/* Axioms valid in each logic	*/

GLOBAL testlist
	TL[AXMAX];		/* Test functions for axioms	*/
