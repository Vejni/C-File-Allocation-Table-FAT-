#include "filesys.h"
#include <stdio.h>
#include <string.h>

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

  /* Using change dir to get into the directory */
  /* Implemented this functionality since then, to see it go to A */
  mychdir("/myfirstdir/myseconddir");
  MyFILE * f = myfopen("testfile.txt","w");
  myfclose(f);

  listdir = mylistdir("/myfirstdir/myseconddir");
  printf("Contents: %s\n",listdir );

  filename = "virtualdiskB3_B1_b";
  writedisk(filename);

  printf("B3-B1 ends here\n" );
}

void A(){
  MyFILE * f;
  char * listdir;
  char * filename;
  char * alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXWZ";
  char * text;

  printf("A5-A1 starts here\n" );

  format();

  printf("\nCreating directories:" );
  mymkdir("/firstdir/seconddir");


  printf("Creating first file: \n" );
  f = myfopen("/firstdir/seconddir/testfile1.txt","w");
  for (size_t i = 0; i < 2*BLOCKSIZE; i++) {
    myfputc(alphabet[i%26],f);
  }
  myfclose(f);

  /* At this point we are already there, but ok */
  printf("Changind directories with absolute path : \n" );
  mychdir("/firstdir/seconddir");

  /* listdir with absolute path */
  printf("Listing with using absolute path : \n" );
  listdir = mylistdir("/firstdir/seconddir");
  printf("Contents of /firstdir/seconddir: %s\n\n",listdir );

  printf("Listing with using . : \n" );
  listdir = mylistdir(".");
  printf("Contents of /firstdir/seconddir: %s\n\n",listdir );

  printf("Creating second file: \n" );
  f = myfopen("testfile2.txt","w");
  text = "Look ma no hands";
  for (size_t i = 0; i < strlen(text); i++) {
    myfputc(text[i],f);
  }
  myfclose(f);

  printf("Creating thirddir using relative path: \n" );
  mymkdir("thirddir");

  printf("Changing directories with using .. : \n" );
  mychdir("..");

  printf("Creating third file with relative path: \n" );
  f = myfopen("thirddir/testfile3.txt", "w");
  text = "interesting text";
  for (size_t i = 0; i < strlen(text); i++) {
    myfputc(text[i],f);
  }
  myfclose(f);

  filename = "virtualdiskA5_A1_a";
  writedisk(filename);

  printf("Print FAT: \n" );
  printFAT();
  printf("A5-A1 ends here\n" );
}

int main(int argc, char const *argv[]) {
  A();


  return 0;
}
