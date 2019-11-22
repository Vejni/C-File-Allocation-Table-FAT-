#include "filesys.h"
#include <stdio.h>

void D(){
  printf("D3-D1 starts here\n" );
  format();

  char * filename = "virtualdiskD3_D1";
  writedisk(filename);
  printFAT();

  printf("D3-D1 ends here\n\n" );
}

void C(){
  printf("C3-C1 starts here\n" );
  format();

  char * filename = "virtualdiskC3_C1";
  char * alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXWZ";
  char * testfilename = "testfile.txt";

  MyFILE * f = myfopen ( testfilename, "w");

  for (size_t i = 0; i < 4*BLOCKSIZE; i++) {
    myfputc(alphabet[i%26],f);
  }

  myfclose(f);

  MyFILE * f2 = myfopen(testfilename, "r");
  FILE *f_real = fopen("testfileC3_C1_copy.txt","w");

  while(1){
    char c = myfgetc(f2);
    if (c == EOF){
      printf(" EOF \n");
      break;
    }
    printf("%c ", c);
    fputc(c,f_real);
  }

  myfclose(f2);
  fclose(f_real);

  writedisk(filename);
  printFAT();

  printf("C3-C1 ends here\n\n" );
}

void B(){
  printf("B3-B1 starts here\n" );
  format();

  mymkdir("/myfirstdir/myseconddir/mythirddir");

  char * listdir = mylistdir("/myfirstdir/myseconddir");
  printf("Contents: %s\n",listdir );

  char * filename = "virtualdiskB3_B1_a";
  writedisk(filename);

  mychdir("/myfirstdir/myseconddir");
  MyFILE * f = myfopen("testfile.txt","w");
  myfclose(f);

  listdir = mylistdir("/myfirstdir/myseconddir");
  printf("Contents: %s\n",listdir );

  filename = "virtualdiskB3_B1_b";
  writedisk(filename);

  printf("B3-B1 ends here\n" );
}

int main(int argc, char const *argv[]) {
  B();


  return 0;
}
