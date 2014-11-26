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

#ifndef _BOX_IMAGE_H
#define _BOX_IMAGE_H

#include "OCR_image.h"
#include "params.h"

//structure who contain a box
typedef struct {
    int num, start_frame, stop_frame;
    float ymin, xmin, ymax, xmax;
    float ymin_current, xmin_current, ymax_current, xmax_current;
    float ymin_avg, xmin_avg, ymax_avg, xmax_avg;
    float ymin_avg_t, xmin_avg_t, ymax_avg_t, xmax_avg_t;
    int current_thr, thr_med;
    float nb_cc;
    float tol_box;
    int max_thr;
    float max_val_diff_b2b;
    
    CvMemStorage* storage_thr;
    CvSeq* seq_thr;
    CvMemStorage* storage_thr_t;
    CvSeq* seq_thr_t;
    
    int nb_img_detect_avg, nb_img_detect_avg_t;
    int nb_transcription, total_transcription;
    
    IplImage* im_current;
    OCR_Image *im_average_mask;
    OCR_Image *im_average_mask_t;
    CvMemStorage* storage_im;
    CvSeq* seq_im;
}box;

// initialisation of a boxes
box* create_init_box(int start_frame, int stop_frame, float ymin, float xmin, float ymax, float xmax, int threshold, float nb_cc, IplImage* im_current, IplImage* im_average_mask, IplImage* im_average_mask_t, LOOV_params* param);
// free boxes memory
void free_box(box* pt_box);    
// printing value of the current boxes
void print_box(box* pt_box, LOOV_params* param);
// write in a file value of the current boxes
void file_print_box(FILE* file_txt, box* pt_box, LOOV_params* param);
// to compare 2 box with tolerance, function for opencv
int cmp_box( const void* _a, const void* _b, void* userdata); 
// to compare 2 box by there start frame to sort them, function for opencv
int cmp_box_by_frame( const void* _a, const void* _b, void* userdata );  
// to compare 2 box by their threshold, function for opencv
int cmp_thr( const void* _a, const void* _b, void* userdata ); 
// process transcription of a boxes
void transcription_box(box *pt_search_box, LOOV_params* param);
// update boxes
void update_box(CvSeq* seq_box, int idx, int frameNum, int threshold_found, box* pt_rect_box, int nb_cc, IplImage* frame_BW, IplImage* im_box, LOOV_params* param);

#endif
