/*				MaGIC1.c	V2.1
*
*	This is the serial version of MaGIC intended eventually to be 
*	compatible with xmagic version 2.1.  It requires such 
*	structures as JOB to be defined in MAGIC.h.
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




#define TOPFILE
#include "MaGIC.h"


static boolean interrupted;



int main(argc,argv)
int argc;
char *argv[];
{
  int i, option;
  boolean batch;
  char batchfile[80];
  extern char *optarg;
  void timesup(), intrupt();
  int getopt();

  noclear = false;
  batch = false;
  filing = false;
  xdialog = false;

  while ((option = getopt (argc, argv, "b:tx#:")) != -1)
    switch( option ) {
    case 'x':
      xdialog = true;
    case 't':
      noclear = true;
      break;
    case 'b':
      batch = true;
      i = 0;
      while ((batchfile[i] = optarg[i])) i++;
    }

  theJob = (JOB*) malloc(sizeof(JOB));
  signal( SIGALRM, timesup );
  interrupted = false;
  signal( SIGINT, intrupt );

  strings_initial();
  tests_initial();
  logics_initial();
  job_defaults( batch );
  perm_initial();

  while ((i = dialog(batch,batchfile)))
    if ( i > 0 ) {
      if ( !batch ) {
	printf("\n Searching.....\n");
	fflush(stdout);
      }
      job_start();
      subf_set();
      interrupted = false;
      alarm(theJob->maxtime);

      if ( newsiz() )
	do if ( pre_set() ) {
	  setperm();
	  transref(&(tr_par));
	}
	while ( newdes() );

      job_stop(batch);
      alarm(0);
      if ( batch ) exit(0);
    }
  if ( !noclear ) {
#ifdef __CYGWIN__
    puts( "\033[2J" );
#else
    system("clear");
#endif
  }
  return 0;
}




/*
*	Action on signal from alarm.
*/

void timesup()
{
  tr_par.done = true;
}



/*
*	Action on signal from ^C or whatever SIGINT may be.
*/

void intrupt()
{
  tr_par.done = true;
  if ( interrupted++ ) exit(3);
}
