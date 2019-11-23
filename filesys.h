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

/* FAT is just an array of shorts */
typedef unsigned char Byte ;
typedef short fatentry_t ;
typedef fatentry_t fatblock_t [ FATENTRYCOUNT ] ;

/* Directories and files have a direntry structure */
typedef struct direntry {
   struct dirblock * dirblock_ptr; /* Points to the relevant dirblock instance, set to NULL for files, replaces isdir too */
   Byte        unused ;
   int         filelength ;
   fatentry_t  firstblock ; /* Start in FAT */
   char   name [MAXNAME] ;
} direntry_t ;

/* Only directories have this */
typedef struct dirblock {
   struct dirblock * parent;
   struct direntry * entry_ptr; /* Pointer to relevant entry instance */
   int childrenNo;
   direntry_t entrylist [ DIRENTRYCOUNT ] ; /* Number of directory entries is capped at 3 */
} dirblock_t ;

/* Data is just an array of bytes */
typedef Byte datablock_t [ BLOCKSIZE ] ;

/* A block can be data,dir or fat */
typedef union block {
   datablock_t data ;
   dirblock_t  dir  ;
   fatblock_t  fat  ;
} diskblock_t ;

/* Virtualdisk is an array of block structures */
extern diskblock_t virtualDisk [ MAXBLOCKS ] ;


/* When a file is opened on this disk, a file handle has to be
   created in the opening program */
typedef struct filedescriptor {
   char        mode[3]    ; /* Some modes are 2 letters, EOS char */
   fatentry_t  blockno    ; /* FAT index */
   int         pos        ;
   diskblock_t buffer     ;
   int         filelength ;
   int         write      ; /* File currently open for writing */
} MyFILE ;


/* Function prototypes */
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
void myremove ( const char * path );
const char * mychtofile (const char * filename);

#endif
