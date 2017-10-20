#include "RM.h"
#include "hmi.h"

main(int argc, char *argv[])
{
  if (argc != 2)
    Abort("synopsis: embedding <file of algebras>",-1);
  relatemats(epimorphic_image, argv[1], "MapsOnto");
}
