#include "RM.h"
#include "hmi.h"

int main(int argc, char *argv[])
{
  if (argc != 2)
    Abort("synopsis: homomorphic <file of algebras>",-1);
  relatemats(homo, argv[1], "Homomorphism");

  return 0;
}
