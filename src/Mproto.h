/*
*			Mproto.h (included in MaGIC.h)
*
*	This file contains the function prototypes for MaGIC. Note that
*	the prototypes for vntr.c are in the file vntr.h (also included 
*	in MaGIC.h).
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
*	In file MaCIC.c
*/

int main();
void timesup();
void intrupt();


/*
*	In file axioms.c
*/

void strings_initial();
void tests_initial();
void logics_initial();
void init_B();
void init_DW();
void init_TW();
void init_EW();
void init_RW();
void init_LIN();
void init_CK();
void init_T();
void init_E();
void init_R();
void init_S4();


/*
*	In file axiom_tests.c
*/

boolean Exmid();
boolean Boolalg();
boolean SemiBool();
boolean paradox();
boolean f_arrow_t();

void E_assertion(unsigned info[]);
void contraction(unsigned info[]);
void Wstar(unsigned info[]);
void Reductio(unsigned info[]);
void assertion(unsigned info[]);
void ata(unsigned info[]);
void TaT(unsigned info[]);
void mingle(unsigned info[]);
void t_atomic(unsigned info[]);
void FF_T(unsigned info[]);
void PARADOX(unsigned info[]);
void RWX(unsigned info[]);
void necessity(unsigned info[]);
void necessitation(unsigned info[]);
void NKI_test(unsigned info[]);
void set_prefix(unsigned info[]);
void set_suffix(unsigned info[]);
void squeeze(unsigned info[], int a, int b, int c, int d);

void pretest_prefix();
void pretest_suffix();
void test_S4axiom();
void test_S5axiom();
void test_Daxiom();
void test_NK();

boolean test_mslat(trs T);
boolean test_jslat(trs T);
boolean Btest(TRS *T);
boolean B2test(TRS *T);
boolean Stest(TRS *T);
boolean Ctest(TRS *T);
boolean WBtest(TRS *T);
boolean NecImpDist(TRS *T);
boolean NecAdj(TRS *T);
boolean NecW(TRS *T);


/*
*	In file dialog.c
*/

int dialog(boolean batch, char *batchfile);
char menu();
char readin(char *str);
void paws();
void job_defaults(boolean batch);
void set_frag( boolean set_sizmax );
void check_frag(int x);
void print_axiom(FILE *f, AXIOM x);
void display();
void disp(FILE *f);
void help(helpcode x);
void put_out(char *f_nm);


/*
*	In file getjob.c
*/

void add_axioms();
void add_one_axiom( AXIOM select );
void bad_guy();
void connective();
void deletion();
void fragment();
void input_direct();
void jump_condition();
void logic_choice();
void set_logic( LOGIC lptr );
void print_version();
int n_of_procs();
void order_change();
void user_order(symb oldsymbols[], char *string, int neworder[]);
void print_options();
boolean read_job(char *batchfile);
void store_job();
void injob(FILE *f);
void outjob(FILE *f);
void trim();
void nospace(char *s);
void delete_saxiom();
void delete_uaxiom();
void delete_connective();
void nodelcon(char *s, symb spec, int formula);


/*
*	In file isom.c
*/

void perm_initial();
void setperm();
int lower_than(int x);
int higher_than(int x);
void newperm(int *vec);
boolean isomorphic(ism ptr, trs T);
void snip(ism p);
void subst(ism p1, ism p2);
boolean isomorphic_anyhow(trs T);
void add_isoms(trs T);
boolean add_this(char mat[], ism i_tree);
ism tack_on(ism p, char mat[]);


/*
*	In file logic_io.c
*/
int newsiz();
int newneg();
int neword();
int newdes();
void sep(int *x);
int next_bit();
int got_siz();
boolean got_neg();
boolean got_ord();
boolean got_des();
void mat_print();
void newmatplus(int x, int y);
void siz_print(FILE *f, output_style x);
void neg_print(FILE *f, output_style x);
void ord_print(FILE *f, output_style x);
void des_print(FILE *f, output_style x);
void C_print(FILE *f, output_style x);
void box_print(FILE *f, output_style x);
void u_print0(FILE *f, output_style x, int y);
void u_print1(FILE *f, output_style x, int y);
void u_print2(FILE *f, output_style x, int y);
void printup(FILE *f, output_style x);
void pretty_size(FILE *f);
void pretty_negno(FILE *f);
void pretty_ordno(FILE *f);
void pretty_desno(FILE *f);
void pretty_matno(FILE *f);
void pretty_boxno(FILE *f);
void pretty_umat(FILE *f, int x);
void fail_print(FILE *f);
void insert_badvalues( int offset );
void stats_print();


/*
*	In file logic_pretest.c
*/
void new_two_ref(int a, int x, int b, int y);
boolean find_twos(unsigned info[], TRS *Tr);
void pretest_fus();
void affix_case(int a, int b, int c, int d);
void test_assertion();
void test_contraction();
void test_TW_upper_bounds();


/*
*	In file logic_set.c
*/
boolean pre_set();
boolean utest(int x, unsigned info[]);
void set_vuloc(int r, int rr);
boolean badcase(int x, unsigned info[]);
int eval(int r, unsigned z[]);
int anothercase(int x);
boolean set_poss(unsigned info[], trs T);
void fusion(unsigned info[]);
boolean permutable(int a, int b, int c, unsigned info[]);
boolean logic_poss(unsigned info[]);
void logic_axioms(boolean x);
void efficient_logic_set();


/*
*	In file logic_test.c
*/
boolean Good_matrix(unsigned info[], trs T);
void vect_into_C(unsigned info[]);
boolean fus_test(trs T);
boolean axtest(trs T);
void set_used(int x, trs T, boolean topper);
void setcon();
int getval(int y);


/*
*	In file mp_parse.c
*/
int parse( symb fla[] );
int s_parse( symb fla[] );
int Match( symb start[] );
int Finish( int subf[], symb conn[], int tot );
int Loc( symb mn, int lft, int rgt );
boolean is_var(symb x);
int symbol_position( int x, symb s );


/*
*	In file setup.c
*/
void subf_set();
int worstcase(int x, int ntop);
void set_u(int arr[], int x);
void CLoCK(int *timer);
void set_up_cc();
void job_start();
void job_stop(boolean batch);
void set_orders(char s[]);
void uglydisp(FILE *f);
void set_up_trin();

/*
*	In file wffs.c
*/

symb newsymbol( char *string, symb symbol_list1[], symb symbol_list2[] );
void wff_initial();
boolean got_formula(int x, int y, int yy, char *s);
int infml(input_case x, int y, int yy);
boolean next_symbol( char longs[] );
symb this_symbol( char *string );
boolean seek_symbol( char longs[], symb fml[], int *k );
void fix_atoms(int y, int wf);
void outformula(int p, int q, FILE *f, varmode vm);
boolean is_in(symb s, int w);
void purge( symb badsym );
