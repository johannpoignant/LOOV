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
#include "highgui.h"

#include "tools.h" 
#include "box_image.h"

// initialisation of a boxes
box* create_init_box(int start_frame, int stop_frame, float ymin, float xmin, float ymax, float xmax, int threshold, float nb_cc, IplImage* im_current, IplImage* im_average_mask, IplImage* im_average_mask_t, LOOV_params* param){
    box* new_box=(box*)malloc(sizeof(box));                                    //alloc memory for a new box
    if (!new_box) return  NULL;
    param->total_box++;
    new_box->num = param->total_box;    
    new_box->start_frame = start_frame;    new_box->stop_frame = stop_frame;
    new_box->ymin = ymin;          new_box->xmin = xmin;            new_box->ymax = ymax;           new_box->xmax = xmax;
    new_box->ymin_current = ymin;  new_box->xmin_current = xmin;    new_box->ymax_current = ymax;   new_box->xmax_current = xmax;
    new_box->ymin_avg = ymin;      new_box->xmin_avg = xmin;        new_box->ymax_avg = ymax;       new_box->xmax_avg = xmax;
    new_box->ymin_avg_t = ymin;    new_box->xmin_avg_t = xmin;      new_box->ymax_avg_t = ymax;     new_box->xmax_avg_t = xmax;    
    
    new_box->thr_med = threshold;  new_box->current_thr = threshold;
    new_box->nb_cc=nb_cc;
    new_box->tol_box=param->tol_box;
    new_box->max_thr=param->max_thr;
    new_box->max_val_diff_b2b=param->max_val_diff_b2b;
    
    new_box->storage_thr = cvCreateMemStorage(0);                            //create storage for the sequence image for the new box
    new_box->seq_thr = cvCreateSeq( 0, sizeof(CvSeq), sizeof(int), new_box->storage_thr);    //create seq of images for the new box
    cvSeqPush(new_box->seq_thr, &threshold);

    new_box->storage_thr_t = cvCreateMemStorage(0);                            //create storage for the sequence image for the new box
    new_box->seq_thr_t = cvCreateSeq( 0, sizeof(CvSeq), sizeof(int), new_box->storage_thr_t);    //create seq of images for the new box
    cvSeqPush(new_box->seq_thr_t, &threshold);
    
    new_box->nb_img_detect_avg = 0;     new_box->nb_img_detect_avg_t = 0;    new_box->nb_transcription = 0;      new_box->total_transcription = 0;
    
    new_box->im_current = cvCloneImage(im_current);
    new_box->im_average_mask   = alloc_init_OCR_Image(cvCloneImage(im_average_mask),NULL,-1,-1);
    new_box->im_average_mask_t = alloc_init_OCR_Image(cvCloneImage(im_average_mask_t),NULL,-1,-1);

    new_box->storage_im = cvCreateMemStorage(0);                            //create storage for the sequence image for the new box
    new_box->seq_im = cvCreateSeq( 0, sizeof(CvSeq), sizeof(OCR_Image*), new_box->storage_im);    //create seq of images for the new box
    return new_box;
}

// free boxes memory
void free_box(box* pt_box){                                                    //free box memory
  cvReleaseImage(&(pt_box->im_current));    
  free_OCR_Image(pt_box->im_average_mask);
  free_OCR_Image(pt_box->im_average_mask_t);
  int j;
  for (j=0;j<pt_box->seq_im->total;j++){
      free_OCR_Image(*(OCR_Image**) cvGetSeqElem(pt_box->seq_im, j));
  }
  cvReleaseMemStorage( &(pt_box->storage_thr) );                         
  cvReleaseMemStorage( &(pt_box->storage_thr_t) );                          
  cvReleaseMemStorage( &(pt_box->storage_im) );                            //release memory of the sequence storage of the box
  free(pt_box);                                                            //free memory allocate to the box
}

// printing value of the current boxes
void print_box(box* pt_box, LOOV_params* param){                                    //printing value of the current box
    int ymin=round_me(pt_box->ymin);
    int xmin=round_me(pt_box->xmin);
    int ymax=round_me(pt_box->ymax);
    int xmax=round_me(pt_box->xmax);
    int pr=0;
    int i;
    if (pt_box->im_average_mask_t->text!=NULL){
        if (strlen(pt_box->im_average_mask_t->text)>3){
            printf("start_box\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s",pt_box->start_frame,pt_box->stop_frame, ymin, xmin, ymax, xmax, pt_box->im_average_mask_t->confidence, pt_box->im_average_mask_t->text);
            pr=1;
        }
        for (i=0;i<pt_box->seq_im->total;i++){
            OCR_Image * oi = *(OCR_Image**)cvGetSeqElem(pt_box->seq_im,i);
            if (pr==1){
                printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s",pt_box->start_frame+(i)*param->fr_reco,pt_box->start_frame+(i+1)*param->fr_reco-1,ymin, xmin, ymax, xmax, oi->confidence, oi->text);
            }
        }
        if (pr==1)  printf("end_box\n");
    }
    else if (pt_box->stop_frame-pt_box->start_frame>param->min_duration_box)  printf("%d\t%d\t%d\t%d\t%d\t%d\n",pt_box->start_frame,pt_box->stop_frame, ymin, xmin, ymax, xmax);
}

// write in a file value of the current boxes
void file_print_box(FILE* file_txt, box* pt_box, LOOV_params* param){                                    //printing value of the current box
    int ymin=round_me(pt_box->ymin);
    int xmin=round_me(pt_box->xmin);
    int ymax=round_me(pt_box->ymax);
    int xmax=round_me(pt_box->xmax);
    int pr=0;
    int i;
    if (pt_box->im_average_mask_t->text!=NULL){
        if (strlen(pt_box->im_average_mask_t->text)>3){
            fprintf(file_txt, "start_box\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s",pt_box->start_frame,pt_box->stop_frame, ymin, xmin, ymax, xmax, pt_box->im_average_mask_t->confidence, pt_box->im_average_mask_t->text);
            pr=1;
        }
        for (i=0;i<pt_box->seq_im->total;i++){
            OCR_Image * oi = *(OCR_Image**)cvGetSeqElem(pt_box->seq_im,i);
            if (pr==1)   fprintf(file_txt, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s",pt_box->start_frame+(i)*param->fr_reco,pt_box->start_frame+(i+1)*param->fr_reco-1,ymin, xmin, ymax, xmax, oi->confidence, oi->text);
        }
        if (pr==1)    fprintf(file_txt, "end_box\n");
    }
    else if (pt_box->stop_frame-pt_box->start_frame>param->min_duration_box)     fprintf(file_txt, "%d\t%d\t%d\t%d\t%d\t%d\n",pt_box->start_frame,pt_box->stop_frame, ymin, xmin, ymax, xmax);
}


// to compare 2 box with tolerance, function for opencv
int cmp_box( const void* _a, const void* _b, void* userdata) {       //compare 2 boxes with tolerance
    box* a = *(box**)_a;
    box* b = *(box**)_b;

    if (!a && !b) return 0;
    if (!a && b) return -1;
    if (a && !b) return 1;
        
    CvPoint2D32f tab_pxA[4] = {cvPoint2D32f(0,0),cvPoint2D32f(0,0),cvPoint2D32f(0,0),cvPoint2D32f(0,0)};
    CvPoint2D32f tab_pxB[4] = {cvPoint2D32f(0,0),cvPoint2D32f(0,0),cvPoint2D32f(0,0),cvPoint2D32f(0,0)};
    
    int nb_pxA=0;
    int nb_pxB=0;
        
    if ((a->ymin >= b->ymin) && (a->ymin <= b->ymax) && (a->xmin >= b->xmin) && (a->xmin <= b->xmax))	{tab_pxA[0] = cvPoint2D32f(a->xmin,a->ymin); nb_pxA++; }
    if ((a->ymax >= b->ymin) && (a->ymax <= b->ymax) && (a->xmin >= b->xmin) && (a->xmin <= b->xmax))	{tab_pxA[1] = cvPoint2D32f(a->xmin,a->ymax); nb_pxA++; }
    if ((a->ymin >= b->ymin) && (a->ymin <= b->ymax) && (a->xmax >= b->xmin) && (a->xmax <= b->xmax))	{tab_pxA[2] = cvPoint2D32f(a->xmax,a->ymin); nb_pxA++; }
    if ((a->ymax >= b->ymin) && (a->ymax <= b->ymax) && (a->xmax >= b->xmin) && (a->xmax <= b->xmax))	{tab_pxA[3] = cvPoint2D32f(a->xmax,a->ymax); nb_pxA++; }
    
    if ((b->ymin >= a->ymin) && (b->ymin <= a->ymax) && (b->xmin >= a->xmin) && (b->xmin <= a->xmax))	{tab_pxB[0] = cvPoint2D32f(b->xmin,b->ymin); nb_pxB++; }
    if ((b->ymax >= a->ymin) && (b->ymax <= a->ymax) && (b->xmin >= a->xmin) && (b->xmin <= a->xmax))	{tab_pxB[1] = cvPoint2D32f(b->xmin,b->ymax); nb_pxB++; }
    if ((b->ymin >= a->ymin) && (b->ymin <= a->ymax) && (b->xmax >= a->xmin) && (b->xmax <= a->xmax))	{tab_pxB[2] = cvPoint2D32f(b->xmax,b->ymin); nb_pxB++; }
    if ((b->ymax >= a->ymin) && (b->ymax <= a->ymax) && (b->xmax >= a->xmin) && (b->xmax <= a->xmax))	{tab_pxB[3] = cvPoint2D32f(b->xmax,b->ymax); nb_pxB++; }    

    float intersection=0.0;
    float area_A = (a->ymax-a->ymin)*(a->xmax-a->xmin);
    float area_B = (b->ymax-b->ymin)*(b->xmax-b->xmin);
    int i;
    if (nb_pxA==1 && nb_pxB==1){
    	CvPoint2D32f ptA=cvPoint2D32f(0.0,0.0);
    	CvPoint2D32f ptB=cvPoint2D32f(0.0,0.0);
    	for (i=0;i<4;i++){
    		if (tab_pxA[i].x != 0.0 && tab_pxA[i].y != 0.0 ) ptA = tab_pxA[i];
    		if (tab_pxB[i].x != 0.0 && tab_pxB[i].y != 0.0 ) ptB = tab_pxB[i];
    	}
  	
    	intersection=fabs(ptA.x-ptB.x)*fabs(ptA.y-ptB.y);
    }
    else if (nb_pxA==2){    	
    	if (tab_pxA[0].x != 0 && tab_pxA[0].y != 0 && tab_pxA[1].x != 0 && tab_pxA[1].y != 0 ) intersection=fabs(tab_pxA[0].y-tab_pxA[1].y)*fabs(tab_pxA[0].x - b->xmax);
    	if (tab_pxA[0].x != 0 && tab_pxA[0].y != 0 && tab_pxA[2].x != 0 && tab_pxA[2].y != 0 ) intersection=fabs(tab_pxA[0].x-tab_pxA[2].x)*fabs(tab_pxA[0].y - b->ymax);	
    	if (tab_pxA[2].x != 0 && tab_pxA[2].y != 0 && tab_pxA[3].x != 0 && tab_pxA[3].y != 0 ) intersection=fabs(tab_pxA[2].y-tab_pxA[3].y)*fabs(tab_pxA[2].x - b->xmin);
    	if (tab_pxA[1].x != 0 && tab_pxA[1].y != 0 && tab_pxA[3].x != 0 && tab_pxA[3].y != 0 ) intersection=fabs(tab_pxA[1].x-tab_pxA[3].x)*fabs(tab_pxA[1].y - b->ymin);
    }
    else if (nb_pxB==2){
    	if (tab_pxB[0].x != 0 && tab_pxB[0].y != 0 && tab_pxB[1].x != 0 && tab_pxB[1].y != 0 ) { intersection=fabs(tab_pxB[0].y-tab_pxB[1].y)*fabs(tab_pxB[0].x - a->xmax); }
    	if (tab_pxB[0].x != 0 && tab_pxB[0].y != 0 && tab_pxB[2].x != 0 && tab_pxB[2].y != 0 ) { intersection=fabs(tab_pxB[0].x-tab_pxB[2].x)*fabs(tab_pxB[0].y - a->ymax); }	
    	if (tab_pxB[2].x != 0 && tab_pxB[2].y != 0 && tab_pxB[3].x != 0 && tab_pxB[3].y != 0 ) { intersection=fabs(tab_pxB[2].y-tab_pxB[3].y)*fabs(tab_pxB[2].x - a->xmin); }
    	if (tab_pxB[1].x != 0 && tab_pxB[1].y != 0 && tab_pxB[3].x != 0 && tab_pxB[3].y != 0 ) { intersection=fabs(tab_pxB[1].x-tab_pxB[3].x)*fabs(tab_pxB[1].y - a->ymin); }  
    }    
    else if (nb_pxA==4)	intersection=area_A;
    else if (nb_pxB==4)	intersection=area_B;    	
   
    float recouvrement=(float)(intersection)/(area_A + area_B - (float)(intersection));
    if (recouvrement<a->tol_box) return 1;

    IplImage* frame_BW = (IplImage*)userdata;
    IplImage* img_b = cvCloneImage(get_img_OCR_Image(b->im_average_mask_t));
    cvThreshold(img_b, img_b, round_me(b->thr_med), a->max_thr, CV_THRESH_TOZERO);                                            

    IplImage* im_diff = cvCloneImage(get_img_OCR_Image(b->im_average_mask_t));

    cvSetImageROI(frame_BW, cvRect(b->xmin_avg_t, b->ymin_avg_t, b->xmax_avg_t-b->xmin_avg_t, b->ymax_avg_t-b->ymin_avg_t));
	IplImage* img_a = cvCreateImage(cvSize(b->xmax_avg_t-b->xmin_avg_t, b->ymax_avg_t-b->ymin_avg_t), frame_BW->depth, 1);
	cvCopy(frame_BW, img_a, NULL);
	cvResetImageROI(frame_BW);
	
	cvRectangle(img_a,cvPoint(0,0),cvPoint(img_a->width-1,img_a->height-1),cvScalar(0,0,0,0),1,4,0);
	cvRectangle(img_a,cvPoint(1,1),cvPoint(img_a->width-2,img_a->height-2),cvScalar(0,0,0,0),1,4,0);
                               
    cvThreshold(img_a, img_a, round_me(b->thr_med), a->max_thr, CV_THRESH_TOZERO);                                            

    cvAbsDiff(img_b, img_a, im_diff);
    double v = cvAvg(im_diff, NULL).val[0];
    
    cvReleaseImage(&img_a);
    cvReleaseImage(&img_b);
    cvReleaseImage(&im_diff);
   
    if (v>a->max_val_diff_b2b) return 1;
    return 0;
}

// to compare 2 box by there start frame to sort them, function for opencv
int cmp_box_by_frame( const void* _a, const void* _b, void* userdata )  {      //compare 2 boxes by there start frame to sort them
    box* a = *(box**)_a;
    box* b = *(box**)_b;
    int start_frame_diff = a->start_frame - b->start_frame ;
    int stop_frame_diff = a->stop_frame - b->stop_frame ;
    return start_frame_diff ? start_frame_diff : stop_frame_diff;
}

// to compare 2 box by their threshold, function for opencv
int cmp_thr( const void* _a, const void* _b, void* userdata ) {
    int* a = (int*) _a;
    int* b = (int*) _b;
    int diff = *a - *b ;
    if (diff == 0) return 0;
    if (diff <0 ) return -1;
    return 1;
}

// process transcription of a boxes
void transcription_box(box *pt_search_box, LOOV_params* param){
	transcription_oi(pt_search_box->im_average_mask_t, param);  
	pt_search_box->nb_transcription++;
	pt_search_box->total_transcription = pt_search_box->total_transcription + strlen(pt_search_box->im_average_mask_t->text);
	float conf=pt_search_box->im_average_mask_t->confidence;
	int i;
	for (i=0;i<pt_search_box->seq_im->total;i++){   
    	OCR_Image * oi = *(OCR_Image**)cvGetSeqElem(pt_search_box->seq_im,i);
    	transcription_oi(oi, param);  
		pt_search_box->nb_transcription++;
		pt_search_box->total_transcription = pt_search_box->total_transcription + strlen(oi->text);
		conf=conf+oi->confidence;
    }
	conf=conf/(pt_search_box->seq_im->total+1);
}

// update boxes
void update_box(CvSeq* seq_box, int idx, int frameNum, int threshold_found, box* pt_rect_box, int nb_cc, IplImage* frame_BW, IplImage* im_box, LOOV_params* param){
    box* pt_search_box = *(box**)cvGetSeqElem(seq_box, idx);

    pt_search_box->stop_frame = frameNum;
    pt_search_box->current_thr = threshold_found;
    pt_search_box->nb_img_detect_avg++;
    pt_search_box->nb_img_detect_avg_t++;
    
    float d = pt_search_box->nb_img_detect_avg_t;

    pt_search_box->ymin_current=pt_rect_box->ymin;
    pt_search_box->xmin_current=pt_rect_box->xmin;
    pt_search_box->ymax_current=pt_rect_box->ymax;
    pt_search_box->xmax_current=pt_rect_box->xmax;
    pt_search_box->ymin=pt_search_box->ymin*d/(d+1)+pt_rect_box->ymin/(d+1);
    pt_search_box->xmin=pt_search_box->xmin*d/(d+1)+pt_rect_box->xmin/(d+1);
    pt_search_box->ymax=pt_search_box->ymax*d/(d+1)+pt_rect_box->ymax/(d+1);
    pt_search_box->xmax=pt_search_box->xmax*d/(d+1)+pt_rect_box->xmax/(d+1);
    
    cvSeqSort( pt_search_box->seq_thr_t, cmp_thr, 0);
    int* thr_med = (int*)cvGetSeqElem( pt_search_box->seq_thr_t, (int)(pt_search_box->nb_img_detect_avg_t/2) ); 
    pt_search_box->thr_med=*thr_med;
    
    pt_search_box->nb_cc=pt_search_box->nb_cc*d/(d+1)+(float)nb_cc/(d+1);

    cvSeqPush(pt_search_box->seq_thr, &threshold_found);
    cvSeqPush(pt_search_box->seq_thr_t, &threshold_found);
    
    if (pt_search_box->stop_frame-pt_search_box->start_frame<=2*param->fr_reco){                //initialisate the average total image if the box have temporal stability
        pt_search_box->ymin_avg_t=round_me(pt_search_box->ymin);
        pt_search_box->xmin_avg_t=round_me(pt_search_box->xmin);
        pt_search_box->ymax_avg_t=round_me(pt_search_box->ymax);
        pt_search_box->xmax_avg_t=round_me(pt_search_box->xmax);
        cvSetImageROI(frame_BW, cvRect(pt_search_box->xmin_avg_t, pt_search_box->ymin_avg_t, pt_search_box->xmax_avg_t-pt_search_box->xmin_avg_t, pt_search_box->ymax_avg_t-pt_search_box->ymin_avg_t));
        IplImage* im_average_mask_t = cvCreateImage(cvSize(pt_search_box->xmax_avg_t-pt_search_box->xmin_avg_t, pt_search_box->ymax_avg_t-pt_search_box->ymin_avg_t), frame_BW->depth, 1);
        cvCopy(frame_BW, im_average_mask_t, NULL);
        cvResetImageROI(frame_BW);
        cvRectangle(im_average_mask_t,cvPoint(0,0),cvPoint(im_average_mask_t->width-1,im_average_mask_t->height-1),cvScalar(0,0,0,0),1,4,0);
        cvRectangle(im_average_mask_t,cvPoint(1,1),cvPoint(im_average_mask_t->width-2,im_average_mask_t->height-2),cvScalar(0,0,0,0),1,4,0);

        change_img_OCR_Image(pt_search_box->im_average_mask_t,im_average_mask_t);                                     
    }
    
    if (pt_search_box->stop_frame-pt_search_box->start_frame>2*param->fr_reco){                    //calculate the average total image
        cvSetImageROI(frame_BW, cvRect(pt_search_box->xmin_avg_t, pt_search_box->ymin_avg_t, pt_search_box->xmax_avg_t-pt_search_box->xmin_avg_t, pt_search_box->ymax_avg_t-pt_search_box->ymin_avg_t));
        IplImage* im_average_mask_t = cvCreateImage(cvSize(pt_search_box->xmax_avg_t-pt_search_box->xmin_avg_t, pt_search_box->ymax_avg_t-pt_search_box->ymin_avg_t), frame_BW->depth, 1);
        cvCopy(frame_BW, im_average_mask_t, NULL);
        cvResetImageROI(frame_BW);
        cvRectangle(im_average_mask_t,cvPoint(0,0),cvPoint(im_average_mask_t->width-1,im_average_mask_t->height-1),cvScalar(0,0,0,0),1,4,0);
        cvRectangle(im_average_mask_t,cvPoint(1,1),cvPoint(im_average_mask_t->width-2,im_average_mask_t->height-2),cvScalar(0,0,0,0),1,4,0);
        cvAddWeighted(get_img_OCR_Image(pt_search_box->im_average_mask_t),d/(d+1),im_average_mask_t,1/(d+1),0,get_img_OCR_Image(pt_search_box->im_average_mask_t));
        cvReleaseImage(&im_average_mask_t);
    }
    //calculate the average image
    cvSetImageROI(frame_BW, cvRect(pt_search_box->xmin_avg, pt_search_box->ymin_avg, pt_search_box->xmax_avg-pt_search_box->xmin_avg, pt_search_box->ymax_avg-pt_search_box->ymin_avg));
    IplImage* im_average_mask = cvCreateImage(cvSize(pt_search_box->xmax_avg-pt_search_box->xmin_avg, pt_search_box->ymax_avg-pt_search_box->ymin_avg), frame_BW->depth, 1);
    cvCopy(frame_BW, im_average_mask, NULL);
    cvResetImageROI(frame_BW);
    cvRectangle(im_average_mask,cvPoint(0,0),cvPoint(im_average_mask->width-1,im_average_mask->height-1),cvScalar(0,0,0,0),1,4,0);
    cvRectangle(im_average_mask,cvPoint(1,1),cvPoint(im_average_mask->width-2,im_average_mask->height-2),cvScalar(0,0,0,0),1,4,0);
        
    d=((pt_search_box->stop_frame-pt_search_box->start_frame) % param->fr_reco)*param->fr_reco;
    cvAddWeighted(get_img_OCR_Image(pt_search_box->im_average_mask),d/(d+1),im_average_mask,1/(d+1),0,get_img_OCR_Image(pt_search_box->im_average_mask));
    cvReleaseImage(&im_average_mask);
    cvReleaseImage(&(pt_search_box->im_current));
    pt_search_box->im_current=cvCloneImage(im_box);
    free_box(pt_rect_box);
}






