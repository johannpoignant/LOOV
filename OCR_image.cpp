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
#include "cv.h"
#include "baseapi.h"
#include "tools.h" 
#include "OCR_image.h"

// initialisation of an OCR image
OCR_Image create_init_OCR_image(IplImage *img, char *text, int confidence, float threshold){
    OCR_Image oi;
    oi.img=NULL;
    oi.text=NULL;
    oi.confidence=-1;
    oi.threshold=-1;
    if (text){ oi.text=strdup(text); if (!oi.text){ return oi; } }
    oi.img=img;
    oi.confidence=confidence;
    oi.threshold=threshold;
    return oi;
}

// allocate memory
OCR_Image* alloc_init_OCR_Image(IplImage *img, char *text, int confidence, float threshold){
    OCR_Image *oi=(OCR_Image*) malloc(sizeof(OCR_Image));
    if (! oi) return NULL;
    *oi=create_init_OCR_image(img,text,confidence, threshold);
    return oi;
}

// free memory
void free_OCR_Image(OCR_Image *oi){
    if (oi->img){ cvReleaseImage(&oi->img); oi->img=NULL;  }
    if (oi->text){ free(oi->text);          oi->text=NULL; }
    oi->confidence=-1;
    oi->threshold=-1;
}

// compare 2 point by there coordinate, function for opencv
int cmp_opencv_point( const void* _a, const void* _b, void* userdata )  {
    CvPoint* a = (CvPoint*)_a;
    CvPoint* b = (CvPoint*)_b;
    int y_diff = a->y - b->y;
    int x_diff = a->x - b->x;
    return y_diff ? y_diff : x_diff;
}

// set an IplImage into an OCR image
void set_img_OCR_Image(OCR_Image *oi, IplImage *img){ oi->img=img;}
// get an IplImage from an OCR image
IplImage* get_img_OCR_Image(OCR_Image *oi){ return oi->img;}

// change IplImage of an OCR image
void change_img_OCR_Image(OCR_Image *oi,IplImage *img){
    cvReleaseImage(& (oi->img));
    if (img){ set_img_OCR_Image(oi, img); }
}

// set a transcription for an OCR image
int set_text_OCR_Image(OCR_Image *oi, char *text){
    if (oi->text) free(oi->text);
    if (! (oi->text=strdup(text))) return 0;
    return 1;
}

// set a confidence for an OCR image
void set_confidence_OCR_Image(OCR_Image *oi, int conf){ oi->confidence=conf;}

// set a threshold for an OCR image
void set_threshold_OCR_Image(OCR_Image *oi, float threshold){ oi->threshold=threshold;}

// normalise string
void normalise_string(char* text){
    int i;
    for (i=0;i<strlen(text);i++){
        if (text[i]=='\n')  text[i]=' ';
        if (text[i]=='\'')  text[i]=' ';                  
    }
    text[strlen(text)-1]='\n';
}

// process transcription of an OCR image and save it
void transcription_oi(OCR_Image * oi, LOOV_params* param){ 
	IplImage *img;
	char* res;
	tesseract::TessBaseAPI  api;
	api.Init(param->tessdata, param->lang, NULL, 0, true);
	api.SetPageSegMode(tesseract::PSM_SINGLE_LINE);
	img=get_img_OCR_Image(oi);
	if (img){
		IplImage* im_box_resize = cvCreateImage(cvSize(round_me((float)param->resize_para*(float)(img->width)/(float)(img->height)), param->resize_para),img->depth, 1);
		cvResize(img, im_box_resize, CV_INTER_CUBIC);
		cvThreshold(im_box_resize, im_box_resize, round_me(oi->threshold), param->max_thr, CV_THRESH_TOZERO);  
	    api.SetImage((const unsigned char*) im_box_resize->imageData, im_box_resize->width, im_box_resize->height, im_box_resize->widthStep/im_box_resize->width, im_box_resize->widthStep);
        res = api.GetUTF8Text();
        normalise_string(res);
	    set_text_OCR_Image(oi,res);		
        set_confidence_OCR_Image(oi,api.MeanTextConf());
	    cvReleaseImage(&im_box_resize);
	    delete[](res);	 
  	}
	else{
		set_text_OCR_Image(oi,NULL);
		set_confidence_OCR_Image(oi,-2);
	}	
}

// print transcription of an OCR image
void print_transcription_image(IplImage *img, int threshold, LOOV_params* param){
	char* res;
	tesseract::TessBaseAPI  api;
	api.Init(param->tessdata, param->lang, NULL, 0, true);
	api.SetPageSegMode(tesseract::PSM_SINGLE_LINE);

    IplImage* im_box_resize = cvCreateImage(cvSize(round_me((float)param->resize_para*(float)(img->width)/(float)(img->height)), param->resize_para),img->depth, 1);
    cvResize(img, im_box_resize, CV_INTER_CUBIC);
    cvThreshold(im_box_resize, im_box_resize, round_me(threshold), param->max_thr, CV_THRESH_TOZERO);  
    api.SetImage((const unsigned char*) im_box_resize->imageData, im_box_resize->width, im_box_resize->height, im_box_resize->widthStep/im_box_resize->width, im_box_resize->widthStep);
    res = api.GetUTF8Text();
    normalise_string(res);
    printf("conf=%d ", api.MeanTextConf());
    printf("text: %s", res);
    cvReleaseImage(&im_box_resize);
    delete[](res);	 
}
