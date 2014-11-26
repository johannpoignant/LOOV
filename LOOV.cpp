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

#define CV_NO_BACKWARD_COMPATIBILITY

#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <time.h>

#include "tools.h" 
#include "params.h"
#include "text_box_detection.h"

int main(int argc, char *argv[]) {
	/* initialisation of the parameters  */
    LOOV_params *param=alloc_init_LOOV_params();
    param = parse_arg(param, argc, argv);
	/* initialisation of the boxes sequences */
    CvMemStorage* storage_box = cvCreateMemStorage(0);
    CvSeq* seq_box = cvCreateSeq( 0, sizeof(CvSeq), sizeof(box*), storage_box);							// list of boxes that are shown in the current frame
    CvMemStorage* storage_box_final = cvCreateMemStorage(0);
    CvSeq* seq_box_final = cvCreateSeq( 0, sizeof(CvSeq), sizeof(box*), storage_box_final);				// boxes list  that no longer appear

    if (param->videoName==NULL) { fprintf(stderr,"enter video name after parameter -v\n"); exit(0); }
    CvCapture* capture = cvCaptureFromFile(param->videoName);    										// read video
    if (!capture)        { printf("error on video %s\n",param->videoName); exit(1); }
    cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, param->startFrame);  							// get video property
    IplImage* frame_temp = cvQueryFrame( capture );   													// get the first frame    

	/* computed parameters depending on the image size */
    int video_depth=1;    																				
    for (int i=0;i<frame_temp->depth;i++) video_depth=video_depth*2;									// find the max threshold
    param->max_thr = video_depth-1;
    param->it_connected_caractere = round_me((float)frame_temp->width*param->aspect_ratio*param->it_connected_caractere);       
    param->y_min_size_text = round_me((float)frame_temp->height*param->y_min_size_text);
    param->x_min_size_text = round_me((float)frame_temp->width*param->aspect_ratio*param->x_min_size_text);
	
	/* read mask image, to process only a part of the images */
    IplImage* frame=cvCreateImage(cvSize(frame_temp->width*param->aspect_ratio, frame_temp->height), frame_temp->depth, frame_temp->nChannels);
    cvResize(frame_temp, frame, CV_INTER_CUBIC);
    IplImage* im_mask=0;    
    if (param->path_im_mask!=NULL) {
        im_mask=cvLoadImage(param->path_im_mask, CV_LOAD_IMAGE_GRAYSCALE);
        if ((frame->width!=im_mask->width) || (frame->height!=im_mask->height)){
            IplImage* im_mask_resize = cvCreateImage(cvSize(frame->width, frame->height),im_mask->depth, 1);  // resize mask to the images video size
            cvResize(im_mask, im_mask_resize, CV_INTER_CUBIC);
            cvReleaseImage(&im_mask);
            im_mask = cvCloneImage(im_mask_resize);
            cvReleaseImage(&im_mask_resize);
        }
    }   
    
    printf("processing of frames from %d to %d\n", param->startFrame, param->startFrame+param->nbFrame);
    
    IplImage* frame_BW=cvCreateImage(cvSize(frame_temp->width*param->aspect_ratio, frame_temp->height), frame_temp->depth, 1);
    IplImage* frame_BW_temp=cvCreateImage(cvSize(frame_temp->width, frame_temp->height), frame_temp->depth, 1);   
    int frameNum=param->startFrame;
    while((frameNum<param->startFrame+param->nbFrame) && (frame_temp = cvQueryFrame( capture ))) {  // capture the current frame and put it in frame_temp
        frameNum++;
        if( frame_temp ) {	
            cvCvtColor(frame_temp, frame_BW_temp, CV_RGB2GRAY);			                            // convert frame from color to gray
            cvResize(frame_temp, frame, CV_INTER_CUBIC);                                            // resize for aspect ratio
            cvResize(frame_BW_temp, frame_BW, CV_INTER_CUBIC);
            cvCvtColor(frame, frame_BW, CV_RGB2GRAY);
			IplImage* im = cvCloneImage(frame_BW);			
            im = sobel_double_H(im, param);															// find edge of characters		
            if (param->path_im_mask!=NULL) cvAnd(im,im_mask,im, NULL);								// apply mask if it exists
            im = connected_caractere(im, param);													// connect edges of a same line
            im = delete_horizontal_bar(im, param);													// filter noise on the resulting image
            im = delete_vertical_bar(im, param);													// filter noise on the resulting image
            if (param->path_im_mask!=NULL) cvAnd(im,im_mask,im, NULL);								// apply mask if it exists
            spatial_detection_box(im, seq_box, frameNum, frame_BW, frame, frame, im_mask, param); 	// Detect boxes spatial position
            temporal_detection_box(seq_box, seq_box_final, frameNum, frame_BW, im_mask, param);     // Temporal tracking of the boxes
            cvReleaseImage(&im);
        }
    }     
    cvReleaseImage(&frame_BW);
    cvReleaseImage(&im_mask);

    /* finish the transcriptin of the boxes in seq_box */
    for (int i=0;i<seq_box->total;i++){
        box* pt_search_box = *(box**)cvGetSeqElem(seq_box, i);
        if (pt_search_box->stop_frame - pt_search_box->start_frame > param->min_duration_box) {         
            cvSeqPush(seq_box_final, &pt_search_box);                                               // copy boxes in seq_box_final
            cvSeqSort(pt_search_box->seq_thr_t, cmp_thr, 0);
            int* thr_med = (int*)cvGetSeqElem( pt_search_box->seq_thr_t, (int)(pt_search_box->nb_img_detect_avg_t/2) );   
            set_threshold_OCR_Image(pt_search_box->im_average_mask_t,*thr_med);                
            transcription_box(pt_search_box, param);                                                // process transcription of the boxes
            if (param->print_text == 1){                                                            // print transcription
                printf("box_%d img_avg ymin=%d ymax=%d xmin=%d xmax=%d " ,pt_search_box->num ,round_me(pt_search_box->ymin_avg), round_me(pt_search_box->xmin_avg), round_me(pt_search_box->ymax_avg), round_me(pt_search_box->xmax_avg));
                print_transcription_image(get_img_OCR_Image(pt_search_box->im_average_mask_t), round_me(pt_search_box->thr_med), param);
            }
        }
        else free_box(pt_search_box);
    }
            
    /* Write transcription in output_path+".OCR" file */
    char * file_txt_temp=sprintf_alloc("%s.OCR", param->output_path);
    FILE * file_txt = fopen(file_txt_temp, "w");
    free(file_txt_temp); 
    cvSeqSort( seq_box_final, cmp_box_by_frame, 0);
    for (int i=0;i<seq_box_final->total;i++){
        file_print_box(file_txt, *(box**)cvGetSeqElem(seq_box_final, i), param);   //
    }    
    fclose(file_txt);

    /* free memory */     
    for (int i=0;i<seq_box_final->total;i++){
        free_box(*(box**)cvGetSeqElem(seq_box_final, i));
    }    
    cvClearSeq(seq_box);
    cvReleaseMemStorage( &storage_box );
    cvReleaseImage(&im_mask);
    cvClearSeq(seq_box_final);
    cvReleaseMemStorage( &storage_box_final );
    cvReleaseCapture( &capture ); 

    return 0;
}

