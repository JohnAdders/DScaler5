#include <math.h>
#include "dsputil.h"

static double coslu[8][8];

void idct_ref(DCTELEM * block)
{
  int x,y,u,v;
  double tmp, tmp2;
  double res[8][8];  
  DCTELEM (*blk)[8] =  (DCTELEM(*)[8])block;
  static int inited=0;

  emms_c();

  if (!inited)
   {
    int a,b;
    double tmp;
    for(a=0;a<8;a++)
     for(b=0;b<8;b++) 
      {
       tmp = cos((double)((a+a+1)*b) * (3.14159265358979323846 / 16.0));
       if(b==0) tmp /= sqrt(2.0);
       coslu[a][b] = tmp * 0.5;
      }
   }   

  for (y=0; y<8; y++) {
    for (x=0; x<8; x++) {
      tmp = 0.0;
      for (v=0; v<8; v++) {
        tmp2 = 0.0;
        for (u=0; u<8; u++) {
          tmp2 += (double) blk[v][u] * coslu[x][u];
        }
        tmp += coslu[y][v] * tmp2;
      }
      res[y][x] = tmp;
    }
  }
  
  for (v=0; v<8; v++) {
    for (u=0; u<8; u++) {
      tmp = res[v][u];
      if (tmp < 0.0) {
        x = - ((int) (0.5 - tmp));
      } else {
        x = (int) (tmp + 0.5);
      }
      blk[v][u] = (DCTELEM) x;
    }
  }
}
