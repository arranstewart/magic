/*
*			axioms.c
*
*	This file contains the code specific to the pre-defined 
*	axioms and logics. To add or remove an axiom, or to add,
*	remove or alter the definition of a logic, various changes 
*	must be made in this file and in axioms.h, as described in
*	the documentation.
*/

#include "MaGIC.h"

/*
*	Strings_initial simply sets the strings containing the names
*	of known logics and the surface forms of known axioms
*/

void strings_initial()
{
	strcpy(logic_name[Null_logic], "");
	strcpy(logic_name[FD], "FD");
	strcpy(logic_name[B], "B");
	strcpy(logic_name[DW], "DW");
	strcpy(logic_name[TW], "TW");
	strcpy(logic_name[EW], "EW");
	strcpy(logic_name[RW], "C");
	strcpy(logic_name[LIN], "LIN");
	strcpy(logic_name[T], "T");
	strcpy(logic_name[E], "E");
	strcpy(logic_name[R], "R");
	strcpy(logic_name[CK], "CK");
	strcpy(logic_name[S4], "S4");

	strcpy(ax_string[RulPref], "p -> q / (r -> p) -> (r -> q)");
	strcpy(ax_string[RulSuff], "p -> q / (q -> r) -> (p -> r)");
	strcpy(ax_string[AxKcomp], "((p -> q) & (p -> r)) -> (p -> (q & r))");
	strcpy(ax_string[AxAcomp], "((p -> r) & (q -> r)) -> ((p v q) -> r)");
	strcpy(ax_string[AxX], "p v ~p");
	strcpy(ax_string[AxBA], "(p & ~p) -> q");
	strcpy(ax_string[AxSBA], "(p & ~p) -> (q v ~q)");
	strcpy(ax_string[AxW2], "(p & (p -> q)) -> q");
	strcpy(ax_string[AxK], "p -> (q -> p)");
	strcpy(ax_string[AxK2], "p / q -> p");
	strcpy(ax_string[AxM], "p -> (p -> p)");
	strcpy(ax_string[AxRED], "(p -> ~p) -> ~p");
	strcpy(ax_string[RulC3], "p / (p -> q) -> q");
	strcpy(ax_string[AxCt], "p -> (t -> p)");
	strcpy(ax_string[Axat], "p v (p -> q)");
	strcpy(ax_string[AxTF], "T -> (F -> F)");
	strcpy(ax_string[AxC2], "p -> ((p -> q) -> q)");
	strcpy(ax_string[AxFN], "(p -> ~q) -> (q -> ~p)");
	strcpy(ax_string[AxW], "(p -> (p -> q)) -> (p -> q)");
	strcpy(ax_string[AxB], "(q -> r) -> ((p -> q) -> (p -> r))");
	strcpy(ax_string[AxB2], "(p -> q) -> ((q -> r) -> (p -> r))");
	strcpy(ax_string[AxS], "(p -> (q -> r)) -> ((p -> q) -> (p -> r))");
	strcpy(ax_string[AxC], "(p -> (q -> r)) -> (q -> (p -> r))");
	strcpy(ax_string[AxWB], "((p -> q) & (q -> r)) -> (p -> r)");
	strcpy(ax_string[RulNec], "p / !p");
	strcpy(ax_string[AxNec], "!p -> p");
	strcpy(ax_string[Ax4], "!p -> !!p");
	strcpy(ax_string[AxNID], "!(p -> q) -> (!p -> !q)");
	strcpy(ax_string[AxNand], "(!p & !q) -> !(p & q)");
	strcpy(ax_string[AxNW], "(!p -> (!p -> q)) -> (!p -> q)");
	strcpy(ax_string[AxNK], "p -> (!q -> p)");
	strcpy(ax_string[AxNKI], "p / !q -> p");
	strcpy(ax_string[Ax5], "p -> !?p");
	strcpy(ax_string[AxD], "!p -> ?p");
}




/*
*	Tests_initial sets the list of axiom test functions.
*	The zero_tests are executed when the extensional
*	setup has been read, before any call to transref.
*	The one_tests are executed when impossible values are 
*	being removed from the search space inside transref.
*	The two_tests are called by transref to pre-process 
*	two-refutations in order to avoid having to generate 
*	before testing them. The many_tests are the default 
*	test routines called when the candidate matrix has 
*	been generated. All tests are null by default
*/

void tests_initial()
{
	AXIOM a;

	for ( a = AxNull; a < AXMAX; a++ ) {
	  TL[a].zero_test = TL[a].many_test = 0;
	  TL[a].one_test = TL[a].two_test = TL[a].three_test = 0;
	}

	TL[AxX].zero_test = Exmid;
	TL[AxBA].zero_test = Boolalg;
	TL[AxSBA].zero_test = SemiBool;
	TL[AxK2].zero_test = paradox;

	TL[RulPref].one_test = set_prefix;
	TL[RulSuff].one_test = set_suffix;
	TL[AxW2].one_test = Wstar;
	TL[AxK].one_test = PARADOX;
	TL[AxM].one_test = mingle;
	TL[AxRED].one_test = Reductio;
	TL[RulC3].one_test = E_assertion;
	TL[AxCt].one_test = ata;
	TL[Axat].one_test = t_atomic;
	TL[AxTF].one_test = FF_T;
	TL[RulNec].one_test = necessitation;
	TL[AxNec].one_test = necessity;
	TL[AxNKI].one_test = NKI_test;

	TL[RulPref].two_test = pretest_prefix;
	TL[RulSuff].two_test = pretest_suffix;
	TL[Ax4].two_test = test_S4axiom;
	TL[Ax5].two_test = test_S5axiom;
	TL[AxD].two_test = test_Daxiom;
	TL[AxC2].two_test = test_assertion;
	TL[AxW].two_test = test_contraction;
	TL[AxNK].two_test = test_NK;

	TL[AxKcomp].many_test = test_mslat;
	TL[AxAcomp].many_test = test_jslat;
	TL[AxNID].many_test = NecImpDist;
	TL[AxNand].many_test = NecAdj;
	TL[AxB].many_test = Btest;
	TL[AxB2].many_test = B2test;
	TL[AxS].many_test = Stest;
	TL[AxC].many_test = Ctest;
	TL[AxWB].many_test = WBtest;
	TL[AxNW].many_test = NecW;
}


/*
*	Logics_initial sets the arrays showing which axioms are valid in
*	which logics. Note that each logic has its own subroutine, except 
*	for the base logics FD and B, which have no proper axioms.
*/

void logics_initial()
{
	LOGIC l;
	AXIOM a;

	for ( l = Null_logic; l < LOGMAX; l++ ) {
		for ( a = AxNull; a < AXMAX; a++ )
		valid[l][a] = false;
		default_orders[l] = distributive_lattices;
		default_fragment[l][n_exists] = true;
		default_fragment[l][lat_exists] = true;
		default_fragment[l][fus_exists] = false;
		default_fragment[l][nec_exists] = false;
	}

	init_B();
	init_DW();
	init_TW();
	init_EW();
	init_RW();
	init_LIN();
	init_CK();
	init_T();
	init_E();
	init_R();
	init_S4();
}



void init_B()
{
	valid[B][RulPref] = true;
	valid[B][RulSuff] = true;
	valid[B][AxKcomp] = true;
	valid[B][AxAcomp] = true;
}



void init_DW()
{
	valid[DW][AxFN] = true;
	valid[DW][RulPref] = true;
	valid[DW][RulSuff] = true;
	valid[DW][AxKcomp] = true;
	valid[DW][AxAcomp] = true;
}


void init_TW()
{
	valid[TW][AxFN] = true;
	valid[TW][AxB] = true;
	valid[TW][AxB2] = true;
	valid[TW][RulPref] = true;
	valid[TW][RulSuff] = true;
	valid[TW][AxKcomp] = true;
	valid[TW][AxAcomp] = true;
}


void init_EW()
{
	valid[EW][AxFN] = true;
	valid[EW][AxB] = true;
	valid[EW][AxB2] = true;
	valid[EW][RulC3] = true;
	valid[EW][RulPref] = true;
	valid[EW][RulSuff] = true;
	valid[EW][AxKcomp] = true;
	valid[EW][AxAcomp] = true;
}


void init_RW()
{
	valid[RW][AxFN] = true;
	valid[RW][AxB] = true;
	valid[RW][AxB2] = true;
	valid[RW][RulC3] = true;
	valid[RW][AxCt] = true;
	valid[RW][AxTF] = true;
	valid[RW][AxC2] = true;
	valid[RW][AxC] = true;
	valid[RW][RulPref] = true;
	valid[RW][RulSuff] = true;
	valid[RW][AxKcomp] = true;
	valid[RW][AxAcomp] = true;
}


void init_LIN()
{
	valid[LIN][AxFN] = true;
	valid[LIN][AxB] = true;
	valid[LIN][AxB2] = true;
	valid[LIN][RulC3] = true;
	valid[LIN][AxCt] = true;
	valid[LIN][AxTF] = true;
	valid[LIN][AxC2] = true;
	valid[LIN][AxC] = true;
	valid[LIN][RulPref] = true;
	valid[LIN][RulSuff] = true;
	valid[LIN][AxKcomp] = true;
	valid[LIN][AxAcomp] = true;
	valid[LIN][RulNec] = true;
	valid[LIN][AxNec] = true;
	valid[LIN][Ax4] = true;
	valid[LIN][AxD] = true;
	valid[LIN][AxNID] = true;
	valid[LIN][AxNW] = true;
	valid[LIN][AxNK] = true;
	valid[LIN][AxNKI] = true;

	default_orders[LIN] = lattices;
	default_fragment[LIN][nec_exists] = true;
}


void init_T()
{
	valid[T][AxFN] = true;
	valid[T][AxB] = true;
	valid[T][AxB2] = true;
	valid[T][AxS] = true;
	valid[T][AxX] = true;
	valid[T][AxW] = true;
	valid[T][AxW2] = true;
	valid[T][AxWB] = true;
	valid[T][AxRED] = true;
	valid[T][RulPref] = true;
	valid[T][RulSuff] = true;
	valid[T][AxKcomp] = true;
	valid[T][AxAcomp] = true;
	valid[T][AxNW] = true;
}


void init_E()
{
	valid[E][AxFN] = true;
	valid[E][AxB] = true;
	valid[E][AxB2] = true;
	valid[T][AxS] = true;
	valid[E][RulC3] = true;
	valid[E][AxX] = true;
	valid[E][AxW] = true;
	valid[E][AxW2] = true;
	valid[E][AxWB] = true;
	valid[E][AxRED] = true;
	valid[E][RulPref] = true;
	valid[E][RulSuff] = true;
	valid[E][AxKcomp] = true;
	valid[E][AxAcomp] = true;
	valid[E][AxNW] = true;
}


void init_R()
{
	valid[R][AxFN] = true;
	valid[R][AxB] = true;
	valid[R][AxB2] = true;
	valid[T][AxS] = true;
	valid[R][RulC3] = true;
	valid[R][AxCt] = true;
	valid[R][AxTF] = true;
	valid[R][AxC2] = true;
	valid[R][AxC] = true;
	valid[R][AxX] = true;
	valid[R][AxW] = true;
	valid[R][AxW2] = true;
	valid[R][AxWB] = true;
	valid[R][AxRED] = true;
	valid[R][RulPref] = true;
	valid[R][RulSuff] = true;
	valid[R][AxKcomp] = true;
	valid[R][AxAcomp] = true;
	valid[R][AxNW] = true;
}


void init_CK()
{
	valid[CK][AxFN] = true;
	valid[CK][AxB] = true;
	valid[CK][AxB2] = true;
	valid[CK][RulC3] = true;
	valid[CK][AxCt] = true;
	valid[CK][AxTF] = true;
	valid[CK][AxC2] = true;
	valid[CK][AxC] = true;
	valid[CK][AxK] = true;
	valid[CK][AxK2] = true;
	valid[CK][AxM] = true;
	valid[CK][RulPref] = true;
	valid[CK][RulSuff] = true;
	valid[CK][AxKcomp] = true;
	valid[CK][AxAcomp] = true;
	valid[CK][AxNKI] = true;
	valid[CK][AxNK] = true;
}


void init_S4()
{
	valid[S4][AxFN] = true;
	valid[S4][AxB] = true;
	valid[S4][AxB2] = true;
	valid[T][AxS] = true;
	valid[S4][RulC3] = true;
	valid[S4][AxX] = true;
	valid[S4][AxW] = true;
	valid[S4][AxW2] = true;
	valid[S4][AxWB] = true;
	valid[S4][AxRED] = true;
	valid[S4][AxBA] = true;
	valid[S4][AxSBA] = true;
	valid[S4][AxK2] = true;
	valid[S4][AxM] = true;
	valid[S4][RulPref] = true;
	valid[S4][RulSuff] = true;
	valid[S4][AxKcomp] = true;
	valid[S4][AxAcomp] = true;
	valid[CK][AxNKI] = true;
}
