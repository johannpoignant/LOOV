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

#ifndef _UNION_FIND_H
#define _UNION_FIND_H

#include "cv.h"

typedef struct{
    CvMemStorage* storage;
    CvSeq* seq;
}l_pt_NdG;

l_pt_NdG* create_init_l_pt_NdG();

void free_l_pt_NdG(l_pt_NdG* l);

int Find(int * tab, int pos);

void Union(int * tab, int pos1, int pos2);

#endif

