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

#ifndef _OCR_IMAGE_H
#define _OCR_IMAGE_H

#include "cv.h"
#include "params.h"

// structur for an OCR image
typedef struct{
    IplImage *img;
    char     *text;       // text as output by OCR, NULL if no text (see confidence field for error)
    int      confidence;  // as given by the OCR, -1 uninitialized value, -2 tesseract failure
    float    threshold;   // value of the threshold for image binarisation
}OCR_Image;

// initialisation of an OCR image
OCR_Image create_init_OCR_image(IplImage *img, char *text, int confidence, float threshold);
// allocate memory
OCR_Image* alloc_init_OCR_Image(IplImage *img, char *text, int confidence, float threshold);
// free memory
void free_OCR_Image(OCR_Image *oi);
// compare 2 point by there coordinate, function for opencv
int cmp_opencv_point( const void* _a, const void* _b, void* userdata ); 
// set an IplImage into an OCR image
void set_img_OCR_Image(OCR_Image *oi, IplImage *img);
// get an IplImage from an OCR image
IplImage* get_img_OCR_Image(OCR_Image *oi);
// change IplImage of an OCR image
void change_img_OCR_Image(OCR_Image *oi,IplImage *img);
// set a transcription for an OCR image
int set_text_OCR_Image(OCR_Image *oi, char *text);
// set a confidence for an OCR image
void set_confidence_OCR_Image(OCR_Image *oi, int conf);
// set a threshold for an OCR image
void set_threshold_OCR_Image(OCR_Image *oi, float threshold);
// normalise string
void normalise_string(char* text);
// process transcription of an OCR image
void transcription_oi(OCR_Image * oi, LOOV_params* param); 
// print transcription of an OCR image
void print_transcription_image(IplImage *img, int threshold, LOOV_params* param);


#endif
