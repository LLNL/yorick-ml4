/* ml4.i Yorick plugin
 *
 * Yorick wrappers for ml4.c
 * Matlab 4 IO
 *
 * $Id: ml4.i,v 1.3 2010-04-15 02:49:42 frigaut Exp $
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
 * $Log: ml4.i,v $
 * Revision 1.3  2010-04-15 02:49:42  frigaut
 *
 *
 * repo update to 0.6.0
 *
 * Revision 1.2  2008/11/20 14:34:46  frigaut
 * - Included and uploaded changes from Thibaut Paumard making ml4 64 bits safe.
 * - Beware: In 64 bits, longs (8bytes) are saved as int (4bytes)
 *
 * - a few more minor text edits
 * - added ML4_VERSION variable
 *
 *
*/

plug_in,"ml4";
ML4_VERSION = "0.6.0";

local ml4
/* DOCUMENT matlab4 I/O
   This package was develop for the Gemini MCAO (RTC file exchange)
   Available functions:
   ml4scan(file[,maxvar])
   ml4search(file,varname)
   ml4read(file,varname,leave_open)
   ml4close,file
   ml4write(file,data,varname,mode,endian=)
   SEE ALSO:
 */

extern ml4endian
/* DOCUMENT ml4endian
   return 1 is machine is little endian, 0 if big endian
   SEE ALSO:
 */

if (ml4endian) default_endian='L'; else default_endian='B';

extern ml4scan
/* DOCUMENT ml4scan,file[,maxvar]
   Scans a matlab4 file for variables and output name, type and dimension.
   If used as a subroutine, the result is printed on the screen.
   If used as a function, returns what would have been printed on the screen.
   maxvar is the maximum number of variables to scan (optional).
   SEE ALSO: ml4read, ml4search
 */

extern ml4search
/* DOCUMENT ml4search(file,varname)
   Returns 1 if the variable is present in file, 0 otherwise
   SEE ALSO: ml4scan, ml4read
 */

extern ml4read
/* DOCUMENT ml4read(file,varname,leave_open)
   Returns the data associated with "varname"
   leave_open will prevent closing the file when the operation is completed.
   This can be useful when reading files containing large numbers of variables.
   Be aware that it is left open, pointed at the next variable in the file.
   Hence a request to read a variable located prior to the one you just read
   will fail ("No Such Variable").
   When the variable read is corrupted (i.e. with NaN), the file is not close
   and hence requires a manual ml4close.
   SEE ALSO: ml4scan, ml4search, ml4close.
 */

extern ml4close
/* DOCUMENT ml4close,file
   Closes access to the matlab4 file.
   SEE ALSO:
 */

func ml4write(file,data,varname,mode,endian=)
/* DOCUMENT ml4write(file,data,varname,mode,endian=)
   mode : "w" or "a"
   endian= 'L' will write the file in little endian. Useful if the target
   machine uses little endian.
   SEE ALSO: ml4read, ml4close
 */
{
  if ((mode!="w")&&(mode!="a")) error,"mode should be \"w\" or \"a\"";
  if (default_endian) endian=default_endian;
  //  if (!endian) endian='B';
  if ((endian!='L')&&(endian!='B')) error,"Endian should be 'L' or 'B'";
  
  if (structof(data)==string) {
    status=matout_string(file, varname, data(1), mode);
    if (status) error,"string write failed";
    return;
  }

  dims = dimsof(data);

  if (dims(1)>2) error,"ml4write only supports writing up 2D arrays";

  if (dims(1)==0) {
    nrows=1n;
    ncols=1n;
  } else if (dims(1)==1) {
    nrows=1n;
    ncols=int(dims(2));
  } else if (dims(1)==2) {
    nrows=int(dims(2));
    ncols=int(dims(3));
  }
  
  tmp = data; // otherwise data may be swapped in place in endian='L'

  // next line: fix from Thibaut Paumard for Debian.
  if      (structof(data)==long)   { type='l'; tmp=int(tmp); }
  else if (structof(data)==int)    { type='l'; }
  else if (structof(data)==float)  { type='r'; }
  else if (structof(data)==double) { type='d'; }
  else if (structof(data)==short)  { type='s'; }
  else if (structof(data)==char)   { type='b'; }
  else error,"Unsupported type";

  status = matout(file,varname,&tmp,nrows,ncols,type,mode,endian);
  if (status) error,"write failed";
}

//=====================================================

extern matout
/* PROTOTYPE
   int matout(string filename, string varname, pointer ptr,
   int nrows, int ncols, char vartype, string mode, char endianess)
*/

extern matout_string
/* PROTOTYPE
   int matout_string(string filename, string varname, string text, string mode)
*/

