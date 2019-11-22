#ifndef FILESYS_H
#define FILESYS_H

#include <time.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MAXBLOCKS     1024
#define BLOCKSIZE     1024
#define FATENTRYCOUNT (BLOCKSIZE / sizeof(fatentry_t))
#define DIRENTRYCOUNT ((BLOCKSIZE - (2*sizeof(int)) ) / sizeof(direntry_t))
#define MAXNAME       256
#define MAXPATHLENGTH 1024
#define FATBLOCKSNEEDED (MAXBLOCKS / FATENTRYCOUNT )

#define UNUSED        -1
#define ENDOFCHAIN     0

typedef unsigned char Byte ;
typedef short fatentry_t ;
typedef fatentry_t fatblock_t [ FATENTRYCOUNT ] ;

typedef struct direntry {
   //int         entrylength ;   // records length of this entry (can be used with names of variables length)
   struct dirblock * dirblock_ptr;
   // struct dirblock * parent_dirblock_ptr;
   Byte        unused ;
   time_t      modtime ;
   int         filelength ;
   fatentry_t  firstblock ;
   char   name [MAXNAME] ;
} direntry_t ;

typedef struct dirblock {
   //char name[MAXNAME];
   struct dirblock * parent;
   struct direntry * entry_ptr;
   //struct dirblock ** children;
   int childrenNo;
   direntry_t entrylist [ DIRENTRYCOUNT ] ; // the first two integer are marker and endpos
   //time_t modTime;
   //fatentry_t fBlock;
} dirblock_t ;

typedef Byte datablock_t [ BLOCKSIZE ] ;

typedef union block {
   datablock_t data ;
   dirblock_t  dir  ;
   fatblock_t  fat  ;
} diskblock_t ;

extern diskblock_t virtualDisk [ MAXBLOCKS ] ;


// when a file is opened on this disk, a file handle has to be
// created in the opening program
typedef struct filedescriptor {
   char        mode[3]    ; /* Some modes are 2 letters, EOS char */
   fatentry_t  blockno    ;
   int         pos        ;
   diskblock_t buffer     ;
   int         filelength ;
   int         write      ;
} MyFILE ;



void format() ;
void writedisk ( const char * filename );
void printBlock(int blockIndex);
void copyFAT();
void printFAT();
int findUnused();
MyFILE * myfopen ( const char * filename, const char * mode );
void myfputc ( int b, MyFILE * stream );
void myfclose ( MyFILE * stream );
int myfgetc ( MyFILE * stream );
int findEntry(const char * entry);
void mymkdir ( const char * path );
void createDirectory(const char * name, int index);
char * mylistdir(const char * path);
void mylistall();
void mylistallinner(dirblock_t * dir);
void mychdir ( const char * path );
#endif

/*
#define NUM_TYPES (sizeof types / sizeof types[0])
static* int types[] = {
    1,
    2,
    3,
    4 };
*/
