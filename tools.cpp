/*************************************************************************
Copyright (C) <2014>  <Johann POIGNANT>

This file is part of LOOV.

LOOV is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

LOOV is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with LOOV.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "tools.h" 


//find the nearest integer of a float   
int round_me(float f) {                                   
     if ((int)(f+0.5)==(int)(f)) return (int)(f);
     else return (int)(f+1);
}


char * sprintf_alloc(const char *fmt, ...){
  char *p;
  va_list ap;  
  int i;
  va_start(ap, fmt);
  i=vasprintf(&p, fmt, ap);
  va_end(ap);
  if (i==-1) return NULL;
  return(p);
}

char *basename_a(char* fileName){  
  char *pos = rindex(fileName,CSEP);
  if (pos) return pos+1;
  return fileName;
}

int fileexists(const char *fname){
  int ret;
  struct stat buf;
  if (!fname || !fname[0]) return(0);
  if (!(ret = stat(fname,&buf))) return(1);
  errno = 0;
  return(0);
}

int get_Arguments( int argc, char* argv[], int *pfrom,  char *option, char *format, ...){
  va_list ap;  
  int nbparam=0;  
  char *tmp;
  int localFrom=0;
  int *from;
  int i;
  if (format){ tmp=format;      while((tmp=strchr(tmp, '%'))){ nbparam++; tmp++;} }
  from= (pfrom) ? pfrom : &localFrom;
  /* looking for a flag */
  if (nbparam==0){
    for (i=*from; i< argc && argv[i][0]; ++i){
      if (! strcmp(option, argv[i])){ *from=i; return 1; }
    }
    return 0;
  }
  for (i=*from;i< argc-1 && argv[i][0] ; ++i){
    if (! strcmp(option, argv[i])){
      va_start(ap, format);
      if (vsscanf(argv[i+1],format, ap) == nbparam){ *from = i; return nbparam; }
      else {return -1;}
    }
  }    
  return 0;
}

char * get_String_Arguments(int argc, char *argv[], char const * option){
  int i;
  for (i=0; i< argc -1; i++){
    if (! strcmp(option, argv[i])){
      return argv[i+1];
    }
  }
  return NULL;
}
