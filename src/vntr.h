
        /****************************************************************
        *                                                               *
        *                          FINDER 3.0                           *
        *                                                               *
        *           (C) 1993 Australian National University             *
        *                                                               *
        *                      All rights reserved                      *
        *                                                               *
        * The information in this software is subject to change without *
        * notice and should not be construed as a commitment by the     *
        * Australian National University. The Australian National Uni-  *
        * versity makes no representations about the suitability of     *
        * this software for any purpose. It is supplied "as is" without *
        * express or implied warranty.  If the software is modified in  *
        * a manner creating derivative copyright rights, appropriate    *
        * legends may be placed on the derivative work in addition to   *
        * that set forth above.                                         *
        *                                                               *
        * Permission to use, copy, modify and distribute this software  *
        * and its documentation for any purpose and without fee is      *
        * hereby granted, provided that both the above copyright notice *
        * and this permission notice appear in all copies and sup-      *
        * porting documentation, and that the name of the Australian    *
        * National University not be used in advertising or publicity   *
        * pertaining to distribution of the software without specific,  *
        * written prior permission.                                     *
        *                                                               *
        ****************************************************************/


#include <stdio.h>
#include <stdlib.h>

#ifndef __APPLE__
#include <malloc.h>
#endif

#include <string.h>



/*
*	Constant definitions
*/

#define REF_MAX 512
#define REF_DEFAULT 16
#define EXTRA_DEFAULT 5
#define MAX_REF_SIZE 8

#ifndef SZ
#define SZ 32
#endif
#define V_LENGTH 1024
/*
*	#define SURJ_MAX 64
*/
#define SURJ_MAX 256
#define STAK_MAX (REF_MAX*MAX_REF_SIZE)
#define RRA 50
#define RRB 30

#define NO_REPORT ((unsigned)1)
#define INT_REPORT ((unsigned)2)
#define FLOAT_REPORT ((unsigned)4)
#define DO_REPORT ((unsigned)8)

#define logical unsigned int

#define FOR(x,y) for( y = x; y; y = y->p )
#define FORR(x,y) FOR( x->cellvals, y )

/*
*	Type definitions
*/

/*
*typedef enum {
*	false,
*	true
*} boolean;
*/

#define boolean char
#define false ((boolean)0)
#define true ((boolean)1)

typedef enum {
	NOCUT,
	FOUND,
	RECORDED
} cutcode;

typedef	enum {
	PERFECT,
	NULL_SPACE,
	BIG_SPACE,
	REF_LIST,
	BUST,
	IBUST,
	NULL_REF,
	SURJES,
	SKIP
} exit_code;

typedef	enum {
	SETPOSSES,
	DOSMALLS,
	SEARCHES,
	SOLUTIONS,
	BAD_TESTS,
	SURJ_BACKTRACKS,
	NULL_BACKTRACKS,
	F_SUBSUMED,
	B_SUBSUMED,
	B1_SUBSUMED,
	PREREFS,
	REFSIZE,
	TOPLESS,
	LOC_MEMORY,
	MEMORY,
	BRANCHES,
	PRE_TIME,
	SEARCH_TIME,
	MAX_REPORT
} report_code;

typedef struct IL {
	boolean positive;	/* Represents positive literal	*/
	short int v, w;		/* A couple of integers 	*/
	struct IL *p;		/* in a list			*/
} INTLIST, *intlist;


typedef struct reftype {
	int	cardinality;	/* Just what is says		*/
	boolean	notop;		/* Useless extension omitted	*/
	intlist	cellvals;	/* The cell-valuess involved	*/
	struct reftype *nxtr;	/* Just for stack management	*/
} REF, *ref;


typedef struct rf_list {	/* A list of refutations	*/
	ref	rf;
	struct rf_list *nextref;
} REFLIST, *reflist;


typedef struct {
	int	blocks;		/* How many times blocked	*/
	ref	back;		/* For calculating secondaries	*/
	reflist	cvrefs;		/* The refs involving this c-v	*/
	reflist	posrefs;	/* Positives involving this c-v	*/
	int	vpos;		/* Offset of its cell		*/
} CELLVALUE, *cellvalue;


typedef struct {
	short	poss;		/* How many possible values	*/
	short	vpos;		/* Vector position of this cell	*/
	reflist	inject;		/* Index to injective function	*/
	CELLVALUE cvals[SZ];	/* The cell-value structures	*/
	boolean	used;		/* Involved in new refutation	*/
	boolean	has_value;	/* This cell has a value	*/
} CELL, *cell;


typedef struct {
	logical	valset;
	int	last_seen[SZ];
	intlist	cellset;
} SURJ;


typedef struct {
	logical	report_type;
	char	s_report[80];
	int	i_report;
	float	f_report;
	void	(*do_report)();
} REPORT;


typedef struct cng {
	int	forkpoint;
	int	cutval;
	logical	comvec[V_LENGTH];
	int	inorder[V_LENGTH];
	logical	possvals[V_LENGTH];
	boolean	active;	
	struct cng *lastone;
} CUTT, *cutt;


typedef struct { REF news[1024]; }	RBATCH,  *rbatch;
typedef struct { INTLIST news[1024]; }	ILBATCH, *ilbatch;
typedef struct { REFLIST news[1024]; }	RLBATCH, *rlbatch;


typedef struct {
	boolean	(*Test)(),	/* The principal test function	*/
		(*Onerefs)(),	/* Removal of impossible values */
		(*Smallrefs)();	/* Special treatment for smalls	*/
	int	vlength,	/* Local vector length		*/
		batches,	/* Local stack size / 1000	*/
		extra_batches,	/* Batches after pretest	*/
		maxref,		/* Maximum refutation size	*/
		maxbak,		/* Maximum number of backtracks */
		verb,		/* Statistics verbosity setting */
		ref_record[RRA][RRB],
		branches[SZ];
	boolean	Nosecs,		/* Suppress secondary refs	*/
		topsoff,	/* Remove redundant ref tops	*/
		report_breaks,	/* Print reason for end search 	*/
		forbidden[V_LENGTH],	/* No backtracking	*/
		done;		/* End-of-search status raised	*/
	REPORT	reps[MAX_REPORT+1];	/* Search statistics	*/
} TRIN, *trin;



typedef struct {
	trin	job;
	CELL	vector[V_LENGTH];
	logical	comvec[V_LENGTH];
	int	inorder[V_LENGTH];
	int	coinorder[V_LENGTH];
	int	priority[V_LENGTH];
	int	this_cell;
	int	susguy;
	int	firstchange;
	int	forkpoint;
	int	maxval;
	int	stakmax;
	int	unfixt;
	int	local_memory;
	int	max_batches;
	boolean	novect;
	boolean	ts;
	boolean	cut_recorded;
	boolean noref;
	rbatch	stak[REF_MAX];
	ilbatch	il_stak[STAK_MAX];
	rlbatch	rl_stak[STAK_MAX + REF_MAX];
	intlist	il_free;
	reflist	rl_free;
	reflist	duds;
	ref	newref;
	ref	stak_ptr;
	SURJ	surjection[SURJ_MAX];
	cutt	cutptr;
	cutcode	cut_flag;
} TRS, *trs;


#ifndef CLK_TCK
extern long sysconf();
#define CLK_TCK sysconf(3)
#endif


/*
*	Global variables
*/

#ifdef TRFILE
#define TR_GLOB
#else
#define TR_GLOB extern
#endif

TR_GLOB trs debugger;
TR_GLOB	int	begin_time, end_time;


/*
*	Function prototypes
*/

void	transref	( trin job );
void	init_trin	( trin tr );
void	clean_up	( trs T );
void	report_initial	( trs T );
trs	tr_initial	( trin job );
void	search		( trs T );
int	pick_cell	( trs T );
int     set_fork        ( trs T, int rc );
void	delete_cut	( trs T );
reflist	get_rl		( trs T );
void	put_rl		( trs T, reflist dead );
intlist get_il		( trs T, boolean b, int x, int y, intlist ip );
void	put_il		( trs T, intlist dead );
int	batches_used	( trs T );
ref	get_ref		( trs T );
void	put_ref		( trs T, ref dead, boolean flag );
void	rec_put_il	( trs T, intlist ip, ref r, boolean flag );
void	more_refs	( trs T );
void	more_rls	( trs T );
void	more_ils	( trs T );
boolean	invalue		( trs T, cell c, int v );
void	transfer_ref	(trs T, cell c, ref rf);
void	blockval	( trs T, cell c, int v, ref rrf, report_code RC );
boolean	surjok		( trs T );
void	outvalue	( trs T, cell c );
void	untransfer_ref	(trs T, cell c, ref rf);
void	unblockval	( trs T, cell c, int v, ref rrf );
void	mark_delete	( trs T, ref r );
void	delete_duds	( trs T );
void	delete_a_dud	( trs T, reflist r );
void	setref		( trs T );
void	reduceref	( trs T );
void	addref		( int x, trs T );
void    print_ref       ( ref r );
void    print_sus       ( trs T, cell c );
void	totsus		( trs T );
void	make_coherent	( trs T );
void	no_ref		( trs T );
void	new_small_ref	( int cells[], int vals[], boolean valency[], trs T );
void	remove_value	( trs T, cell c, int x );
intlist	get_all_ils	( int cells[], int vals[], boolean valency[], int x, trs T );
boolean	subsumed	( ref the_ref, trs T );
boolean	subsumes	( ref little_ref, ref big_ref );
void	back_sub	( trs T, ref the_ref );
void	Ref		( int x, trs T );
void	record_cut	( trs T );
void	record_part_cut	( int x, trs T );
void	complete_cut	( int ex, trs T );
boolean	prepared	( trs T );
void	setinx		( trs T );
boolean	pre_test	( trs T );
boolean	absfixt		( int x );
void	surjective	( logical vec[], logical vset, trs T );
void	injective	( logical vec[], int x, trs T );
void	co_priority	( int x, int y, trs T );
void	ref_size_report	();
void	Report		( trin tr );
void	report_branches	( trin tr );
void	dump_refs	( int rows, int columns );
void	job_done	( trin t, char *s );
int	timestamp	();
void	skipout		( char s[], int exitcode );
void test_backs         (trs T);
