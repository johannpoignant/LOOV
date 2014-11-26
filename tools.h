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

#ifndef _TOOLS_H
#define _TOOLS_H

#define CSEP  '\\'

int round_me(float f);

char * sprintf_alloc(const char *fmt, ...);

int fileexists(const char *fname);

char *basename_a(char* fileName);

int get_Arguments( int argc, char* argv[], int *pfrom,  char *option, char *format, ...);

char * get_String_Arguments(int argc, char *argv[], char const * option);

#endif

