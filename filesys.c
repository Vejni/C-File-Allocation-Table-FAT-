#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "filesys.h"


diskblock_t  virtualDisk [MAXBLOCKS];
fatentry_t   FAT [MAXBLOCKS];
fatentry_t   rootDirIndex = FATBLOCKSNEEDED + 1;
dirblock_t * rootDirBlock_ptr = NULL;
//direntry_t * currentDirEntry = NULL;
fatentry_t   currentDirIndex = 0;
dirblock_t * currentDirBlock_ptr = NULL;
const char parser[2] = "/";


void writedisk ( const char * filename ){
   // printf ( "writedisk> virtualdisk[0] = %s\n", virtualDisk[0].data ) ;
   FILE * dest = fopen( filename, "w" ) ;
   if ( fwrite ( virtualDisk, sizeof(virtualDisk), 1, dest ) < 0 )
      fprintf ( stderr, "write virtual disk to disk failed\n" ) ;
   //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
   fclose(dest) ;

}

void readdisk ( const char * filename ){
   FILE * dest = fopen( filename, "r" ) ;
   if ( fread ( virtualDisk, sizeof(virtualDisk), 1, dest ) < 0 )
      fprintf ( stderr, "write virtual disk to disk failed\n" ) ;
   //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
      fclose(dest) ;
}

void writeblock ( diskblock_t * block, int block_address ){
   //printf ( "writeblock> block %d = %s\n", block_address, block->data ) ;
   memmove ( virtualDisk[block_address].data, block->data, BLOCKSIZE ) ;
   //printf ( "writeblock> virtualdisk[%d] = %s / %d\n", block_address, virtualDisk[block_address].data, (int)virtualDisk[block_address].data ) ;
}

/* Format disk and FAT */
void format (){
   diskblock_t block ;

   /* Set root and current dir */
   rootDirBlock_ptr = (dirblock_t *) &virtualDisk[rootDirIndex].dir;
   currentDirIndex = rootDirIndex;
   //currentDirEntry = &rootDir_ptr->entrylist[currentDirIndex];
   currentDirBlock_ptr = rootDirBlock_ptr;

   /*Initialize Block 0 */
   for ( int i = 0; i < BLOCKSIZE; i++) block.data[i] = '\0';
   strcpy(block.data,"CS3026 Operating Systems Assessment");
   writeblock(&block,0);

   /* Initialize FAT */
   FAT[0] = ENDOFCHAIN;

   int i = 1;
   while (i < FATBLOCKSNEEDED){
     FAT[i] = i+1;
     i++;
   }
   FAT[i] = ENDOFCHAIN;
   i++;

   // Root
   FAT[i] = ENDOFCHAIN;
   i++;

   while (i < MAXBLOCKS){
     FAT[i] = UNUSED;
     i++;
   }

   copyFAT();

   /* Initialize Root */
   rootDirBlock_ptr->parent = NULL; /* No parent of root */
   rootDirBlock_ptr->entry_ptr = NULL; /* No entry for root */

   /* Set all direntries to available */
   for (int i = 0; i < DIRENTRYCOUNT; i++) {
     rootDirBlock_ptr->entrylist[i].unused = TRUE;
   }

   /* Print */
   printBlock(0);

}

/* Writes FAT to disk */
void copyFAT(){
  for ( int i = 0; i < FATBLOCKSNEEDED; i++){
    for ( int j = 0; j < FATENTRYCOUNT; j++){
      virtualDisk[i+1].fat[j] = FAT[j+i*FATENTRYCOUNT];
    }
  }
}

/* Open file, currently possible: r, w */
/* Create file descriptor and file entry*/
MyFILE * myfopen ( const char * filename, const char * mode ){
  MyFILE * file_ptr = (MyFILE *) malloc(sizeof(MyFILE));
  dirblock_t * Dir_ptr;
  direntry_t * file_entry;
  int freeblock;

  /* Write */
  if (strcmp(mode,"w") == 0){
    freeblock = findUnused();

    /* Check if any block is available */
    if (freeblock == FALSE){
      printf("Could not open %s, for %s\n",filename,mode);
      return NULL;
    }

    /* Prepare memory */
    memset(file_ptr, '\0', sizeof(MyFILE));


    /* File descriptor */
    file_ptr->blockno = freeblock;
    file_ptr->buffer = virtualDisk[freeblock];
    file_ptr->filelength = 1; /* EOF included */
    file_ptr->write = TRUE;

    /* Create file entry and append to current folder */
    Dir_ptr = (dirblock_t *) &virtualDisk[currentDirIndex].dir;
    file_entry = (direntry_t *) Dir_ptr->entrylist;
    strcpy(file_entry->name, filename);
    file_entry->dirblock_ptr = NULL;
    file_entry->filelength = 0;
    file_entry->firstblock = freeblock;
    file_entry->unused = FALSE;
    file_entry->modtime = 0; // later

    /* FAT */
    FAT[freeblock] = ENDOFCHAIN;
    copyFAT();
  }
  else{
    /* Read */
    int entry = findEntry(filename);
    file_ptr->write = FALSE;
    file_ptr->blockno = virtualDisk[currentDirIndex].dir.entrylist[entry].firstblock;
    memcpy(&file_ptr->buffer, &virtualDisk[file_ptr->blockno], BLOCKSIZE);
  }

  file_ptr->pos = 0;
  strcpy(file_ptr->mode, mode);

  // Write FAT to disk, also set file open
  return file_ptr;
}

/* Closes file, puts EOF and writes to block */
void myfclose ( MyFILE * stream ){
  //dirblock_t * rootDir_ptr = (dirblock_t *) &virtualDisk[rootDirIndex].dir;
  //direntry_t * entry_ptr = rootDir_ptr->entrylist[0];
  if (stream->write == TRUE){
    myfputc(EOF, stream);
    //entry_ptr->filelength = stream->filelength;
    writeblock(&stream->buffer, stream->blockno);
    stream->write = 0;
  }
  free(stream);
  return;
}

/* Put character into file */
void myfputc ( int b, MyFILE * stream ){
  /* Check the mode of the file */
  if (strcmp(stream->mode, "r") == 0){
    printf("Cannot write in read-only.\n");
    return;
  }

  if (stream->write == FALSE){
    printf("File not open for writing.\n");
    return;
  }

  /* Check if it is within block */
  if (stream->pos >= BLOCKSIZE){
    /* Write to disk as block is full */
    writeblock(&stream->buffer,stream->blockno);

    int unused_index = findUnused();
    if (unused_index == FALSE){
      printf("No more free space available.\n");
      return;
    }

    /* Initialise values if there is a free one */
    FAT[stream->blockno] = unused_index;
    FAT[unused_index] = ENDOFCHAIN;
    stream->blockno = unused_index;
    stream->pos = 0;
    copyFAT();
  }

  /* Write byte */
  stream->buffer.data[stream->pos] = b;
  stream->pos++;
  stream->filelength++;
  // Write to disk if mode is ?
  if (strcmp(stream->mode, "w") == 0){
    writeblock(&stream->buffer,stream->blockno);
  }

  return;
}

/* Reads a character from file */
// Doublecheck
int myfgetc ( MyFILE * stream ){
  if (stream->pos >= BLOCKSIZE){
    if (FAT[stream->blockno] == ENDOFCHAIN){
      return EOF;
    }
    stream->blockno = FAT[stream->blockno];
    memcpy(&stream->buffer, &virtualDisk[stream->blockno], BLOCKSIZE);
    stream->pos = 1;
    return stream->buffer.data[stream->pos-1];
  }
  if(stream->buffer.data[stream->pos] == EOF){
    return EOF;
  }
  ++stream->pos;
  return stream->buffer.data[stream->pos-1];
}

/* Find unused FAT entry, return 0 if full */
int findUnused(){
  int i = 0;
  while (i < MAXBLOCKS){
    if (FAT[i] == UNUSED) return i;
    else i++;
  }
  return FALSE;
}

/* Returns index of entry within entrylist, returns -1 if fails */
int findEntry(const char * entry){
  while(TRUE){
    for(int i = 0; i < DIRENTRYCOUNT; i++){
      if (strcmp(virtualDisk[currentDirIndex].dir.entrylist[i].name, entry) == 0)
        return i;
    }
    if(FAT[currentDirIndex] == ENDOFCHAIN) break;
    currentDirIndex = FAT[currentDirIndex];
  }
  return -1;
}

/* Directories created for the whole path structure, callse createDirectory */
void mymkdir ( const char * path ){
  char * token;
  char * pathCopy = malloc(strlen(path));
  strcpy(pathCopy, path);

  token = strtok(pathCopy,parser);
  printf("\n");
  /* If absolute path, change back to root */
  if (pathCopy[0] == '/'){
    currentDirIndex = rootDirIndex;
    currentDirBlock_ptr = rootDirBlock_ptr;
  }

  while (token != NULL){
    int entryIndex = findEntry(token);

    if (entryIndex > -1){
      /* Directory found */
      currentDirIndex = virtualDisk[currentDirIndex].dir.entrylist[entryIndex].firstblock;
    }
    else{
      /* Not found */
      if (currentDirBlock_ptr == rootDirBlock_ptr) printf("Currently in: root. Creating directory %s\n", token);
      else printf("Currently in: %s. Creating directory %s\n",currentDirBlock_ptr->entry_ptr->name, token);
      int unused_index = findUnused();
      createDirectory(token, unused_index);
    }
    token = strtok(NULL, parser);
  }
  free(pathCopy);

  printf("\n");
}

/* Initialising the directory */
void createDirectory(const char * name, int index){
  int i = 0;
  while (virtualDisk[currentDirIndex].dir.entrylist[i].unused != TRUE){
    if (i <= DIRENTRYCOUNT) i++;
    else{
      printf("Cannot create -%s- as there is no more space in this directory.\n", name);
      return;
    }
  }

  diskblock_t * block = (diskblock_t *) malloc(sizeof(diskblock_t));
  direntry_t * free_entry = &virtualDisk[currentDirIndex].dir.entrylist[i];
  free_entry->unused = FALSE;
  free_entry->dirblock_ptr = &block->dir;
  free_entry->modtime = 0;
  free_entry->firstblock = index;
  memset(free_entry->name, '\0', MAXNAME);
  strcpy(free_entry->name, name);

  //currentDirEntry = free_entry;

  block->dir.parent = currentDirBlock_ptr;
  block->dir.childrenNo = 0;
  block->dir.entry_ptr = free_entry;

  /* Set all direntries to available */
  for (int i = 0; i < DIRENTRYCOUNT; i++) {
    block->dir.entrylist[i].unused = TRUE;
  }

  //currentDirEntry = &block.dir.entrylist[0];

  currentDirIndex = index;
  //currentDirBlock_ptr->childrenNo++;
  currentDirBlock_ptr = &block->dir;

  writeblock(block, index);


  FAT[index] = ENDOFCHAIN;
  copyFAT();

}

/* Changes dirs, creates path if not found*/
void mychdir ( const char * path ){
  char * token;
  char * pathCopy = malloc(strlen(path));
  strcpy(pathCopy, path);
  int index;
  dirblock_t * nextDirBlock_ptr;

  /* If absolute path, change back to root */
  if (pathCopy[0] == '/'){
    currentDirIndex = rootDirIndex;
    currentDirBlock_ptr = rootDirBlock_ptr;
  }

  /* get the first token */
  token = strtok(pathCopy, parser);

  /* walk through other tokens */
  while( token != NULL ) {
     /* Change dir to token */
     index = findEntry(token);

     if (index > -1){
       nextDirBlock_ptr = virtualDisk[currentDirIndex].dir.entrylist[index].dirblock_ptr;
       if (currentDirBlock_ptr == rootDirBlock_ptr) printf( "Currently in: root. Changing diretory to: %s\n",token );
       else printf( "Currently in: %s. Changing diretory to: %s\n",currentDirBlock_ptr->entry_ptr->name ,nextDirBlock_ptr->entry_ptr->name );


       currentDirBlock_ptr = nextDirBlock_ptr;
       currentDirIndex = currentDirBlock_ptr->entry_ptr->firstblock;
     }
     else{
       printf( "No %s directory found, creating now...\n", token );
       int unused_index = findUnused();
       if (unused_index != FALSE) createDirectory(token,unused_index);
     }
     /* Get next token */
     token = strtok(NULL, parser);
  }
  free(pathCopy);
  printf("\n");
}

/* List contents of the directory, changes to it as well if it is deeper */
char * mylistdir(const char * path){
  /* Changin directories first until last one */
  mychdir(path);

  dirblock_t * dir = &virtualDisk[currentDirIndex].dir;

  char buff[MAXNAME*MAXBLOCKS];
  memset(buff, '\0', MAXNAME*MAXBLOCKS);
  for(int i = 0; i < DIRENTRYCOUNT; i++) {
    if (dir->entrylist[i].unused == FALSE){
      strcat(buff, dir->entrylist[i].name);
      strcat(buff, "\t");
    }
  }
  char * ls = malloc(strlen(buff)+1);
  memset(ls, '\0', strlen(buff)+1);
  for(int i=0; i < (strlen(buff)+1); i++) ls[i] = buff[i];

  return ls;
}

void printBlock ( int blockIndex ){
   printf ( "virtualdisk[%d] = %s\n", blockIndex, virtualDisk[blockIndex].data ) ;
}

void printFAT(){
  for ( int i = 0; i < FATBLOCKSNEEDED; i++){
    for ( int j = 0; j < FATENTRYCOUNT; j++){
      if (virtualDisk[i+1].fat[j] != -1){
        printf("virtualDisk[%d].fat[%d]= %d\n", i+1,j, virtualDisk[i+1].fat[j]);
      }
    }
  }
  printf("Rest is -1.\n");
}
