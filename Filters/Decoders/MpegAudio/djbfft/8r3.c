#include "pre8.c"

void r2048(register real *a)
{
  rpassbig(a,d2048,256);
  r1024(a);
  c512((complex *)(a + 1024));
}
