#include "RM.h"
#include "hmi.h"

int main(int argc, char *argv[])
{
  if (argc != 2)
    Abort("synopsis: embedding <file of algebras>",-1);
  relatemats(embedding, argv[1], "subalgebra");

  return 0;
}
