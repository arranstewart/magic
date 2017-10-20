
#include "RM.h"


static int tot_got;		/* Total of elements generated	*/
static int got[SZ];		/* Elements generated so far	*/



int main( argc, argv )
int argc;
char *argv[];
{
  int option;
  PRINTMODE p = UGLY;
  int g2();
  int getopt();

  while ((option = getopt (argc, argv, "upt")) != -1)
    switch( option ) {
    case 'p': p = PRETTY;	break;
    case 't': p = TeX;
    }
  selectmats( g2, p );
  return 0;
}




int g2(m)
MATRIX *m;
{
  int i;
  int generator;
  int position = 0;
  void try();

  FORALL(m,generator) {
    FORALL(m,i) got[i] = 0;
    if (m->tee_exists) {
      got[m->tee] = 1;
      tot_got = 1;
    }
    else tot_got = 0;
    FORALLCON(m,i)
      if ( m->adicity[i] == 0 ) try( m, m->nulladic[i] );
    try( m, generator );
    if ( tot_got > m->siz ) {
      m->my_values[position] = generator;
      if ( position++ )
	strcpy( m->my_string, "Possible generators: ");
      else strcpy( m->my_string, "Generator: ");
    }
  }
  return( position > 0 );
}





void try(m,x)
MATRIX *m;
int x;
{
  int i, j;

  if ( got[x] ) return;

  got[x] = 1;
  tot_got++;
  if ( m->fragment[NEG] ) try( m, m->neg[x] );
  if ( m->fragment[BOX] ) try( m, m->box[x] );
  FORALLCON(m,i)
    if ( m->adicity[i] == 1 ) try( m, m->monadic[i][x] );
  FORALL(m,i) if ( got[i] ) {
    try( m, m->C[i][x] );
    try( m, m->C[x][i] );
    if ( m->fragment[LAT] ) {
      try( m, m->K[i][x] );
      try( m, m->A[i][x] );
    }
    if ( m->fragment[FUS] ) {
      try( m, m->fus[i][x] );
      try( m, m->fus[x][i] );
    }
    FORALLCON(m,j) {
      if ( m->adicity[j] == 2 )
	try( m, m->dyadic[j][i][x] );
      try( m, m->dyadic[j][x][i] );
    }
  }
}
