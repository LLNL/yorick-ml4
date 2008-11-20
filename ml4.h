/* ml4.h Yorick plugin
 *
 * header file for ml4.c. C function for matlab 4 IO
 *
 * $Id: ml4.h,v 1.2 2008-11-20 14:34:46 frigaut Exp $
 * Francois Rigaut, 2005-2007
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
 * $Log: ml4.h,v $
 * Revision 1.2  2008-11-20 14:34:46  frigaut
 * - Included and uploaded changes from Thibaut Paumard making ml4 64 bits safe.
 * - Beware: In 64 bits, longs (8bytes) are saved as int (4bytes)
 *
 * - a few more minor text edits
 * - added ML4_VERSION variable
 *
 *
*/

int matout(char *filename,char *varname,void *var,int nrows,int ncols,char vartype, char *mode, char endianess);
int matout_string(char *filename,char *varname,char *string,char *mode);
FILE *openmat(char *filename);
int matin(char *filename,char *varname,void *var,int nrows,int ncols,int vartype);
int matread(char *filename, char *varname,void *var,int nrows,int ncols,int *realrows, int *realcols, int vartype);
int matskip(char *filename);
int matcheck(char *filename, char *varname, char *message_insert, int nrows, int ncols, int vartype);
int matsearch(char *filename, char *varname);
void matscan(FILE *fs, int maxVarsToSearch, int returnString);
void matclose(char *filename);
int textread(char *file, char *variable, float *values, int nvals);
int matfind(FILE *fs, char *var, int maxVarsToSearch);
int matload(char *filename, char *varname, void *var, int nelements, int vartype);
int matloadnowarn(char *filename, char *varname, void *var, int nelements, int vartype);
int matGetFrameCount(char *filename);
int matchvarname(char *var, char *match);
void InitMatsave(int saveStates);
int Mat5Out(char *filename,char *varname,void *var, int nframes, int nrows, int ncols, int vartype);
int Mat5HeaderOut(int fd);

