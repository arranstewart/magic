
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

/*				vntr.c
*
*  This "Very New Transferred Refutations" module implements an
*  essentially very simple backtracking search algorithm. The aim is
*  to construct vectors of integer values satisfying some
*  constraints. The values are placed in the communication vector
*  T->comvec and sent to be tested for goodness by a function
*  Job->test which is independent of this module. If the vector is not
*  good, a "refutation" is constructed by means of calls to the
*  function Ref. This is a subset of the vector cell/value pairs
*  which should not be incorporated in any candidate vector. The
*  refutations, or negative constraints, are kept in a database
*  indexed by linked lists starting from the various
*  T->vector[_].cvals[_].cvrefs for fast lookup. Impossible values
*  may be removed from the sets of possibilities before the search
*  starts.  This is done by the function Job->Onerefs which is also
*  independent of any details in this file. Additionally, small
*  refutations may be detected in advance and stored in the database
*  before the search begins. This is done by Job->Smallrefs which
*  calls the function new_small_ref in this file with each
*  refutation it wants to add. "Small" is not defined.
*
*  The basic action is to pick a cell to be treated next and to insert
*  each of its possible values in turn, each time visiting the
*  constraint database to remove possible values inconsistent with
*  the chosen one and so forth, and each time recursively searching
*  the space constituted by the remaining cells.  Cells with only
*  one possible value are chosen before any others, then ones with
*  higher priority before those with lower, and within each priority
*  ones with the smallest number of remaining possible values first.
*
*  If the search space is too big to be handled in one hit, as
*  represented by a limit on the number of batches of 1024
*  refutations allowed for the database, a "cut cell" is chosen (the
*  forkpoint, or first one on which the search forks) and we divide
*  on that cell in order to conquer, first making its current value
*  the only one possible for it, and then making just the larger
*  values possible.
*
*  There are too many details of the implementation to be described in
*  this short comment. For an account, see the chapter "How It Works"
*  in the User's Guide.
*/




#define TRFILE

#include <sys/types.h>
#include <sys/times.h>
#include <limits.h>

#include "vntr.h"



/*
*  This is the top level function for this file. The "Job" contains a
*  succinct description of the problem, consisting of the length of
*  the vectors required, the addresses of the test function etc. and
*  some miscellaneous settings. The search is performed on the whole
*  space and then on each subspace created by "cutting" (see above)
*  until there is no "cut" in the stack.  */

void transref( trin Job )
{
  trs T;
  int local_time[3];
  void dump_cuts();

  if ( Job->done ) return;
  T = tr_initial( Job );
  do {
    local_time[0] = timestamp();
    if ( Job->verb > 1 ) dump_cuts(T);
    T->local_memory = 0;
    if ( prepared( T )
	 && !T->novect
	 && pre_test( T )) {
      local_time[1] = timestamp();
      Job->reps[PRE_TIME].f_report += 
	((local_time[1]-local_time[0]) * 1.0) / CLK_TCK;
      search( T );
      Job->reps[SEARCHES].i_report++;
      local_time[2] = timestamp();
      Job->reps[SEARCH_TIME].f_report +=
	((local_time[2]-local_time[1]) * 1.0) / CLK_TCK;
    }
    clean_up( T );
  }
  while ( T->cutptr && !T->job->done );
  free(T);
}



/*
*	This is not part of the search. It just initialises a job description.
*/

void init_trin( trin tr )
{
  int i, j;

  tr->Test = 0;
  tr->Onerefs = 0;
  tr->Smallrefs = 0;
  tr->batches = REF_DEFAULT;
  tr->extra_batches = EXTRA_DEFAULT;
  tr->maxref = MAX_REF_SIZE;

  for ( i = 0; i < MAX_REPORT; i++ ) {
    tr->reps[i].report_type = 0;
    tr->reps[i].i_report = 0;
    tr->reps[i].f_report = 0;
    tr->reps[i].do_report = 0;
  }

  for ( i = 0; i < RRA; i++ )
    for ( j = 0; j < RRB; j++ )
      tr->ref_record[i][j] = 0;

  for ( i = 0; i < SZ; i++ )
    tr->branches[i] = 0;
}



/*
*  Clean up after a loop through the search by deleting the database
*  and freeing the memory it used. This is mostly just garbage
*  collection.
*
*  After a search, there may be a dangling active cut, or the need for
*  a cut may have been signalled but not acted upon. This function
*  puts all such things back into their proper state, and activates
*  the cut for the next loop if there is one.  */

void clean_up( trs T )
{
  int i;

  for ( i = 0; T->stak[i] && i < T->job->batches; i++ ) {
    free( T->stak[i] );
    T->stak[i] = 0;
  }
  for ( i = 0; T->il_stak[i] && i < T->stakmax; i++ ) {
    free( T->il_stak[i] );
    T->il_stak[i] = 0;
  }
  for ( i = 0; T->rl_stak[i] && i < T->job->batches+T->stakmax; i++ ) {
    free( T->rl_stak[i] );
    T->rl_stak[i] = 0;
  }
  if ( !T->cutptr ) return;
  if ( T->cutptr->active ) delete_cut( T );
  T->cut_flag = NOCUT;
  if ( T->cutptr ) T->cutptr->active = true;
  if ( T->local_memory > T->job->reps[LOC_MEMORY].i_report )
    T->job->reps[LOC_MEMORY].i_report = T->local_memory;	
}



/*
*	It is possible to report many statistics on the execution. The routine
*	for printing these out is somewhere else in the code, but the definitions
*	of the printable quantities are here. Each report has a type (integer, 
*	floating point or function) and am identifying string.
*/

void report_initial( trs T )
{
  int i;

  switch( T->job->verb ) {
  case 0:
    T->job->reps[0].report_type = 0;
    return;

  case 1:
    for ( i = 0; i < SOLUTIONS; i++ )
      T->job->reps[i].report_type = NO_REPORT;
    strcpy(T->job->reps[SOLUTIONS].s_report,  "Solutions found:       ");
    T->job->reps[SOLUTIONS].report_type = INT_REPORT;
    T->job->reps[SOLUTIONS+1].report_type = 0;
    return;

  case 2:
    strcpy(T->job->reps[SOLUTIONS].s_report,  "Solutions found:       ");
    T->job->reps[SOLUTIONS].report_type = INT_REPORT;
	
    strcpy(T->job->reps[SEARCHES].s_report,   "Subspaces searched:    ");
    T->job->reps[SEARCHES].report_type = INT_REPORT;

    strcpy(T->job->reps[SETPOSSES].s_report,  "Subspaces prepared:    ");
    T->job->reps[SETPOSSES].report_type = INT_REPORT;

    strcpy(T->job->reps[DOSMALLS].s_report,   "Subspaces preprocessed:");
    T->job->reps[DOSMALLS].report_type = INT_REPORT;

    strcpy(T->job->reps[BAD_TESTS].s_report,  "Bad candidates tested: ");
    T->job->reps[BAD_TESTS].report_type = INT_REPORT;

    strcpy(T->job->reps[SURJ_BACKTRACKS].s_report,
	   "Surjection backtracks: ");
    T->job->reps[SURJ_BACKTRACKS].report_type = INT_REPORT;

    strcpy(T->job->reps[NULL_BACKTRACKS].s_report,
	   "Other backtracks:      ");
    T->job->reps[NULL_BACKTRACKS].report_type = INT_REPORT;

    strcpy(T->job->reps[LOC_MEMORY].s_report, "Maximum database size: ");
    T->job->reps[LOC_MEMORY].report_type = INT_REPORT;

    strcpy(T->job->reps[MEMORY].s_report,     "Total database entries:");
    T->job->reps[MEMORY].report_type = INT_REPORT;

    strcpy(T->job->reps[PRE_TIME].s_report,   "Preprocess time:       ");
    T->job->reps[PRE_TIME].report_type = FLOAT_REPORT;

    strcpy(T->job->reps[SEARCH_TIME].s_report,"Search time:           ");
    T->job->reps[SEARCH_TIME].report_type = FLOAT_REPORT;

    strcpy(T->job->reps[PREREFS].s_report,    "Pre-test refutations:  ");
    T->job->reps[PREREFS].report_type = INT_REPORT;

    strcpy(T->job->reps[F_SUBSUMED].s_report, "Forward subsumptions:  ");
    T->job->reps[F_SUBSUMED].report_type = INT_REPORT;

    strcpy(T->job->reps[B_SUBSUMED].s_report, "Back subsumptions:     ");
    T->job->reps[B_SUBSUMED].report_type = INT_REPORT;

    strcpy(T->job->reps[B1_SUBSUMED].s_report,"   Back subs by units: ");
    T->job->reps[B1_SUBSUMED].report_type = INT_REPORT;

    strcpy(T->job->reps[TOPLESS].s_report,    "No-top deletions:      ");
    T->job->reps[TOPLESS].report_type = INT_REPORT;

    strcpy(T->job->reps[BRANCHES].s_report, "Branches");
    T->job->reps[BRANCHES].report_type = DO_REPORT;
    T->job->reps[BRANCHES].do_report = report_branches;

    strcpy(T->job->reps[REFSIZE].s_report,    "Search refutations:    ");
    T->job->reps[REFSIZE].report_type = INT_REPORT | DO_REPORT;
    T->job->reps[REFSIZE].do_report = ref_size_report;

    T->job->reps[MAX_REPORT].report_type = 0;
  }
}



/*
*  This is the initialisation function for a TRS structure, which
*  contains all the variables used by functions in this file to
*  communicate with each other. It is really a subroutine of transref
*  above. The job description becomes a field of the TRS so that every
*  function can refer to it.
*/

trs tr_initial( trin Job )
{
  trs T;
  int i;

  if ( !(Job->vlength) )
    skipout( "Attempt to search null space", NULL_SPACE );
  if ( Job->vlength > V_LENGTH )
    skipout( "Search space too big to fit in vector length", BIG_SPACE );
  if ( Job->batches < 1 ) {
    fprintf(stderr,"Setting stack size to 1\n");
    Job->batches = 1;
  }
  if ( Job->extra_batches < 1 ) {
    fprintf(stderr,"Setting marginal stack size to 1\n");
    Job->extra_batches = 1;
  }
  if ( Job->batches >= REF_MAX ) {
    fprintf(stderr,"Setting stack size to %d\n", REF_MAX-1);
    Job->batches = REF_MAX-1;
  }
  if ( Job->maxref < 1 ) {
    fprintf(stderr,"Setting ref-length to 1\n");
    Job->maxref = 1;
  }
  if ( Job->maxref > MAX_REF_SIZE )
    fprintf(stderr, "Warning: ref-length %d exceeds notional maximum %d\n",
	    Job->maxref, MAX_REF_SIZE);

  T = (trs) malloc(sizeof(TRS));
  T->job = Job;
  T->max_batches = T->job->batches - 1;
  T->stakmax = (Job->maxref < 4? 4: Job->maxref) * Job->batches;
  report_initial( T );
  T->cutptr = 0;
  T->noref = false;
  T->cut_flag = NOCUT;
  for ( i = 0; i <= T->job->vlength; i++ ) T->priority[i] = 0;
  return T;
}




/*********************************************************************
*
*  This is where the going gets serious. The search function calls
*  itself recursively to search the subspaces determined by setting
*  the value of the picked cell successively in each possible
*  way. Value wy is inserted in cell ex. If the picked cell is -1 this
*  means the vector is complete and should be tested.
*
*  When the program is re-constituting its search after a "cut", it
*  has to take account of the values that cells had before the
*  cut. This is done by attending to the starting point for the loop
*  of assignments to wy.
*/

void search( trs T )
{
  cell c;
  int ex, wy;
  boolean inok, ccut, fch;

  if ( T->job->done || T->novect ) return;

  ex = pick_cell( T );
  if ( ex < 0 ) {
    if ( T->job->Test ) {
      T->noref = false;
      if ( (*T->job->Test)(T->comvec,T) ) {
	T->job->reps[SOLUTIONS].i_report++;
	T->firstchange = V_LENGTH;
      }
      else {
	if ( T->noref ) T->noref = false;
	else setref( T );
	T->job->reps[BAD_TESTS].i_report++;
      }
    }
    else T->job->done = true;
  }
  else {
    c = T->vector + ex;
    for ( wy = (T->cutptr && T->cutptr->active)? T->cutptr->comvec[ex] :0;
	  wy <= T->maxval && !T->novect; wy++ )
      if ( !(c->cvals[wy].blocks) ) {
	if ( T->cutptr && T->cutptr->active 
	     && wy != T->cutptr->comvec[ex] ) delete_cut( T );
	inok = invalue( T, c, wy );
	if ( T->cut_flag == FOUND && T->forkpoint >= 0 )
	  record_cut( T );
	else if ( inok && !T->ts ) search( T );
	if ( T->job->done ) return;
	if ( T->ts ) totsus( T );
	ccut = (T->cut_flag == RECORDED && ex == T->forkpoint);
	if ( (fch = (T->coinorder[ex]) <= T->firstchange) )
	  T->firstchange = T->coinorder[ex];
	outvalue( T, c );
	if ( T->ts && T->vector[T->susguy].poss ) T->ts = false;
	if ( ccut ) complete_cut( ex, T );
	if ( T->newref ) {
	  if ( c->used ) {
	    addref( ex, T );
	    if ( T->ts ) totsus( T );
	  }
	  else break;
	}
	if ( fch && T->job->forbidden[ex] ) break;
      }
  }
}



/*
*	The next routine picks the next cell to have its value fixed.
*	If there is a cell with only one possible value, that is picked.
*	Otherwise, pick the cell with the smallest number of possible 
*	values among those with the highest priority.
*	If there is no open cell, return -1.
*
*	Things are a little different when there is an active cut, since
*	the change order up to the cut point must be reproduced exactly
*	in order to maintain integrity.
*/

int pick_cell( trs T )
{
  int rc;		/* Return code */
  int smallest;	/* Cardinality of cell rc */
  int best;	/* Priority of cell rc */
  int i;

  if ( T->cutptr && T->cutptr->active ) {
    for ( i = 0; i < T->job->vlength && T->inorder[i] >= 0; i++ );
    if ( i < T->job->vlength && (rc = T->cutptr->inorder[i]) >= 0 ) {
      if ( T->forkpoint < 0 && T->vector[rc].poss > 1 )
        T->forkpoint = rc;
      return rc;
    }
    delete_cut( T );
  }

  smallest = SZ;
  best = 0;
  rc = -1;
  while ( T->unfixt >= 0 && T->vector[T->unfixt].has_value ) T->unfixt--;

  for (i = T->unfixt; i >= 0; i-- )
    if ( !(T->vector[i].has_value) && T->vector[i].poss == 1 ) {
      T->job->branches[1]++;
      return i;
    }
    else if ( !(T->vector[i].has_value) && T->priority[i] >= best
	      && ( T->priority[i] > best || T->vector[i].poss < smallest )) {
      rc = i;
      smallest = T->vector[i].poss;
      best = T->priority[i];
    }
  
  if (rc >= 0) {
    T->job->branches[smallest]++;
    if ( T->forkpoint < 0 && T->vector[rc].poss > 1 )
	  T->forkpoint = rc;
  }
  return rc;
}



/*
*	Old cuts are trashed to stop them interfering with the search.
*/

void delete_cut( trs T )
{
  cutt ftc;

  ftc = T->cutptr;
  T->cutptr = ftc->lastone;
  free(ftc);
}



/*
*	The next few functions have to do with allocation and de-allocation of
*	refutations, refutation-list entries and integer-pairs. Some of this is
*	a little intricate, but none of it is deep.
*
*	First, to get a refutation list entry.
*/

reflist get_rl( trs T )
{
  reflist r;

  if ( !T->rl_free->nextref ) more_rls( T );
  r = T->rl_free;
  T->rl_free = T->rl_free->nextref;
  return r;
}


/*
*	Conversely, to put a dead one to rest.
*/

void put_rl( trs T, reflist dead )
{
  if ( !dead ) return;
  dead->rf = 0;
  dead->nextref = T->rl_free;
  T->rl_free = dead;
}


/*
*	Removing a refutation is more than just putting the stake through its 
*	heart. It may occur in several lists. Here is the routine to delete 
*	the reference to it from a given list and to kill that rl.
*/

void remove_rl( trs T, ref r, reflist *source )
{
  reflist rp, rs;

  if ( !*source ) return;
  if ( (*source)->rf == r ) {
    rp = (*source)->nextref;
    put_rl( T, *source );
    *source = rp;
    return;
  }

  for ( rs = *source; rs->nextref; rs = rs->nextref )
    if ( rs->nextref->rf == r ) {
      rp = rs->nextref->nextref;
      put_rl( T, rs->nextref );
      rs->nextref = rp;
      return;
    }
}



/*
*	Now there is analogous stuff for integer-pair lists. When one of these
*	is allocated, it has its initial values kindly filled in here.
*/

intlist get_il( trs T, boolean b, int x, int y, intlist nx )
{
  intlist ip;

  if ( !T->il_free->p ) more_ils( T );
  ip = T->il_free;
  T->il_free = T->il_free->p;
  ip->positive = b;
  ip->v = x;
  ip->w = y;
  ip->p = nx;
  return ip;
}


/*
*	This is how to put down a pair of integers.
*/

void put_il( trs T, intlist dead )
{
  if ( !dead ) return;
  dead->v = -1;
  dead->p = T->il_free;
  T->il_free = dead;
}



/*
*	Getting a ref involves filling it with default values.
*/

ref get_ref( trs T )
{
  ref r;

  if ( !T->stak_ptr->nxtr ) more_refs( T );
  r = T->stak_ptr;
  T->stak_ptr = T->stak_ptr->nxtr;
  r->cardinality = 0;
  r->notop = false;
  r->cellvals = 0;
  T->job->reps[MEMORY].i_report++;
  T->local_memory++;
  if ( batches_used(T) > T->max_batches )
    T->cut_flag = FOUND;
  return r;
}



/*
*	And losing it again involves first getting rid of the cell-value pairs
*	which constitute the refutation proper. For this, call recursive put_il.
*/

void put_ref( trs T, ref dead, boolean flag )
{
  rec_put_il( T, dead->cellvals, dead, flag );
  dead->cardinality = -1;
  dead->nxtr = T->stak_ptr;
  T->stak_ptr = dead;
  T->job->reps[MEMORY].i_report--;
  T->local_memory--;
}


/*
*	Here's the recursive put_il, which deletes a whole list of integer pairs.
*/

void rec_put_il( trs T, intlist ip, ref r, boolean flag )
{
  if ( !ip ) return;
  rec_put_il( T, ip->p, r, flag );
  if ( flag ) {
    if ( ip->positive ) 
      remove_rl( T, r, &(T->vector[ip->v].cvals[ip->w].posrefs) );
    else
      remove_rl( T, r, &(T->vector[ip->v].cvals[ip->w].cvrefs) );
    if ( T->vector[ip->v].cvals[ip->w].back == r )
      skipout("Ghost in the back field",SKIP);
  }
  put_il( T, ip );
}



/*
*	In order to know when to cut-and-guess, we need to know how many 
*	batches of 1024 refutations have been used. So we count them!
*/

int batches_used( trs T )
{
  int i;

  for ( i = 0; i <= T->job->batches; i++ )
    if ( !T->stak[i] ) return i;
  skipout("Too many refutations requested",BUST);
  /* That causes an exit, but just to keep the warnings quiet: */
  return i;
}



/*
*  The next three routines are for grabbing new chunks of memory to
*  hold refutations, rls or ils. The relevant data structure is an
*  array of pointers to such chunks. That is, T->stak[x] is a pointer
*  to a structure which consists simply of an array of 1024 refs.
*/

void more_refs( trs T )
{
  int i;
  ref r;

  i = batches_used( T );
  T->stak[i] = (rbatch) malloc(sizeof(RBATCH));
  for ( r = T->stak[i]->news; r-T->stak[i]->news < 1024; r++ ) {
    r->nxtr = (r == T->stak[i]->news+1023? 0: r+1);
    r->cardinality = -1;
  }
  if ( i ) T->stak_ptr->nxtr = T->stak[i]->news;
  else T->stak_ptr = T->stak[i]->news;
  return;
}



/*
*	Getting more rls is pretty much like getting more refs.
*/

void more_rls( trs T )
{
  int i;
  reflist rp;

  for ( i = 0; i <= T->stakmax+T->job->batches; i++ )
    if ( !T->rl_stak[i] ) {
      T->rl_stak[i] = (rlbatch) malloc(sizeof(RLBATCH));
      for ( rp = T->rl_stak[i]->news; rp-T->rl_stak[i]->news < 1024; rp++ )
	rp->nextref = (rp == T->rl_stak[i]->news+1023? 0: rp+1);
      if ( i ) T->rl_free->nextref = T->rl_stak[i]->news;
      else T->rl_free = T->rl_stak[i]->news;
      return;
    }
}



/*
*	And getting more ils is similar.
*/

void more_ils( trs T )
{
  int i;
  intlist ip;

  for ( i = 0; i <= T->stakmax; i++ )
    if ( !T->il_stak[i] ) {
      T->il_stak[i] = (ilbatch) malloc(sizeof(ILBATCH));
      for ( ip = T->il_stak[i]->news; ip-T->il_stak[i]->news < 1024; ip++ )
	ip->p = (ip == T->il_stak[i]->news+1023? 0: ip+1);
      if ( i ) T->il_free->p = T->il_stak[i]->news;
      else T->il_free = T->il_stak[i]->news;
      return;
    }
}



/******************************************************************
*
*  Now back to the plot. The next few functions are where a healthy
*  run of any vntr program spends most of its time.
*
* First we have the routine to insert a new value v in a cell c.
*/

boolean invalue( trs T, cell c, int v )
{
  reflist r;
  intlist ip;
  int i;

  if ( T->job->done ) return false;
  if ( T->job->maxbak &&
       T->job->reps[NULL_BACKTRACKS].i_report +
       T->job->reps[SURJ_BACKTRACKS].i_report +
       T->job->reps[SOLUTIONS].i_report +
       T->job->reps[BAD_TESTS].i_report >= T->job->maxbak ) {
    job_done( T->job, "limit on backtracks" );
    return false;
  }
  c->has_value = true;
  T->comvec[c->vpos] = v;
  T->inorder[++T->this_cell] = c->vpos;
  T->coinorder[c->vpos] = T->this_cell;

  for ( r = c->cvals[v].cvrefs; r; r = r->nextref )
    transfer_ref( T, c, r->rf );

  for ( i = 0; i <= T->maxval; i++ ) if ( i != v )
    for ( r = c->cvals[i].posrefs; r; r = r->nextref )
      transfer_ref( T, c, r->rf );

  for ( r = c->inject; r; r = r->nextref )
    if ( v < r->rf->cardinality ) {
      FORR( r->rf, ip )
	if ( !(T->vector[ip->v].has_value) )
	  blockval( T, T->vector+ip->v, v, r->rf, NULL_BACKTRACKS );
    }

  return surjok( T );
}


/*
*  This is just a subroutine of invalue. It decrements the *
*  cardinality of a refutation. If the result is cardinality 1 * then
*  either there is a value inserted in some cell other than * the
*  value involved at that cell, in which case no action need * be
*  taken, or the refutation now blocks the insertion of a * value. In
*  the latter case, the value is marked as blocked and * if this newly
*  removes it the `poss' count for its cell is * decremented. If this
*  empties the possval set, we are ready to * backtrack.  */

void transfer_ref( trs T, cell c, ref rf )
{
  intlist ip, iq;
  int j;

  if ( --(rf->cardinality) == 1 ) {
    iq = 0;
    FORR( rf, ip ) {
      if ( !(T->vector[ip->v].has_value) ) {
	if ( ip->positive ) {
	  for ( j = 0; j <= T->maxval; j++ )
	    if ( j != ip->w )
	      blockval( T, T->vector+ip->v, j, rf,
			NULL_BACKTRACKS );
	}
	else	blockval( T, T->vector+ip->v, ip->w, rf,
			  NULL_BACKTRACKS );
	if ( iq ) {
	  iq->p = ip->p;
	  ip->p = rf->cellvals;
	  rf->cellvals = ip;
	}
	return;
      }
      else if (( ip->positive && T->comvec[ip->v] == ip->w )
	       || ( !ip->positive && T->comvec[ip->v] != ip->w )) {
	if ( iq ) {
	  iq->p = ip->p;
	  ip->p = rf->cellvals;
	  rf->cellvals = ip;
	}
	return;
      }
      iq = ip;
    }
  }
}



/*
*  The subroutine to block a value in the light of a transferred
*  refutation could be left in-line, but since it is called from three
*  places, it is better isolated like this.
*/

void blockval( trs T, cell c, int v, ref rrf, report_code RC )
{
  c->cvals[v].blocks++;
  if ( c->cvals[v].blocks == 1 ) {
    c->poss--;
    if ( !(c->poss) && !T->ts ) {
      T->ts = true;
      T->susguy = c->vpos;
      if ( RC ) T->job->reps[RC].i_report++;
    }
    c->cvals[v].back = rrf;
  }
}



/*
*	Injective and surjective fragments of the vector are given special 
*	treatment. This routine checks the surjections to ensure that every 
*	value is still possible for at least one of the cells. The trick of
*	remembering the cell where the value was last seen and looking there
*	first keeps time spent on this to something rather small, and where 
*	surjectiveness is important, surjok pays its way handsomely.
*	
*/

boolean surjok( trs T )
{
  int i, j, ls;
  intlist ip;
  static boolean onoff;

  if ( T->ts ) return true;
  if ( onoff ) { onoff = false; return true; }
  onoff = true;
  for ( i = 0; T->surjection[i].valset; i++ )
    for ( j = 0; j <= T->maxval; j++ )
      if ( T->surjection[i].valset & (1 << j) ) {
	ls = T->surjection[i].last_seen[j];
	if ( T->vector[ls].cvals[j].blocks || 
	     ( T->vector[ls].has_value && T->comvec[ls] != j )) {
	  FOR( T->surjection[i].cellset, ip )
	    if ( !(T->vector[ip->v].cvals[j].blocks) &&
		 (!T->vector[ip->v].has_value || T->comvec[ip->v] == j)) {
	      T->surjection[i].last_seen[j] = ip->v;
	      /* Try this */
	      ip->v = T->surjection[i].cellset->v;
	      T->surjection[i].cellset->v = T->surjection[i].last_seen[j];
	      /* for efficiency */
	      break;
	    }
	  if ( !ip ) {
	    T->job->reps[SURJ_BACKTRACKS].i_report++;
	    return false;
	  }
	}
      }
  return true;
}



/*
*	The converse of putting a value in is taking one out. Why do I have to 
*	point that out to you? Note that these functions maintain T->inorder[x] 
*	(the x-th cell to get a value) and T->coinorder[x] (the y such that x 
*	is T->inorder[y]). These arrays are set to -1 where undefined.
*/

void outvalue( trs T, cell c )
{
  intlist ip;
  reflist r;
  int i;

  for ( r = c->cvals[T->comvec[c->vpos]].cvrefs; r; r = r->nextref )
    untransfer_ref( T, c, r->rf );

  for ( i = 0; i <= T->maxval; i++ ) if ( i != T->comvec[c->vpos] )
    for ( r = c->cvals[i].posrefs; r; r = r->nextref )
      untransfer_ref( T, c, r->rf );

  for ( r = c->inject; r; r = r->nextref )
    if ( T->comvec[c->vpos] < r->rf->cardinality ) {
      FORR( r->rf, ip )
	if ( !(T->vector[ip->v].has_value) )
	  unblockval( T, T->vector+ip->v, T->comvec[c->vpos], r->rf );
    }

  if ( c->vpos == T->forkpoint ) {
    c->cvals[T->comvec[c->vpos]].blocks++;
    if ( c->cvals[T->comvec[c->vpos]].blocks == 1 )
      if ( --c->poss == 1 ) T->forkpoint = -1;
  }
  c->has_value = false;
  T->comvec[c->vpos] = 0;
  T->inorder[T->this_cell--] = -1;
  T->coinorder[c->vpos] = -1;
  if ( c->vpos > T->unfixt ) T->unfixt = c->vpos;
  if ( T->duds && T->cut_flag != RECORDED ) delete_duds( T );
}


/*
*	The converse of transfer_ref. This unblocks any blocked cell after 
*	incrementing the cardinality of the refutation.
*/

void untransfer_ref(trs T, cell c, ref rf)
{
  int i;
  intlist ip;

  rf->cardinality++;
  if ( rf->cardinality == 2 && !T->vector[rf->cellvals->v].has_value ) {
    ip = rf->cellvals;
    if ( ip->positive ) {
      for ( i = 0; i <= T->maxval; i++ ) if ( i != ip->w )
	unblockval( T, T->vector+ip->v, i, rf );
    }
    else	unblockval( T, T->vector+ip->v, ip->w, rf );
  }

  if ( rf->notop ) {
    FORR( rf, ip )
      if ( ip->v != c->vpos && T->vector[ip->v].has_value )
	break;
    if ( !ip ) {
      mark_delete( T, rf );
      T->job->reps[TOPLESS].i_report++;
    }
  }
}



/*
*	Unblockval is the converse of blockval: re-transfer a refutation by 
*	deleting the block and adjusting the poss and back fields as necessary.
*/

void unblockval( trs T, cell c, int v, ref rrf )
{
  if ( !(--c->cvals[v].blocks) ) c->poss++;
  if ( c->cvals[v].back == rrf ) c->cvals[v].back = 0;
}




/*
*	Refutations are deleted when they are no longer meaningful. Those
*	to be deleted are marked for death while other things are going on
*	and later executed all together.
*/

void mark_delete( trs T, ref r )
{
  reflist rr;

  rr = get_rl( T );
  rr->rf = r;
  rr->nextref = T->duds;
  T->duds = rr;
}


/*
*	This empties the list of duds created by calls to mark_delete.
*/

void delete_duds( trs T )
{
  delete_a_dud( T, T->duds );
  T->duds = 0;
}


/*
*	And this is the recursive work routine for delete_duds.
*/

void delete_a_dud( trs T, reflist r )
{
  if ( !r ) return;
  delete_a_dud( T, r->nextref );
  put_ref ( T, r->rf, true );
  put_rl( T, r );
}





/*
*  When a new refutation has been produced, it is sent to setref to
*  have any unnecessary parts removed and generally to be smartened up
*  for its entry into the database. First any cells which have
*  absolutely only one * possible value are not really involved in the
*  refutation and can be * removed. Secondly, if the refutation
*  involves the two or more most * significant cells (not including
*  the absolutely fixed ones) then all * but the least significant of
*  these can be deleted, since when the search * backtracks far enough
*  to change that cell, this refutation can never * again be
*  active. In such a case it is marked as topless so that outvalue *
*  knows to delete it when its useful life is over. Finally, any
*  refutation * which is unreasonably large occupies a lot of the
*  database and yields * little information, so we can pretend it is a
*  small refutation until such * time as this pretence is no longer
*  safe, then throw it away. We do this * by treating it as though it
*  involved all of the cells more significant * than a chosen one and
*  applying the "tops off" strategy above.
*/

void setref( trs T )
{
  int size_before, size_after, i, j;
  int latest, del;

  if ( !T->newref ) {
    T->novect = true;
    return;
  }
  size_before = size_after = T->newref->cardinality;
  for ( latest = 0; latest < T->job->vlength; latest++ )
    if ( T->inorder[latest] < 0 ||
	 T->vector[T->inorder[latest]].poss > 1 ) break;
    else if ( T->vector[T->inorder[latest]].used ) {
      T->vector[T->inorder[latest]].used = false;
      size_after--;
    }
  if ( size_after < size_before ) reduceref( T );
  if ( !T->newref->cardinality ) {
    T->novect = true;
    return;
  }
  size_before = size_after;

  for ( del = -1;
	T->job->topsoff &&
	  latest < T->job->vlength &&
	  T->inorder[latest] >= 0 &&
	  T->vector[T->inorder[latest]].used;
	latest++ ) {
    if ( del >= 0 ) {
      T->vector[T->inorder[del]].used = false;
      size_after--;
    }
    del = latest;
  }
  if ( size_after < size_before ) {
    reduceref( T );
    T->newref->notop = true;
  }

  i = size_before;  j = size_after;
  if ( i >= RRA ) i = RRA-1;
  if ( j >= RRB ) j = RRB-1;
  T->job->ref_record[i][j]++;
  size_before = size_after = T->newref->cardinality;

  while ( size_after > T->job->maxref ) {
    if ( T->inorder[latest] >= 0 && T->vector[T->inorder[latest]].used ) {
      if ( del >= 0 ) {
	T->vector[T->inorder[del]].used = false;
	size_after--;
      }
      del = latest;
    }
    latest++;
  }
  if ( size_after < size_before ) {
    reduceref ( T );
    T->newref->notop = true;
  }
}



/*
*	This is the routine to take unused cell-value pairs out of a refutation.
*	It comes in two stages: one to take care of an unused starter pair and 
*	one for the other cases.
*/

void reduceref( trs T )
{
  intlist ip, iq;

  if ( !T->newref->cellvals ) return;
  while ( !T->vector[T->newref->cellvals->v].used ) {
    ip = T->newref->cellvals->p;
    put_il( T, T->newref->cellvals );
    T->newref->cellvals = ip;
    T->newref->cardinality--;
    if ( !ip ) {
      T->novect = true;
      return;
    }
  }

  FORR( T->newref, ip )
    while ( ip->p && !(T->vector[ip->p->v].used) ) {
      iq = ip->p->p;
      put_il( T, ip->p );
      ip->p = iq;
      T->newref->cardinality--;
    }
}



/*
*	Adding a refutation to the database is reasonably straightforward.
*	The call to this comes when backtracking has reached the point where
*	the least significant cell involved has just lost its value. The other
*	cells involved still have their values.
*/

void addref( int x, trs T )
{
  intlist ip;
  reflist rp;
  int i;
  int ipv, ipw;
  boolean ipb;

  if ( !T->newref ) skipout("Null refutation",NULL_REF);
  if ( T->newref->cardinality == 1 && T->newref->notop ) {
    T->vector[x].used = false;
    put_ref( T, T->newref, true );
    T->newref = 0;
    return;
  }
  if ( !(T->newref->cardinality) ) {
    T->novect = true;
    return;
  }
  T->job->reps[REFSIZE].i_report++;

  FORR( T->newref, ip )
    if ( !ip->positive && T->vector[ip->v].cvals[ip->w].blocks ) {
      FORR( T->newref, ip )
	T->vector[ip->v].used = false;
      put_ref( T, T->newref, true );
      T->newref = 0;
      return;
    }

  FORR( T->newref, ip ) {
    T->vector[ip->v].used = false;
    rp = get_rl( T );
    rp->rf = T->newref;
    if ( ip->positive ) {
      rp->nextref = T->vector[ip->v].cvals[ip->w].posrefs;
      T->vector[ip->v].cvals[ip->w].posrefs = rp;
    }
    else {
      rp->nextref = T->vector[ip->v].cvals[ip->w].cvrefs;
      T->vector[ip->v].cvals[ip->w].cvrefs = rp;
    }
    if ( ip->v == x ) {
      if ( ip->positive ) {
	for ( i = 0; i <= T->maxval; i++ ) if ( i != ip->w )
	  blockval( T, T->vector+x, i, rp->rf, 0 );
      }
      else blockval( T, T->vector+x, ip->w, rp->rf, 0 );
      ipv = ip->v;  ipw = ip->w;  ipb = ip->positive;
      ip->v = T->newref->cellvals->v;
      ip->w = T->newref->cellvals->w;
      ip->positive = T->newref->cellvals->positive;
      T->newref->cellvals->v = ipv;
      T->newref->cellvals->w = ipw;
      T->newref->cellvals->positive = ipb;
    }
    else T->newref->cardinality--;
  }
  if ( T->newref->cardinality != 1 )
    skipout("Refutation cardinality not correctly set",SKIP);
  for ( i = 0; i < T->job->vlength; i++ )
    if ( T->vector[i].used ) skipout("Used not reset",SKIP);
  T->newref = 0;
}



void print_ref( ref r )
{
  intlist ip;

  if ( !r ) return;
  FORR(r,ip) printf(" %d:%d%c ",ip->v, ip->w, ip->positive? '*': ' ');
  if ( r->notop ) printf(" +");
}


void print_sus( trs T, cell c )
{
  intlist ip;
  int i;

  puts("");
  for ( i = 0; i < T->maxval; i++ ) {
    printf("    %d:%d",c->vpos,i);
    if ( c->cvals[i].back )
      FORR(c->cvals[i].back,ip) if ( ip->v == c->vpos )
	printf("%c  ",ip->positive? '*': ' ');
    if ( c->cvals[i].back )
      FORR(c->cvals[i].back,ip) if ( ip->v != c->vpos )
	printf("%d:%d%c  ", ip->v, ip->w, ip->positive? '*': ' ');
    puts("");
  }
}



void totsus( trs T )
{
  int i, addval;
  cell c;
  intlist ip, jp, kp=0;
  boolean pos_used[V_LENGTH];

  if ( T->job->Nosecs ) {
    if ( T->vector[T->susguy].poss ) T->ts = false;
    return;
  }
  T->ts = false;
  c = T->vector + T->susguy;
  T->susguy = 0;
  T->newref = get_ref( T );
  for (i=0; i<V_LENGTH; i++) pos_used[i] = false;

  for ( i = 0; i <= T->maxval; i++ ) if ( c->cvals[i].back ) {
    if ( c->cvals[i].back->notop ) T->newref->notop = true;
  }

  for ( i = 0; i <= T->maxval; i++ ) if ( c->cvals[i].back )
    FORR( c->cvals[i].back, ip )
      if ( ip->v != c->vpos ) {
	if ( ip->w < 0 ) {	/* The case of an injection */
	  if ( T->vector[ip->v].has_value
	       && T->comvec[ip->v] == i )
	    addval = i;
	  else addval = -1;
	}
	else addval = ip->w;
	if ( ip->positive ) pos_used[ip->v] = true;			
	if ( (pos_used[ip->v] || !T->vector[ip->v].used)
	     && addval >= 0 ) {
	  jp = get_il( T, ip->positive, ip->v, addval, 0 );
	  if ( !T->newref->cellvals )
	    T->newref->cellvals = jp;
	  else kp->p = jp;
	  kp = jp;
	  T->vector[jp->v].used = true;
	  T->newref->cardinality++;
	}
      }
  make_coherent( T );

  if ( T->newref->notop ) {
    ip = T->newref->cellvals;
    FOR( ip->p, jp )
      if ( T->coinorder[jp->v] < T->coinorder[ip->v] )
	T->vector[jp->v].used = false;
      else {
	T->vector[ip->v].used = false;
	ip = jp;
      }
    reduceref( T );
  }
  else setref( T );
}






/*
*	At present, the program is not set up to deal with two positive
*	cell-value pairs in a refutation which both pertain to the same
*	cell. That is, we cannot represent a disjunction saying that cell
*	c has "either value v1 or value v2". The following subroutine of
*	totsus checks for such difficulties and if they are present 
*	substitutes for the offending set of pairs the negative one that
*	the cell should not have its current value.
*/

void make_coherent( trs T )
{
  intlist ip, iq, ir;

  FORR(T->newref,ip) if ( ip->positive ) {
    while ( T->newref->cellvals != ip
	    && T->newref->cellvals->v == ip->v ) {
      ir = T->newref->cellvals->p;
      put_il( T, T->newref->cellvals );
      T->newref->cellvals = ir;
      T->newref->cardinality--;
      ip->w = -1;
    }
    for ( iq = T->newref->cellvals; iq != ip && iq->p != ip; iq = iq->p )
      while ( iq->p != ip && iq->p->v == ip->v ) {
	ir = iq->p;
	iq->p = ir->p;
	put_il( T, ir );
	T->newref->cardinality--;
	ip->w = -1;
      }
    for ( iq = ip; iq && iq->p; iq = iq->p )
      while ( iq->p && iq->p->v == ip->v ) {
	if ( iq->p->w != ip->w ) ip->w = -1;
	ir = iq->p;
	iq->p = ir->p;
	put_il( T, ir );
	T->newref->cardinality--;
      }
    if ( ip->w < 0 ) {
      ip->positive = false;
      ip->w = T->comvec[ip->v];
    }
  }
}





void new_small_ref( int cells[], int vals[], boolean valency[], trs T )
{
  ref the_ref;
  reflist rp, rrp, *tvc;
  int i, j, k, m;
  int refcard, maxrefcard;
  boolean b;

	/* Cludge at this stage: don't allow refutations with more than
	   one positive member. This restriction can be removed once the
	   basic program is running OK. */

  for ( i = 0; cells[i] >= 0; i++ ) if ( valency[i] )
    for ( j = i+1; cells[j] >= 0; j++ ) if ( valency[j] )
      if ( cells[i] != cells[j] || vals[i] != vals[j] )
	skipout("Refutation clause is non-Horn",SKIP);

	/* Check for subsumption by a one-refutation 
	   and remove any fixed cell-value pairs. Then
	   skip out of the search if the refutation is null */

  for ( maxrefcard = 0; cells[maxrefcard] >= 0; maxrefcard++ );
  for ( i = 0; cells[i] >= 0; i++ ) {
    if ( valency[i] ) {
      if ( T->vector[cells[i]].cvals[vals[i]].blocks ) {
	for ( j = i; cells[j] >= 0; j++ ) {
	  cells[j] = cells[j+1];
	  vals[j] = vals[j+1];
	  valency[j] = valency[j+1];
	}
	i--;	
      }
      else if ( T->vector[cells[i]].poss == 1 ) return;
    }
    else {
      if ( T->vector[cells[i]].cvals[vals[i]].blocks ) return;
      if ( T->vector[cells[i]].poss == 1 ) {
	for ( j = i; cells[j] >= 0; j++ ) {
	  cells[j] = cells[j+1];
	  vals[j] = vals[j+1];
	  valency[j] = valency[j+1];
	}	
	i--;
      }
    }
  }
  if ( !(refcard = i) ) {
    T->novect = true;
    return;
  }

	/* Sort the refutation, removing repetitions. Remember that 
	   refcard is likely to be about 2 or 3 (4 at most) so 
	   the crudest sorting method is as good as anything!       */

  b = true; while (b) {
    b = false;
    for ( i = 0; cells[i] >= 0 && cells[i+1] >= 0; i++ )
      if ( cells[i] > cells[i+1] ) {
	b = true;
	j = cells[i];  k = vals[i];  m = valency[i];
	cells[i] = cells[i+1];  vals[i] = vals[i+1];
	valency[i] = valency[i+1];
	cells[i+1] = j;  vals[i+1] = k; valency[i+1] = m; 
      }
      else if ( cells[i] == cells[i+1] ) {
	if ( vals[i] == vals[i+1] ) {
	  if ( valency[i] != valency[i+1] ) return;
	  for ( j = i; cells[j] >= 0; j++ ) {
	    cells[j] = cells[j+1];
	    vals[j] = vals[j+1];
	    valency[j] = valency[j+1];
	  }
	  i--;
	  refcard--;
	  b = true;
	}
	else {
	  if ( !valency[i] && !valency[i+1] ) return;
	  for ( j = (valency[i]? i: i+1); cells[j] >= 0; j++ ) {
	    cells[j] = cells[j+1];
	    vals[j] = vals[j+1];
	    valency[j] = valency[j+1];
	  }
	  i--;
	  refcard--;
	  b = true;
	}
      }
  }
	
  if ( refcard == 1 ) {
    if ( valency[0] ) {
      for ( i = 0; i <= T->maxval; i++ ) if ( i != vals[0] )
	if ( !T->vector[cells[0]].cvals[i].blocks )
	  remove_value( T, T->vector+cells[0], i );
    }
    else remove_value( T, T->vector+cells[0], vals[0] );
  }
  else if ( !T->cut_flag ) {
    the_ref = get_ref( T );
    the_ref->cardinality = refcard;
    the_ref->cellvals = get_all_ils( cells, vals, valency, 0, T );
    if ( subsumed(the_ref, T) ) {
      put_ref( T, the_ref, false );
      T->job->reps[F_SUBSUMED].i_report++;
      return;
    }
    T->job->reps[PREREFS].i_report++;
    for ( i = 0; cells[i] >= 0; i++ ) {
      rp = get_rl( T );
      rp->rf = the_ref;
      if ( valency[i] )
	tvc = &(T->vector[cells[i]].cvals[vals[i]].posrefs);
      else tvc = &(T->vector[cells[i]].cvals[vals[i]].cvrefs);
      if ( !*tvc || ((*tvc)->rf->cellvals->v != cells[i])
	   || the_ref->cellvals->v == cells[i] ) {
	rp->nextref = *tvc;
	*tvc = rp;
      }
      else {
	for (rrp=*tvc; rrp->nextref; rrp=rrp->nextref)
	  if ( rrp->nextref->rf->cellvals->v != cells[i] )
	    break;
	rp->nextref = rrp->nextref;
	rrp->nextref = rp;
      }
    }
    if ( refcard < maxrefcard ) back_sub( T, the_ref );
  }
}




void remove_value( trs T, cell c, int x )
{
  reflist rp;
  intlist ip, iq;
  int i;

  c->cvals[x].blocks++;
  if ( c->cvals[x].blocks == 1 ) {
    if ( !--c->poss ) T->novect = true;
  }
  else skipout("Nonexistent value removed by 1-refutation",SKIP);
  for ( rp = c->cvals[x].cvrefs; rp; rp = rp->nextref ) {
    FORR( rp->rf, ip ) if ( ip->v == c->vpos ) break;
    if ( !ip ) skipout("Error in refutation",SKIP);
    if ( ip->positive ) {
      if ( ip == rp->rf->cellvals ) rp->rf->cellvals = ip->p;
      else {
	FORR( rp->rf, iq )
	  if ( iq->p == ip ) break;
	iq->p = ip->p;
      }
      put_il( T, ip );
      remove_rl( T, rp->rf, &(c->cvals[x].cvrefs) );
      rp->rf->cardinality--;
      if ( rp->rf->cardinality == 1 ) {
	if ( rp->rf->cellvals->positive ) {
	  for ( i = 0; i <= T->maxval; i++ )
	    if ( i != rp->rf->cellvals->w )
	      if ( !T->vector[rp->rf->cellvals->v].cvals[i].blocks )
		remove_value( T, T->vector+rp->rf->cellvals->v, i );
	}
	else remove_value(T,T->vector+rp->rf->cellvals->v,rp->rf->cellvals->w);
      }
      else back_sub( T, rp->rf );
    }
    else {
      mark_delete( T, rp->rf );
      T->job->reps[B_SUBSUMED].i_report++;
      T->job->reps[B1_SUBSUMED].i_report++;
      T->job->reps[PREREFS].i_report--;
    }
  }
  delete_duds(T);
  T->comvec[c->vpos] &= ~(1 << x);
}




intlist get_all_ils( int cells[], int vals[], boolean valency[], int x, trs T )
{
  if ( cells[x] < 0 ) return 0;
  return get_il( T, valency[x], cells[x], vals[x],
		 get_all_ils(cells, vals, valency, x+1, T) );
}




boolean subsumed( ref the_ref, trs T )
{
  reflist rp;
  intlist ip;

  FORR( the_ref, ip ) if ( ip->p )
    for ( rp = T->vector[ip->v].cvals[ip->w].cvrefs; rp; rp = rp->nextref ) {
      if ( rp->rf->cellvals->v != ip->v) break;
      if ( (rp->rf->cardinality < the_ref->cardinality ||
	    (ip == the_ref->cellvals &&
	     rp->rf->cardinality == the_ref->cardinality))
	   && subsumes(rp->rf, the_ref) ) return true;
    }
  return false;
}


boolean subsumes( ref little_ref, ref big_ref )
{
  intlist il, ib;

  ib = big_ref->cellvals;
  FORR( little_ref, il ) {
    while ( ib && (il->v != ib->v) ) ib = ib->p;
    if ( !ib ) return false;
    if ( ib->positive && !il->positive ) return false;
    if ( !ib->positive && il->positive ) {
      if ( il->w == ib->w ) return false;
    }
    else if ( il->w != ib->w ) return false;
  }
  return true;
}


void back_sub( trs T, ref the_ref )
{
  reflist rp;
  intlist ip;
  ip = the_ref->cellvals;
  for ( rp = T->vector[ip->v].cvals[ip->w].cvrefs; rp; rp = rp->nextref )
    if ( rp->rf->cardinality > the_ref->cardinality )
      if ( subsumes(the_ref, rp->rf) ) {
	mark_delete( T, rp->rf );
	T->job->reps[B_SUBSUMED].i_report++;
	T->job->reps[PREREFS].i_report--;
      }
  delete_duds( T );
}




void Ref( int x, trs T )
{
  intlist it;

  if ( T->vector[x].used ) return;
  if ( !(T->newref) ) T->newref = get_ref( T );
  it = get_il( T, false, x, T->comvec[x], T->newref->cellvals );
  T->newref->cellvals = it;
  T->newref->cardinality++;
  T->vector[x].used = true;
}



void no_ref( trs T )
{
  T->noref = true;
}



/*
*	This is done when a forkpoint exists and has a value, as do zero or
*	more less significant cells which next have to be outvalued.
*
*	If there is currently an active cut it must be deleted, as this one
*	subsumes it. Failure to do this may produce looping.
*/

void record_cut( trs T )
{
  if ( T->cutptr && T->cutptr->active ) delete_cut( T );
  record_part_cut( T->comvec[T->forkpoint], T );
  T->cut_flag = RECORDED;
  T->ts = false;
  T->novect = true;
}



/*
*	Record the cuts for values from x up.
*/

void record_part_cut( int x, trs T )
{
  int i;
  cutt newcut;

  if ( x > T->maxval ) return;
  record_part_cut( x+1, T );
  if ( T->vector[T->forkpoint].cvals[x].blocks ) return;
  newcut = (cutt) malloc(sizeof(CUTT));
  newcut->lastone = T->cutptr;
  T->cutptr = newcut;

  if ( x == T->comvec[T->forkpoint] )
    for ( i = 0; i <= T->job->vlength; i++ ) {
      T->cutptr->comvec[i] = T->comvec[i];
      T->cutptr->inorder[i] = T->inorder[i];
    }
  else {
    for ( i = 0; T->inorder[i] != T->forkpoint; i++ ) {
      T->cutptr->comvec[i] = T->comvec[i];
      T->cutptr->inorder[i] = T->inorder[i];
    }
    do {
      T->cutptr->comvec[i] = 0;
      T->cutptr->inorder[i] = -1;
    }
    while ( ++i <= T->job->vlength );
  }
  T->cutptr->forkpoint = T->forkpoint;
  T->cutptr->active = false;
  T->cutptr->cutval = x;
}




/*
*	This is the action when the last cell in inorder to have a value 
*	is the forkpoint, and there is a cut being recorded.
*/

void complete_cut( int ex, trs T )
{
  int i, j;
  cutt cp;

  for ( cp = T->cutptr; cp && cp->forkpoint == ex; cp = cp->lastone ) {
    for ( i = 0; i < T->job->vlength; i++ ) {
      cp->possvals[i] = 0;
      for ( j = 0; j <= T->maxval; j++ )
	if ( !T->vector[i].cvals[j].blocks )
	  cp->possvals[i] |= (1 << j);
    }
    cp->possvals[ex] = (1 << cp->cutval);
  }
  T->cut_flag = NOCUT;
}



boolean	prepared( trs T )
{
  int i, j;
  logical u = 0;
  cell c;

  T->novect = false;
  if ( T->cutptr ) {
    for ( i = 0; i < V_LENGTH; i++ )
      T->comvec[i] = T->cutptr->possvals[i];
  }
  else {
    for ( i = 0; i < SZ; i++ ) u |= (1 << i);
    for ( i = 0; i < V_LENGTH; i++ )
      T->comvec[i] = (i < T->job->vlength)? u: 0;
  }

  if ( T->job->Onerefs ) {
    (*(T->job->Onerefs))(T->comvec,T);
    T->job->reps[SETPOSSES].i_report++;
  }
  T->maxval = 0;
  for ( i = 0; i < T->job->vlength; i++ ) {
    if ( !T->comvec[i] ) {
      T->novect = true;
      return false;
    }
    for ( j = T->maxval+1; j < SZ; j++ )
      if ( T->comvec[i] & (1 << j) )
	T->maxval = j;
  }

  for ( i = 0; i < T->job->vlength; i++ ) {
    c = T->vector + i;
    c->vpos = i;
    for ( j = 0; j <= SZ; j++ ) {
      c->cvals[j].vpos = i;
      c->cvals[j].cvrefs = 0;
    }
    c->inject = 0;
  }
  for ( i = 0; i <= T->job->vlength; i++ ) {
    T->inorder[i] = -1;
    T->coinorder[i] = -1;
  }
  for ( i = 0; i < T->job->batches; i++ ) T->stak[i] = 0;
  for ( i = 0; i < T->stakmax; i++ ) T->il_stak[i] = 0;
  for ( i = 0; i < T->stakmax+T->job->batches; i++ ) T->rl_stak[i] = 0;
  T->stak_ptr = 0;
  T->il_free = 0;
  T->rl_free = 0;
  more_refs( T );
  more_ils( T );
  more_rls( T );
  for ( i = 0; i < SURJ_MAX; i++ ) {
    T->surjection[i].valset = 0;
    T->surjection[i].cellset = 0;
    for ( j = 0; j < SZ; j++ )
      T->surjection[i].last_seen[j] = 0;
  }

  setinx( T );

  T->firstchange = -1;
  T->forkpoint = -1;
  T->this_cell = -1;
  T->unfixt = T->job->vlength-1;
  T->ts = false;
  T->susguy = 0;
  T->newref = 0;
  T->duds = 0;
  return true;
}



void setinx( trs T )
{
  int i, j;
  cell c;

  for ( i = 0; i < T->job->vlength; i++ ) {
    c = T->vector+i;
    c->poss = 0;
    for ( j = 0; j < SZ; j++ ) {
      if ( T->comvec[i] & (1 << j) ) {
	c->poss++;
	c->cvals[j].blocks = 0;
      }
      else c->cvals[j].blocks = 1;
      c->cvals[j].cvrefs = 0;
      c->cvals[j].posrefs = 0;
      c->cvals[j].back = 0;
    }
    c->used = false;
    c->has_value = false;
  }
}





boolean	pre_test( trs T )
{
  int i, j;

  if ( !T->job->Smallrefs ) return true;
  T->job->reps[DOSMALLS].i_report++;
  for ( i = 0; i < T->job->vlength; i++ ) {
    T->comvec[i] = 0;
    for ( j = 0; j <= T->maxval; j++ )
      if ( !T->vector[i].cvals[j].blocks )
	T->comvec[i] |= (1 << j);
  }
  if ( !T->job->Smallrefs(T->comvec, T) ) return false;
  if ( T->novect ) return false;

  T->maxval = 0;
  for ( i = 0; i < T->job->vlength; i++ ) {
    if ( !(T->vector[i].poss) ) {
      T->novect = true;
      return false;
    }
    for ( j = T->maxval+1; j < SZ; j++ )
      if ( T->comvec[i] & (1 << j) ) T->maxval = j;
    T->comvec[i] = 0;
  }
  T->max_batches = batches_used(T) + T->job->extra_batches;
  if ( T->max_batches >= T->job->batches )
    T->max_batches = T->job->batches - 1;
  return true;
}




boolean absfixt( int x )
{
  /* Only a stub at present */
  return false;
}





void surjective( logical vec[], logical vset, trs T )
{
  int i, j;

  for ( i = 0; T->surjection[i].valset; i++ )
    if ( i == SURJ_MAX ) skipout("Too many surjections",SURJES);

  T->surjection[i].valset = vset;
  for ( j = 0; j < T->job->vlength; j++ ) if ( vec[j] )
    T->surjection[i].cellset = get_il(T,false,j,-1,T->surjection[i].cellset);
  for ( j = 0; j < SZ; j++ )
    T->surjection[i].last_seen[j] = T->surjection[i].cellset->v;
}




void injective( logical vec[], int x, trs T )
{
  int i;
  ref r;
  reflist rp;

  r = get_ref( T );
  r->cardinality = x;
  r->notop = false;

  for ( i = 0; i < T->job->vlength; i++ ) if ( vec[i] ) {
    r->cellvals = get_il( T, false, i, -1, r->cellvals );
    rp = get_rl( T );
    rp->rf = r;
    rp->nextref = T->vector[i].inject;
    T->vector[i].inject = rp;
  }
}



void co_priority( int x, int y, trs T )
{
  T->priority[x] = y;
}




void job_done( trin t, char *s )
{
  if ( t->report_breaks )
    fprintf(stderr,"\nStopped by %s\n", s);
  t->done = true;
}




int timestamp()
{
  struct tms time_buffer;

  times(&time_buffer);
  return time_buffer.tms_utime;
}


void dump_cuts(trs T)
{
  cutt ct;

  printf("\nSearching.");
  if ( T->cutptr ) {
    printf("  Cut stack = ");
    for ( ct = T->cutptr; ct; ct = ct->lastone )
      printf(" %d.%d ", ct->forkpoint, ct->cutval );
  }
  puts("");
  fflush(stdout);
}



void skipout( char s[], int exitcode )
{
  fprintf(stderr,"\n\n\n Aborting on detection of an error");
  fprintf(stderr,(*s? ":": "."));
  fprintf(stderr,"\n\n %s.\n\n", s);
  exit( exitcode );
}


void test_backs(trs T)
{
  int i, j;
  cellvalue cv;

  for ( i = 0; i < T->job->vlength; i++ )
    for ( j = 0; j <= T->maxval; j++ ) {
      cv = &(T->vector[i].cvals[j]);
      /*
       * if ( cv->blocks && !cv->back )
       * skipout("Block without back",SKIP);
       */
      if ( cv->back && !cv->blocks )
	skipout("Back without block",SKIP);
    }
}
