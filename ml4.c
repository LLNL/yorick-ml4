/* ml4.c Yorick plugin
 *
 * C function for matlab 4 IO
 *
 * $Id: ml4.c,v 1.3 2008-11-20 14:34:46 frigaut Exp $
 * Original writen by Stephen Browne, tOSC.
 * Adapted and yorickized by Francois Rigaut, 2005-2007
 * last revision/addition: 2007jun14
 *
 * Copyright (c) 2005, Francois RIGAUT (frigaut@gemini.edu, Gemini
 * Observatory, 670 N A'Ohoku Place, HILO HI-96720).
 *
 * This program is free software; you can redistribute it and/or  modify it
 * under the terms of the GNU General Public License  as  published  by the
 * Free Software Foundation; either version 2 of the License,  or  (at your
 * option) any later version.
 *
 * This program is distributed in the hope  that  it  will  be  useful, but
 * WITHOUT  ANY   WARRANTY;   without   even   the   implied   warranty  of
 * MERCHANTABILITY or  FITNESS  FOR  A  PARTICULAR  PURPOSE.   See  the GNU
 * General Public License for more details (to receive a  copy  of  the GNU
 * General Public License, write to the Free Software Foundation, Inc., 675
 * Mass Ave, Cambridge, MA 02139, USA).
 *
 * $Log: ml4.c,v $
 * Revision 1.3  2008-11-20 14:34:46  frigaut
 * - Included and uploaded changes from Thibaut Paumard making ml4 64 bits safe.
 * - Beware: In 64 bits, longs (8bytes) are saved as int (4bytes)
 *
 * - a few more minor text edits
 * - added ML4_VERSION variable
 *
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "ml4.h"
#include "ydata.h"
#include "yapi.h"
#include "pstdlib.h"

// Macs and SGIs are Big-Endian; PCs are little endian
// returns TRUE if current machine is little endian
extern int IsLittleEndian(void);
extern Array *GrowArray(Array *array, long extra);

/******************************************************************************
  FUNCTION: SwapEndian
  PURPOSE: Swap the byte order of a structure
  EXAMPLE: float F=123.456;; SWAP_FLOAT(F);
******************************************************************************/

#define SWAP_SHORT(Var)  Var = *(short*)         swap((void*)&Var, sizeof(short))
#define SWAP_USHORT(Var) Var = *(unsigned short*)swap((void*)&Var, sizeof(short))
#define SWAP_INT(Var)    Var = *(int*)           swap((void*)&Var, sizeof(int))
#define SWAP_LONG(Var)   Var = *(long*)          swap((void*)&Var, sizeof(long))
#define SWAP_ULONG(Var)  Var = *(unsigned long*) swap((void*)&Var, sizeof(long))
#define SWAP_RGB(Var)    Var = *(int*)           swap((void*)&Var, 3)
#define SWAP_FLOAT(Var)  Var = *(float*)         swap((void*)&Var, sizeof(float))
#define SWAP_DOUBLE(Var) Var = *(double*)        swap((void*)&Var, sizeof(double))


#define MAXFILES (20)

#define DEBUG 0

#define MIN(A,B) (A)<(B) ? (A) : (B)

//static FILE **fd[MAXFILES]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
static FILE *fd[MAXFILES];
static char matfile[MAXFILES][256]={{0}};
static char fullname[256]={0};
static char tempvarname[256] = {0};
static char message[100];

char *matlab_fullname = fullname;
int disable_disk_writing = 0;

static int nfiles=0;
static int saveNormally=1;

//typedef unsigned int ulong;

void writerr(void);
void *swap(void* Addr, int size);
void warn(char *mes);


/***********************************************/

int matout(char *fullname,char *varname,void *data,int nrows,int ncols,char vartype, char *mode, char endianess)
{
  int  size;
  long nelem;
  int  type,namelen;
  int  mrows,mcols,imagf;
  int i;
  FILE *fs;
  
  mrows = nrows;
  mcols = ncols;
  nelem = mrows*mcols;

  switch (vartype) {
  case 'd':   /* 8-byte doubles */
    type=00;
    size=8;
    double *vard=data;
    if (endianess=='B') { for (i=0;i<nelem;i++) SWAP_DOUBLE(vard[i]); };
    break;
  case 'r':    /* 4-byte reals */
    type=10;
    size=4;
    float *varf=data;
    if (endianess=='B') { for (i=0;i<nelem;i++) SWAP_FLOAT(varf[i]); };
    break;
  case 'l':    /* 4-byte int, row-wise */
    type=20;  
    size=4;
    int *varl=data;
    if (endianess=='B') { for (i=0;i<nelem;i++) SWAP_INT(varl[i]); };
    break;
  case 's':    /* 2-byte signed shorts */
    type=30;
    size=2;
    short *vars=data;
    if (endianess=='B') { for (i=0;i<nelem;i++) SWAP_SHORT(vars[i]); };
    break;
  case 'u':    /* 2-byte unsigned shorts */
    type=40;
    size=2;
    short *varus=data;
    if (endianess=='B') { for (i=0;i<nelem;i++) SWAP_SHORT(varus[i]); };
    break;
    //  case 't':    /* 2-byte unsigned shorts saved as "text" */
    //    type=41;
    //    nbytes = mrows*mcols*2;
  case 't':    /* 1-byte unsigned shorts saved as "text" */
    type=51;
    size=1;
    break;
  case 'b':    /* 1-byte unsigned chars */
  case 'c':    /* 1-byte signed chars */
    type=50;
    size=1;
    break;
  default:
    return (-1);
  }

  if (disable_disk_writing) return (0);

  for (i=0 ; i<nfiles ; i++) {
    if (! *matfile[i]) break;
    if (!strcmp(fullname,matfile[i])) break;
  }
  if (i<nfiles) {
    fs = fd[i];
  } else {
    fs = NULL;
  }
  if (fs == NULL) {
    if ((fs=fopen(fullname,mode))==NULL) {
      sprintf(message,"%s could not be opened because of reason %d",fullname,errno);;
      warn(message);
      return (-1);
    }
    strcpy(matfile[i],fullname);
    fd[i] = fs;
    if ((i==nfiles) && (nfiles<MAXFILES-1)) nfiles++;
  }
  
  imagf=0;
  if (endianess=='B') SWAP_INT(type);
  if (fwrite(&type,sizeof(int),1,fs) != 1) {
    writerr();
    return (-1);
  }
  if (endianess=='B') SWAP_INT(mrows);
  if (fwrite(&mrows,sizeof(int),1,fs) != 1) {
    writerr();
    return (-1);
  }
  if (endianess=='B') SWAP_INT(mcols);
  if (fwrite(&mcols,sizeof(int),1,fs) != 1) {
    writerr();
    return (-1);
  }
  if (endianess=='B') SWAP_INT(imagf);
  if (fwrite(&imagf,sizeof(int),1,fs) != 1) {
    writerr();
    return (-1);
  }
    
  namelen=strlen(varname)+1;
  if (endianess=='B') SWAP_INT(namelen);
  if (fwrite(&namelen,sizeof(int),1,fs) != 1) {
    writerr();
    return (-1);
  }
  if (endianess=='B') SWAP_INT(namelen);
  if (fwrite(varname,(unsigned int)namelen,1,fs) != 1) {  
    writerr();
    return (-1);
  }

  if (fwrite(data,size,nelem,fs) != nelem) {
    writerr();
    return (-1);
  }

  matclose(fullname);
  
  return (0);
}

/***********************************************/

int matout_string(char *filename,char *varname,char *string, char *mode)
{
  int err;

  err = matout(filename,varname,string,1,strlen(string),'t',mode,'L');
  // 'L' mode doesn't matter for strings.

  return (err);
}

/***********************************************/

FILE *openmat(char *fullname)
{
  FILE *fs;
  int i;
  
  for (i=0 ; i<nfiles ; i++) {
    if (!strcmp(fullname,matfile[i])) break;
  }
  if (i>=nfiles) {    // could not find a name match
    for (i=0 ; i<nfiles ; i++) {    // look for a blank slot
      if (! *matfile[i]) break;
    }
  }
  if (i<nfiles) {
    fs = fd[i];
  } else {
    fs = NULL;
  }
  if (fs == NULL) {
    if ((fs=fopen(fullname,"r"))==NULL) {
      //      sprintf(message,"%s could not be opened because of reason %d",fullname,errno);
      //      warn(message);
      return NULL;
    }
    strcpy(matfile[i],fullname);
    fd[i] = fs;
    if ((i==nfiles) && (nfiles<MAXFILES-1)) nfiles++;
  }
  
  return (fs);
}
  
/*******************************************************/

void Y_ml4read(int nArgs)

{
  char *filename="";
  char *varname="";
  int leave_open = 0;
  
  if (nArgs==2) {
    filename=YGetString(sp-nArgs+1);
    varname=YGetString(sp-nArgs+2);
    leave_open = 0;
  } else if (nArgs==3) {
    filename=YGetString(sp-nArgs+1);
    varname=YGetString(sp-nArgs+2);
    leave_open=YGetInteger(sp-nArgs+3);
  }

  unsigned long bytes_read;
  int type,namelen;
  unsigned long nElements,nBytesToRead;
  int mrows,mcols,imagf;
  FILE *fs;
  int fileptr;
  int endian = 'L';
  int size=0,i;

  fs = openmat(filename);
  if (fs == NULL) YError(p_strncat("Can't open file ",filename,0));

  if (!matfind(fs,varname,50000)) YError(p_strncat("No Such variable ",varname,0));

  fileptr = ftell(fs);
  if (DEBUG) printf("@ position %d\n",fileptr);
  
  bytes_read = fread(&type,sizeof(int),1,fs);
  if (bytes_read==0) {
    matclose(filename);
    YError("Premature end of file");; // end of file
  }
  fread(&mrows,sizeof(int),1,fs);
  fread(&mcols,sizeof(int),1,fs);
  fread(&imagf,sizeof(int),1,fs);
    
  fread(&namelen,sizeof(int),1,fs);

  if (namelen & 0xffff0000) {
    if (DEBUG) printf("Big endian file\n");
    endian = 'B';
    SWAP_INT(type);
    SWAP_INT(mrows);
    SWAP_INT(mcols);
    SWAP_INT(imagf);
    SWAP_INT(namelen);
  }
  type = type%1000;

  if (DEBUG) printf("rows,cols,namelen= %d %d %d\n",mrows,mcols,namelen);

  if (namelen>255) {
    fseek(fs,fileptr,SEEK_SET);  // leave file ptr at begginning of this variable
    matclose(filename);
    YError("Variable name too long!");
  }

  fread(tempvarname,(unsigned int)namelen,1,fs);
  //  if ((*varname!='*') && strcmp(varname,tempvarname)) {  // error if not same varname
  if (!matchvarname(tempvarname,varname)) {  // error if not same varname
    fseek(fs,fileptr,SEEK_SET);  // leave file ptr at begginning of this variable
    matclose(filename);
    YError(p_strncat("Can't find variable",varname,0));
  }

  nElements = (unsigned)mrows*(unsigned)mcols;
  
  Dimension *tmp=tmpDims;
  tmpDims=0;
  FreeDimension(tmp);
  if (mrows<=1) {
    tmpDims= NewDimension(mcols, 1L, (Dimension *)0);
  } else if (mcols<=1) {
    tmpDims= NewDimension(mrows, 1L, (Dimension *)0);
  } else {
    tmpDims= NewDimension(mrows, 1L, (Dimension *)0);
    tmpDims= NewDimension(mcols, 1L, tmpDims);
  }
  
  if (type==0) {
    // 8-byte doubles
    size = 8;
    Array *a= PushDataBlock(NewArray(&doubleStruct, tmpDims));
    double *data = a->value.d;
    bytes_read = fread((void *)data,size,nElements,fs);
    if (endian=='B') { for (i=0;i<nElements;i++) SWAP_DOUBLE(data[i]); }

  } else if (type==10) {
    // 4-byte reals
    size = 4;
    Array *a= PushDataBlock(NewArray(&floatStruct, tmpDims));
    float *data = a->value.f;
    bytes_read = fread((void *)data,size,nElements,fs);
    if (endian=='B') { for (i=0;i<nElements;i++) SWAP_FLOAT(data[i]); }

  } else if ((type==120) || (type==20)) {
    // 4-byte int
    size = 4;
    Array *a= PushDataBlock(NewArray(&intStruct, tmpDims));
    int *data = a->value.l;
    bytes_read = fread((void *)data,size,nElements,fs);
    if (endian=='B') { for (i=0;i<nElements;i++) SWAP_INT(data[i]); }

  } else if (type==30) {
    // 2-byte signed (30) shorts
    size = 2;
    Array *a= PushDataBlock(NewArray(&shortStruct, tmpDims));
    short *data = a->value.s;
    bytes_read = fread((void *)data,size,nElements,fs);
    if (endian=='B') { for (i=0;i<nElements;i++) SWAP_SHORT(data[i]); }

  } else if (type==40) {
    // 2-byte unsigned (40) shorts
    size = 2;
    Array *a= PushDataBlock(NewArray(&shortStruct, tmpDims));
    short *data = a->value.s;
    Array *b= PushDataBlock(NewArray(&longStruct, tmpDims));
    long *data2 = b->value.l;
    bytes_read = fread((void *)data,size,nElements,fs);
    if (endian=='B') { for (i=0;i<nElements;i++) SWAP_SHORT(data[i]); }
    for (i=1;i<=nElements;i++) *(data2++) = (((long)*(data++))|0xFFFF0000)+65535;

  } else if (type==50) {
    // 1-byte signed or unsigned chars (50)
    size = 1;
    Array *a= PushDataBlock(NewArray(&charStruct, tmpDims));
    char *data = a->value.c;
    bytes_read = fread((void *)data,size,nElements,fs);

  } else if (type==51) {
    // text (51)
    size = 1;
    Array *a= PushDataBlock(NewArray(&stringStruct, (Dimension *)0));
    char *buf;
    a->value.q[0] = buf = p_malloc(nElements+1);
    if (DEBUG) printf("strlen: %d\n",strlen((void *)a->value.q[0]));
    //    bytes_read = fread(a->value.q[0],1,nElements,fs);
    bytes_read = fread(buf,1,nElements,fs);
    *((char *)buf + nElements) = 0;  // append a NULL to text string

  } else {
    matclose(filename);
    sprintf(message,"Unknown type %d",type);
    YError(message);
  }

  if (bytes_read!=nElements) {
    fseek(fs,nElements*size,SEEK_CUR);
    matclose(filename);
    if (DEBUG) printf("read:%ld expected:%ld\n",bytes_read,nBytesToRead);
    YError("Premature end of file");
  }

  if (!leave_open) matclose(filename);
}

/***********************************************/

// matskip() skips over the next array in a matfile
int matskip(char *filename)
{
  unsigned long nbytes, bytes_read;
  unsigned long type,namelen;
  long mrows,mcols,imagf;
  FILE *fs;
  int fileptr;
  int endian = 'L';
  int size;

  fs = openmat(filename);
  if (fs == NULL) return -1;
  
  fileptr = ftell(fs);

  bytes_read = fread(&type,sizeof(long),1,fs);
  if (bytes_read==0) return (-1); // end of file
  fread(&mrows,sizeof(long),1,fs);
  fread(&mcols,sizeof(long),1,fs);
  fread(&imagf,sizeof(long),1,fs);
    
  fread(&namelen,sizeof(long),1,fs);

  //if (type & 0xffff0000) {
  //  endian = 'B';
  // swap((void *)&type,4,1);
  // swap((void *)&mrows,4,1);
  // swap((void *)&mcols,4,1);
  // swap((void *)&imagf,4,1);
  // swap((void *)&namelen,4,1);
  // type = type%1000;
  //}
  if (namelen>255) {
    fseek(fs,fileptr,SEEK_SET);  // leave file ptr at begginning of this variable
    return(-1);
  }

  fread(tempvarname,(unsigned int)namelen,1,fs);   

  if (type==0) {  // 8-byte doubles 
    size = 8;
  } else if (type==10) {  // 4-byte reals 
    size = 4;
  } else if ((type==120) || (type==20)) {  // 4-byte int 
    size = 4;
  } else if ((type==30) || (type==40)) {  // 2-byte signed (30) or unsigned (40) shorts 
    size = 2;
  } else if ((type==50) || (type==51)) {  // 1-byte signed or unsigned chars (50) or text (51)
    size = 1;
    endian = 'L';   // don't bother swapping bytes
  } else {
    return -1;
  }

  nbytes = mrows * mcols* size;

  fseek(fs,nbytes,SEEK_CUR);

  return (0);
}


/***********************************************/

// This routine checks for the existence of "varname" within 50 variables of
// the current file position
// If the variable is found, the file pointer points there;
// if not, the file remains at the position it had when entering this routine
int matsearch(char *filename, char *varname)
{
  FILE *fs;

  fs = openmat(filename);
  if (fs == NULL) return -1;

  return matfind(fs,varname,50000);    
}

void Y_ml4search(int nArgs)
{
  char *filename=YGetString(sp-nArgs+1);
  char *varname=YGetString(sp-nArgs+2);

  FILE *fs;

  fs = openmat(filename);
  if (fs == NULL) YError(p_strncat("Can't open file ",filename,0));

  PushIntValue(matfind(fs,varname,50000));    
}


void Y_ml4scan(int nArgs)
{
  char *filename=YGetString(sp-nArgs+1);
  int maxvar=0;
  int returnString=(1-yarg_subroutine());
  if (nArgs==1) {
    maxvar=10000;
  } else if (nArgs==2) {
    maxvar=YGetInteger(sp-nArgs+2);
  } else {
    YError("ml4scan takes one or two arguments");
  }
  
  FILE *fs;

  fs = openmat(filename);
  if (fs == NULL) YError(p_strncat("Can't open file ",filename,0));

  matscan(fs,maxvar,returnString);

  matclose(filename);
}

/***********************************************/

void matclose(char *fullname)
{
  int i;
  //  char *fullname;
  //  fullname = p_strncat(filename,".mat",0);

  for (i=0 ; i<nfiles ; i++) {
    if (!strcmp(fullname,matfile[i])) break;
  }

  if (i<nfiles) {
    fclose(fd[i]);
    //    fd[i] = NULL;
    *matfile[i] = 0;
    if (i==nfiles-1) nfiles--;
  }
}

void Y_ml4close(int nArgs)
{
  char *filename=YGetString(sp-nArgs+1);
  matclose(filename);
}

/***************************************************/

int textread(char *file, char *variable, float *values, int nvals)
{
  FILE *fd;
  int i,mvals=0;
  char line[82];
  char *p;

  fd=fopen(file,"rt");
  while (fgets(line,81,fd)) {
    if (!(p=strtok(line,"="))) continue;
    if (strncmp(p,variable,strlen(variable))) continue;
    for (i=0 ; i<nvals ; i++) {
      if (!(p=strtok((char *)NULL,", \t"))) break;
      *(values+i) = (float)atof(p);
      mvals++;
    }
  }
  for (i=mvals ; i<nvals ; i++) *(values+i) = 0;
  fclose(fd);
  return (mvals);
}

/****************************************************************************/

// This routine searches for the existence of "var"
// The search can be limited to the next "maxVarsToSearch" by setting maxVarsToSearch>0
int matfind(FILE *fs, char *var, int maxVarsToSearch)
{
  int  info[5];
  long i;
  long fileptr,tfileptr,tfp;
  long nbyt,nelem,skip;
  long rest,prec;
  int type,mrows,mcols;
  int imagf;
  int namelen;
  long varNumber = 0;
  char varname[80];
  char string[200];

  if (*var=='*') return (1);      // requested variable name matches any array name
  fileptr = ftell(fs);

  if (DEBUG) printf("Entering matfind\n");

  while (1) {

    tfileptr = ftell(fs);

    if (DEBUG) printf("at address %ld \n",tfileptr);

    if (fread(info,4,5,fs)==5) {

      if (info[4] & 0xffff0000) {	// convert header from big endian to little indian
        // info[0] changed to info[4] 2006/3/15 as double type can be 0, hence
        // no way to know big from little endian info[0] for doubles.
        if (DEBUG) printf("swapping!\n");
        for (i=0;i<5;i++) SWAP_INT(info[i]);
      }

      info[0] = info[0]%1000;

      tfp = ftell(fs);

      if (DEBUG) printf("at address %ld \n",tfp);
      if (DEBUG) printf("info = %d %d %d %d %d\n",info[0],info[1],info[2],info[3],info[4]);

      if ((namelen = info[4])<80L) {
        if (fread(varname,1,info[4],fs)==(int)info[4]) {
          if (DEBUG) printf("variable name: %s\n",varname);
          if (matchvarname(varname,var)) {    // success if a (possibly wildcard in "var") match
            fseek(fs,tfileptr,SEEK_SET);
            return (1);
          } else {
            type = *info - 10*(*info/10);
            rest = (*info - type)/10;
            prec = rest - 10*(rest/10);
            switch (prec) {
            case 0: nbyt = 8; break;
            case 1: nbyt = 4; break;
            case 2: nbyt = 4; break;
            case 3: nbyt = 2; break;
            case 4: nbyt = 2; break;
            case 5: nbyt = 1; break;
            default:
              sprintf(string,"Precision specification not available");
              warn(string);
              fseek(fs,fileptr,SEEK_SET);
              return (0);
            }
            mrows=info[1];
            mcols=info[2];
            nelem=mrows*mcols;
            imagf=info[3];
            if (imagf) nbyt=2*nbyt;
            skip = nbyt*nelem;
            if (DEBUG) printf("skiping %ld bytes\n",skip);
            if (skip) fseek(fs,nbyt*nelem,SEEK_CUR);
          }
        }
      }
    } else {
      break;
    }
    if (maxVarsToSearch) {
      if (++varNumber >= maxVarsToSearch) {
        break;
      }
    }
  }
  //lseek(fh,fileptr,SEEK_SET);
  fseek(fs,fileptr,SEEK_SET);
  return(0); 

}

/***************************************************************/

void matscan(FILE *fs, int maxVarsToSearch, int returnString)
{
  int  info[5];
  long i;
  long fileptr,tfileptr,tfp;
  long nbyt=0,nelem,skip;
  int  type;
  int  mrows,mcols;
  int  imagf;
  int  namelen;
  long varNumber = 0;
  char varname[80];
  char *stype="";
  int varnum=0;
  Array *a= PushDataBlock(NewArray(&stringStruct, (Dimension *)0));
  long extra=1;
  
  fileptr = ftell(fs);

  if (DEBUG) printf("Entering matscan\n");

  while (1) {
    tfileptr = ftell(fs);
    if (DEBUG) printf("at address %ld \n",tfileptr);
    if (fread(info,4,5,fs)==5) {

      if (info[4] & 0xffff0000) {	// convert header from little endian to big indian
        // info[0] changed to info[4] 2006/3/15 as double type can be 0, hence
        // no way to know big from little endian info[0] for doubles.
        if (DEBUG) printf("swapping!\n");
        for (i=0;i<5;i++) SWAP_INT(info[i]);
      }

      info[0] = info[0]%1000;

      tfp = ftell(fs);

      if (DEBUG) printf("at address %ld \n",tfp);
      if (DEBUG) printf("info = %d %d %d %d %d\n",info[0],info[1],info[2],info[3],info[4]);

      type = info[0]%1000;

      if ((namelen = info[4])<80L) {
        if (fread(varname,1,info[4],fs)==(int)info[4]) {
          if (type==0) {
            // 8-byte doubles 
            stype=p_strcpy("double*8"); nbyt=8;
          } else if (type==10) {
            // 4-byte reals 
            stype=p_strcpy("real*4  "); nbyt=4;
          } else if ((type==120) || (type==20)) {
            // 4-byte int 
            stype=p_strcpy("int*4   "); nbyt=4;
          } else if (type==30) {
            // 2-byte signed (30) shorts 
            stype=p_strcpy("short*2 "); nbyt=2;
          } else if (type==40)  {
            // 2-byte unsigned (40) shorts 
            stype=p_strcpy("ushort*2"); nbyt=2;
          } else if ((type==50) || (type==51))  {
            // 1-byte signed or unsigned chars (50) or text (51)
            stype=p_strcpy("char*1  "); nbyt=1; 
          } else {
            sprintf(message,"Unknown data type %d",type);
            YError(message);
          }
          
          if (returnString) {
            if (varnum!=0) a= PushDataBlock((void *)GrowArray(a, extra));
            a->value.q[varnum] = p_malloc(81);
            sprintf(a->value.q[varnum],"%30s  %s array [%d,%d]",varname,   \
                    stype,info[1],info[2]);
            varnum++;
          } else {
            printf("%30s  %s array [%d,%d]\n",varname,stype,info[1],info[2]);
          }

          mrows=info[1];
          mcols=info[2];
          nelem=mrows*mcols;
          imagf=info[3];
          if (imagf) nbyt=2*nbyt;
          skip = nbyt*nelem;
          if (DEBUG) printf("skiping data part: %ld bytes\n",skip);
          if (skip) fseek(fs,nbyt*nelem,SEEK_CUR);
        }
      }
    } else {
      break;
    }
    if (maxVarsToSearch) {
      if (++varNumber >= maxVarsToSearch) {
        break;
      }
    }
  }
}

/****************************************************************************/

void writerr(void)
{
  if (errno == ENOSPC) {
    warn("Insufficient Disk Space!");
  } else if (errno == EBADF) {
    warn("Bad File Descriptor!");
  } else {
    warn("Error Writing Data File!");
  }
}

/***************************************************/

// accepts '?' and single incidence of '*' in "match"; returns 1 if a match, 0 otherwise
int matchvarname(char *var, char *match)
{
  int i,n1,n2;
  char *p,*p1,*p2;

  if (*match == '*') return 1;  // guaranteed match if first char of match string is '*'

  n1 = strlen(var);
  if ((p=strchr(match,'*'))) {
    n2 = p-match;
    if (n2 > n1) return 0;       // guaranteed mismatch if lengths of unambiguous portions of strings are not equal
  } else {
    n2 = strlen(match);
    if (n1!=n2) return 0;        // guaranteed mismatch if lengths of strings are not equal
  }
    
  for (i=0,p1=var,p2=match ; i<n2 ; i++,p1++,p2++) {
    if (*p2 == '?') continue;   // '?' matches any character in this position
    if (*p1 != *p2) {
      return 0;
    }
  }

  return 1;       // 
}

/***********************************************/

void InitMatsave(int saveStates)    // 0 => don't create or output to files
{
  saveNormally = saveStates;
}

/***********************************************/

void warn(char *mes) { printf("%s\n",mes); }

static long _TestEndian=1;

int IsLittleEndian(void) {
  return *(char*)&_TestEndian;
}

void Y_ml4endian(int nArgs)
{
  PushIntValue(*(char*)&_TestEndian);
}

/******************************************************************************
  FUNCTION: SwapEndian
  PURPOSE: Swap the byte order of a structure
  EXAMPLE: float F=123.456;; SWAP_FLOAT(F);
******************************************************************************/

void *swap(void* Addr, const int Nb) {
  static char Swapped[16];

  switch (Nb) {
  case 2:
    Swapped[0]=*((char*)Addr+1);
    Swapped[1]=*((char*)Addr  );
    break;
  case 3:	// As far as I know, 3 is used only with RGB images
    Swapped[0]=*((char*)Addr+2);
    Swapped[1]=*((char*)Addr+1);
    Swapped[2]=*((char*)Addr  );
    break;
  case 4:
    Swapped[0]=*((char*)Addr+3);
    Swapped[1]=*((char*)Addr+2);
    Swapped[2]=*((char*)Addr+1);
    Swapped[3]=*((char*)Addr  );
    break;
  case 8:
    Swapped[0]=*((char*)Addr+7);
    Swapped[1]=*((char*)Addr+6);
    Swapped[2]=*((char*)Addr+5);
    Swapped[3]=*((char*)Addr+4);
    Swapped[4]=*((char*)Addr+3);
    Swapped[5]=*((char*)Addr+2);
    Swapped[6]=*((char*)Addr+1);
    Swapped[7]=*((char*)Addr  );
    break;
  case 16:
    Swapped[0]=*((char*)Addr+15);
    Swapped[1]=*((char*)Addr+14);
    Swapped[2]=*((char*)Addr+13);
    Swapped[3]=*((char*)Addr+12);
    Swapped[4]=*((char*)Addr+11);
    Swapped[5]=*((char*)Addr+10);
    Swapped[6]=*((char*)Addr+9);
    Swapped[7]=*((char*)Addr+8);
    Swapped[8]=*((char*)Addr+7);
    Swapped[9]=*((char*)Addr+6);
    Swapped[10]=*((char*)Addr+5);
    Swapped[11]=*((char*)Addr+4);
    Swapped[12]=*((char*)Addr+3);
    Swapped[13]=*((char*)Addr+2);
    Swapped[14]=*((char*)Addr+1);
    Swapped[15]=*((char*)Addr  );
    break;
  }
  return (void*)Swapped;
}

