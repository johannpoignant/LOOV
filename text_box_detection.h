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

#ifndef _TEXT_BOX_DETECTION_H
#define _TEXT_BOX_DETECTION_H

#include "cv.h"
#include "highgui.h"

#include "tools.h" 
#include "union_find.h" 
#include "params.h"
#include "box_image.h"
#include "OCR_image.h"

// Find global threshold with Sauvola binarisation algorithm
int sauvola(IplImage* im_thr, int video_depth);
// Find global threshold with Wolf binarisation algorithm
int wolf(IplImage* im_thr, int video_depth);
// Find threshold to delete noise in the image
double noise_image(IplImage* im, int* thr_otsu, LOOV_params* param);
// Apply a sobel filter to detect character edges
IplImage* sobel_double_H(IplImage* im, LOOV_params* param );
// Connect character from Sobel image with dilatation and erosion
IplImage* connected_caractere(IplImage* im, LOOV_params* param);
// Delete horizontal noise
IplImage* delete_horizontal_bar(IplImage* im, LOOV_params* param);   
// Delete vertical noise 
IplImage* delete_vertical_bar(IplImage* im, LOOV_params* param);
// refine spatial detection of boxes
void refine_detection(int* threshold_found, int* ymin, int* ymax, int* xmin, int* xmax, IplImage* frame_BW, int* ymin_final, int* ymax_final, int* xmin_final, int* xmax_final, LOOV_params* param);
// coarse spatial detection of boxes
void spatial_detection_box(IplImage* im, CvSeq* seq_box, int frameNum, IplImage* frame_BW_original, IplImage* frame_4, IplImage* frame_5, IplImage* im_mask, LOOV_params* param);
// temporal tracking of boxes
void temporal_detection_box(CvSeq* seq_box, CvSeq* seq_box_final, int frameNum, IplImage* frame_BW, IplImage* im_mask, LOOV_params* param);



#endif
