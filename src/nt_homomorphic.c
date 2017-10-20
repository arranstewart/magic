#include "RM.h"
#include "hmi.h"

int main(int argc, char *argv[])
{
  if (argc != 2)
    Abort("synopsis: nt_homomorphic <file of algebras>",-1);
  relatemats(nt_homo, argv[1], "NontrivialHomomorphism");

  return 0;
}
