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

#include "cv.h"
#include "highgui.h"

#include "union_find.h" 



l_pt_NdG* create_init_l_pt_NdG(){
    l_pt_NdG* new_l=(l_pt_NdG*)malloc(sizeof(l_pt_NdG)); 
    if (new_l==NULL) return NULL;
    new_l->storage=cvCreateMemStorage(0);  
    new_l->seq = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvPoint), new_l->storage);
    return new_l;
}

void free_l_pt_NdG(l_pt_NdG* l){
    cvReleaseMemStorage(&(l->storage));
    free(l);
}

int Find(int * tab, int pos){
      if (tab[pos] <= -1) return pos;
      int posRoot = Find(tab, tab[pos]);
      tab[pos] = posRoot;
     return posRoot;
}

void Union(int * tab, int pos1, int pos2){
     int pos1Root = Find(tab, pos1);
     int pos2Root = Find(tab, pos2);     
     if (pos1Root!=pos2Root){     
          if (tab[pos1Root] <= tab[pos2Root]){     
               tab[pos1Root]=tab[pos1Root]+tab[pos2Root];
               tab[pos2Root]=pos1Root;
               tab[pos2]=pos1Root;
          }
          else{
               tab[pos2Root]=tab[pos2Root]+tab[pos1Root];
               tab[pos1Root]=pos2Root;
               tab[pos1]=pos2Root;
          }
     }
     
}
