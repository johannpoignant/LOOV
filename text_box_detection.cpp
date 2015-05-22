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

#include "box_image.h"
#include "text_box_detection.h"

// Find global threshold with Sauvola binarisation algorithm
int sauvola(IplImage* im_thr, int video_depth){
    float k_sauvola=0.5;

	cvNot(im_thr,im_thr);
	float mean_sauv=0.0;
	float total_px_sauv=0;
	int nb_px_sauv=0;
    int val=0;
    float connect[video_depth];
    int x,y,i;
    for(i=0;i<video_depth;i++){
    	connect[i]=0;
    }
	for (x=0;x<im_thr->width;x++){
		for (y=0;y<im_thr->height;y++){
			val = (int)(cvGet2D(im_thr, y, x).val[0]); 
			if (val!=0) connect[val]++;
			connect[val]++;
		}
	}
	
	for (x=0;x<im_thr->width;x++){
		for (y=0;y<im_thr->height;y++){
			val = (int)(cvGet2D(im_thr, y, x).val[0]); 
			total_px_sauv+=val;
			nb_px_sauv++;
		}
	}	
	mean_sauv=total_px_sauv/nb_px_sauv;
	float ecart_type_sauv=0.0;
	for(i=0;i<video_depth-1;i++) ecart_type_sauv+=(connect[i]*(i-mean_sauv)*(i-mean_sauv));

	ecart_type_sauv/=total_px_sauv;
	ecart_type_sauv=sqrt(ecart_type_sauv);
	
	float thr_sauv=mean_sauv*(1+k_sauvola*(ecart_type_sauv/(video_depth/2)-1));  // Tsauvola = m (1 + k(s/R -1))	
	cvNot(im_thr,im_thr);
	return 255-round_me(thr_sauv);
}

// Find global threshold with Wolf binarisation algorithm
int wolf(IplImage* im_thr, int video_depth){
	cvNot(im_thr,im_thr);
	float mean_sauv=0.0;
	float total_px_sauv=0;
	float min_val=(float)video_depth;
	int nb_px_sauv=0;
    int val=0;
    float connect[video_depth];
    int x,y,i;
    for(i=0;i<video_depth;i++) connect[i]=0;
	for (x=0;x<im_thr->width;x++){
		for (y=0;y<im_thr->height;y++){
			val = (int)(cvGet2D(im_thr, y, x).val[0]); 
			if (val!=0) connect[val]++;
			connect[val]++;
			if (min_val > val) min_val = (float)(val);
		}
	}	
	for (x=0;x<im_thr->width;x++){
		for (y=0;y<im_thr->height;y++){
			val = (int)(cvGet2D(im_thr, y, x).val[0]); 
			total_px_sauv+=val;
			nb_px_sauv++;
		}
	}	
	mean_sauv=total_px_sauv/nb_px_sauv;
	float ecart_type_sauv=0.0;
	for(i=0;i<video_depth-1;i++){
		ecart_type_sauv+=(connect[i]*(i-mean_sauv)*(i-mean_sauv));
	}
	ecart_type_sauv/=total_px_sauv;
	ecart_type_sauv=sqrt(ecart_type_sauv);
	
	float thr_sauv= 0.5*mean_sauv + 0.5*min_val + 0.5*ecart_type_sauv/128*(mean_sauv-min_val);  	// Twolf = (1-a).m + a.M + a.(s/R).(m-M) ou a = 0.5
	cvNot(im_thr,im_thr);

	return 255-round_me(thr_sauv);
}

// Find threshold to delete noise in the image
double noise_image(IplImage* im, int* thr_otsu, LOOV_params* param){
    IplImage* im_temp = cvCloneImage(im);
    *thr_otsu = sauvola(im_temp, param->max_thr+1);

    int tab_UF[im->width*im->height];
    int tab[im->width*im->height];
    int x,y;
    for (x=0;x<im->width;x++) for (y=0;y<im->height;y++)  tab_UF[x+y*im->width] = -1; 
    for (x=0;x<im->width;x++){
        for (y=0;y<im->height;y++){             
             if (cvGet2D(im_temp, y, x).val[0]>=*thr_otsu){                  
                  if (y-1>=0)                               if (cvGet2D(im_temp, y-1, x).val[0]>=*thr_otsu)      Union(tab_UF, x+y*im->width, x+(y-1)*im->width);   
                  if (y+1<im->height)                       if (cvGet2D(im_temp, y+1, x).val[0]>=*thr_otsu)      Union(tab_UF, x+y*im->width, x+(y+1)*im->width);            
                  if (x-1>=0)                               if (cvGet2D(im_temp, y, x-1).val[0]>=*thr_otsu)      Union(tab_UF, x+y*im->width, x-1+y*im->width);            
                  if (x+1<im->width)                        if (cvGet2D(im_temp, y, x+1).val[0]>=*thr_otsu)      Union(tab_UF, x+y*im->width, x+1+y*im->width);

                  if (y-1>=0 && x-1>=0)                     if (cvGet2D(im_temp, y-1, x-1).val[0]>=*thr_otsu)    Union(tab_UF, x+y*im->width, x-1+(y-1)*im->width);   
                  if (y-1>=0 && x+1<im->width)              if (cvGet2D(im_temp, y-1, x+1).val[0]>=*thr_otsu)    Union(tab_UF, x+y*im->width, x+1+(y-1)*im->width);            
                  if (y+1<im->height && x-1>=0)             if (cvGet2D(im_temp, y+1, x-1).val[0]>=*thr_otsu)    Union(tab_UF, x+y*im->width, x-1+(y+1)*im->width);            
                  if (y+1<im->height && x+1<im->width)      if (cvGet2D(im_temp, y+1, x+1).val[0]>=*thr_otsu)    Union(tab_UF, x+y*im->width, x+1+(y+1)*im->width);
             }             
        }
    }
    double moy=0;
    int nb_moy=0;
    for (x=0;x<im->width;x++) {
        for (y=0;y<im->height;y++) {
            if (-tab_UF[x+y*im->width]>1){
                moy=moy-tab_UF[x+y*im->width];
                tab[nb_moy] = -tab_UF[x+y*im->width];
                nb_moy++;
            }
        }
    }
    if (nb_moy<round_me(im->width/im->height*param->density_coef_character)) return 0.0;
         
    moy/=nb_moy;
    double var=0;
    int i;
    for (i=0; i<nb_moy; i++)   var = var+(tab[i]-moy)*(tab[i]-moy);

    if (var!=0) {
        var/=nb_moy;
        var/=(im->height*im->height);                                    
    }
    cvReleaseImage(&im_temp);
    return var;    
}

// Apply a sobel filter to detect character edges
IplImage* sobel_double_H(IplImage* im, LOOV_params* param ){                                                                                //do filter sobel to detect edge of character
    IplImage* im_sobel1 = cvCreateImage(cvSize(im->width, im->height), IPL_DEPTH_16S, 1);                            //create temp image in 16 bit signed and put into the result of the sobel
    cvSobel(im, im_sobel1,1,0,1);
    cvConvertScaleAbs(im_sobel1, im, 1, 0);                                                                            //convert image in 8 bit unsigned with there Absolute value
    cvReleaseImage(&im_sobel1);                                                                                        //release temp image
    cvThreshold(im, im, param->threshold_sobel, param->max_thr, CV_THRESH_BINARY);                                                    //threshold image to keep just some edge found by sobel
    return im;
}

// Connect character from Sobel image with dilatation and erosion
IplImage* connected_caractere(IplImage* im, LOOV_params* param){                                                                        //Connect character from Sobel image with dilatation and erosion
    IplConvKernel* element = cvCreateStructuringElementEx(3, 1, 1, 0, CV_SHAPE_RECT, NULL);                            //create matrice for the operation dilatation and erosion
    cvDilate(im, im, element, param->it_connected_caractere);
    cvErode(im, im, element, param->it_connected_caractere);
    cvReleaseStructuringElement(&element);                                                                            //release matrice
    return im;
}

// Delete horizontal noise
IplImage* delete_horizontal_bar(IplImage* im, LOOV_params* param){                                                                       //Delete horizontal bar of 1 px high
	cvRectangle(im,cvPoint(0,0),cvPoint(im->width-1,im->height-1),cvScalar(0,0,0,0),2,4,0) ;                      //draw line around the image because the erode operation do not correctly do ???
	IplConvKernel* element1 = cvCreateStructuringElementEx(1, 2, 0, 1, CV_SHAPE_RECT, NULL);                       //create a matrice for the operation dilatation and erosion
	cvErode(im, im, element1, param->y_min_size_text/4) ;
    cvReleaseStructuringElement(&element1); 

	IplConvKernel* element2 = cvCreateStructuringElementEx(1, 2, 0, 0, CV_SHAPE_RECT, NULL); 
	cvDilate(im, im, element2, param->y_min_size_text/4);
    cvReleaseStructuringElement(&element2); //release matrice
    return im;
}

// Delete vertical noise 
IplImage* delete_vertical_bar(IplImage* im, LOOV_params* param){                                                                        //Delete vertical bar
    cvRectangle(im,cvPoint(0,0),cvPoint(im->width-1,im->height-1),cvScalar(0,0,0,0),2,4,0) ;                    //draw line around the image because the erode operation do not correctly do ???
    IplConvKernel* element1 = cvCreateStructuringElementEx(2, 1, 1, 0, CV_SHAPE_RECT, NULL);                        //create matrice for the operation dilatation and erosion
    cvErode(im, im, element1, param->x_min_size_text/4);
    cvReleaseStructuringElement(&element1); 
    
    IplConvKernel* element2 = cvCreateStructuringElementEx(2, 1, 0, 0, CV_SHAPE_RECT, NULL);
    cvDilate(im, im, element2, param->x_min_size_text/4);
    cvReleaseStructuringElement(&element2);                                                                        //release matrice
    return im;
}

// refine spatial detection of boxes
void refine_detection(int* threshold_found, int* ymin, int* ymax, int* xmin, int* xmax, IplImage* frame_BW, int* ymin_final, int* ymax_final, int* xmin_final, int* xmax_final, LOOV_params* param){
    int threshold_found2 = *threshold_found;
    if (param->threshold_find_detection!=-1) threshold_found2 = param->threshold_find_detection;

    *ymin=*ymin-param->margin_coarse_detection_Y*(*ymax-*ymin);
    *xmin=*xmin-param->margin_coarse_detection_X*(*ymax-*ymin);
    *ymax=*ymax+param->margin_coarse_detection_Y*(*ymax-*ymin);
    *xmax=*xmax+param->margin_coarse_detection_X*(*ymax-*ymin);
    
    if (*ymin<0) *ymin=0;
    if (*xmin<0) *xmin=0;
    if (*ymax>=frame_BW->height) *ymax=frame_BW->height;
    if (*xmax>=frame_BW->width)  *xmax=frame_BW->width;
   
    cvSetImageROI(frame_BW, cvRect(*xmin_final, *ymin, *xmax_final-*xmin_final, *ymax-*ymin));                    //get the ROI
    IplImage* im_temp = cvCreateImage(cvSize(*xmax_final-*xmin_final, *ymax-*ymin), frame_BW->depth, 1);
    cvCopy(frame_BW, im_temp, NULL);                                                                              //copy in a new image
    cvResetImageROI(frame_BW);                                                                                                                  

    cvThreshold(im_temp, im_temp, threshold_found2, param->max_thr, CV_THRESH_TOZERO); 

    IplConvKernel* element = cvCreateStructuringElementEx(3, 1, 1, 0, CV_SHAPE_RECT, NULL);
    cvDilate(im_temp, im_temp, element, im_temp->width);                                                          //draws a horizontal line where there are text
    cvReleaseStructuringElement(&element);
    
    char found_ymin_final=0;
    int j;
    for (j=(int)(im_temp->height/2); j>=0; j--){  
        if (cvGet2D(im_temp, j, (int)(im_temp->width/2)).val[0]<=threshold_found2){
            *ymin_final=*ymin+j+1;
            found_ymin_final=1;
            break;
        }                     
    }
    char found_ymax_final=0;
    for (j=(int)(im_temp->height/2); j<im_temp->height; j++){
        if (cvGet2D(im_temp, j, (int)(im_temp->width/2)).val[0]<=threshold_found2){
            *ymax_final=*ymin+j;
            found_ymax_final=1;
            break;
        } 
    }                    
    cvReleaseImage(&im_temp);

    if (!found_ymax_final || !found_ymin_final){                                                                  //if we don't found box edges, we try with mean color line
        cvSetImageROI(frame_BW, cvRect(*xmin_final, *ymin, *xmax_final-*xmin_final, *ymax-*ymin));                //get the ROI
        im_temp = cvCreateImage(cvSize(*xmax_final-*xmin_final, *ymax-*ymin), frame_BW->depth, 1);
        cvCopy(frame_BW, im_temp, NULL);                                                                          //copy in a new image
        cvResetImageROI(frame_BW);
        
        double val_mean;
        double val_current;
        if (!found_ymin_final){
            val_mean = cvAvg(im_temp).val[0];
            CvMat * mat_temp=cvCreateMat(1, im_temp->width, im_temp->depth) ;
            for (j=(int)(im_temp->height/2); j>=0; j--){  
                val_current = cvAvg(cvGetRow(im_temp, mat_temp, j)).val[0];
                if (val_current<val_mean){
                    *ymin_final=*ymin+j+1;
                    val_mean = val_current;
                }                               
            }  
            cvReleaseMat(&mat_temp);
        }

        if (!found_ymax_final){
            val_mean = cvAvg(im_temp).val[0];
            CvMat * mat_temp=cvCreateMat(1, im_temp->width, im_temp->depth) ;
            for (j=(int)(im_temp->height/2); j<im_temp->height; j++){
                val_current = cvAvg(cvGetRow(im_temp, mat_temp, j)).val[0];
                if (val_current<val_mean){
                    *ymax_final=*ymin+j;
                    val_mean = val_current;
                }                                  
            }
            cvReleaseMat(&mat_temp);
        }     
        cvReleaseImage(&im_temp);
    }

    if (*ymax_final-*ymin_final>=param->y_min_size_text){
        cvSetImageROI(frame_BW, cvRect(*xmin, *ymin_final, *xmax-*xmin, *ymax_final-*ymin_final));                //get the ROI
        IplImage* im_temp2 = cvCreateImage(cvSize(*xmax-*xmin, *ymax_final-*ymin_final), frame_BW->depth, 1);
        cvCopy(frame_BW, im_temp2, NULL);                                                                         //copy in a new image
        cvResetImageROI(frame_BW);

        cvThreshold(im_temp2, im_temp2, threshold_found2, param->max_thr, CV_THRESH_TOZERO);          
        element = cvCreateStructuringElementEx(1, 3, 0, 1, CV_SHAPE_RECT, NULL);

        cvDilate(im_temp2, im_temp2, element, im_temp2->height);                                                  //draws a vertical line where there are text
        cvReleaseStructuringElement(&element);
        
        for (j=0; j<im_temp2->width; j++){          
            if (cvGet2D(im_temp2, (int)(im_temp2->height/2), j).val[0]>=threshold_found2){
                *xmin_final=*xmin+j;
                break;
            } 
        }
        for (j=im_temp2->width-1; j>=0; j--){ 
            if (cvGet2D(im_temp2, (int)(im_temp2->height/2), j).val[0]>=threshold_found2){                     
                *xmax_final=*xmin+j+1;
                break;
            }
        }
        cvReleaseImage(&im_temp2);
    }    

}

// coarse spatial detection of boxes
void spatial_detection_box(IplImage* im, CvSeq* seq_box, int frameNum, IplImage* frame_BW_original, IplImage* frame_4, IplImage* frame_5, IplImage* im_mask, LOOV_params* param){        //Find coordinates of boxes
	CvSeq* seq_contour=NULL;                                                                                                // create sequence who contain contour list
    CvSeq* current_contour=NULL;                                                                                            // create sequence who contain point list of the sequence
    int i, j, idx;
    cvRectangle(im,cvPoint(0,0),cvPoint(im->width-1,im->height-1),cvScalar(0,0,0,0),2,4,0) ;                                // draw line around the image because findcontour do not work correctly ???
    CvMemStorage* storage=cvCreateMemStorage(0);                                                                            // create memory storage for the contour sequence
    cvFindContours(im, storage, &seq_contour, sizeof(CvContour), CV_RETR_LIST, CV_LINK_RUNS, cvPoint(0,0));                 // find all contours of an image and put it in seq_contour
    //coarse detection
    int ymin=im->height-2;
    int xmin=im->width-2;
    int ymax=2;
    int xmax=2;
    for(current_contour=seq_contour; current_contour!=NULL; current_contour=current_contour->h_next ) {                     // for each contour in contour list put it in current_contour
        ymin=im->height-2;
        xmin=im->width-2;
        ymax=2;
        xmax=2;
        cvSeqSort( current_contour, cmp_opencv_point, 0);                                                                   // compare 2 point to sort the sequence of point
        for( i=0; i<current_contour->total; i=i+2 ) {                                                                       // each pair of point corresponds to a line in the contour
            CvPoint* p1 = (CvPoint*)cvGetSeqElem(current_contour, i );
            CvPoint* p2 = (CvPoint*)cvGetSeqElem(current_contour, i+1 );
            if ((p2->x-p1->x) >= param->x_min_size_text){                                                                   // if the line has a width greater than the minimum width
                if (p1->y<ymin) ymin = p1->y;                                                                               // find the top left corner
                if (p1->x<xmin) xmin = p1->x;                                                                               // find the top left corner
                if (p2->y>ymax) ymax = p2->y;                                                                               // find the bottom right corner
                if (p2->x>xmax) xmax = p2->x;                                                                               // find the bottom right corner
            }
        } 
        
        xmin = xmin + 2 + 1;                                                                                                // offset due to previous operation                                    
        xmax = xmax + 2 + 1;  
        ymin = ymin + 1;                                               
        ymax = ymax + 1;
        
        if (ymin<2) ymin=2;
        if (xmin<2) xmin=2;
        if (ymax>im->height-2) ymax=im->height-2;
        if (xmax>im->width-2) xmax=im->width-2;
        if (ymax-ymin>=param->y_min_size_text && (float)(xmax-xmin)>=param->ratio_width_height*(float)(ymax-ymin)){         //if the box has the good geometry
            int ymin_final=ymin;
            int xmin_final=xmin;
            int ymax_final=ymax;
            int xmax_final=xmax;

            IplImage* frame_BW = cvCloneImage(frame_BW_original);
            if (param->path_im_mask!=NULL) cvAnd(frame_BW,im_mask,frame_BW, NULL);

            //text color detection (black on white or vice versa)
            cvSetImageROI(frame_BW, cvRect(xmin-2, ymin-2, xmax-xmin+4, ymax-ymin+4));                                      //get the ROI
            IplImage* im_temp = cvCreateImage(cvSize(xmax-xmin+4, ymax-ymin+4), frame_BW->depth, 1);
            cvCopy(frame_BW, im_temp, NULL); 
            cvResetImageROI(frame_BW);
            IplImage* im_temp_mask = cvCreateImage(cvSize(xmax-xmin+4, ymax-ymin+4), frame_BW->depth, 1);
            if (param->path_im_mask!=NULL) {
                cvSetImageROI(im_mask, cvRect(xmin-2, ymin-2, xmax-xmin+4, ymax-ymin+4));                                   //get the ROI                
                cvCopy(im_mask, im_temp_mask, NULL);
                cvResetImageROI(im_mask);
            }             
            
            if (param->text_white == 0) cvNot(frame_BW, frame_BW);
            else if (param->text_white == -1){
                double var1=0;
                double var2=0;  
                int thr_otsu_var1=0;
                int thr_otsu_var2=0; 
                var1 = noise_image(im_temp, &thr_otsu_var1, param);
                if (var1>param->max_var || var1==0.0){
                     cvNot(im_temp, im_temp); 
                     if (param->path_im_mask!=NULL) cvAnd(im_temp,im_temp_mask,im_temp, NULL);
                     var2 = noise_image(im_temp, &thr_otsu_var2, param);
                     if ((var1<var2 && var1!=0.0) || var2==0.0){
                        cvNot(im_temp, im_temp); 
                        if (param->path_im_mask!=NULL) cvAnd(im_temp,im_temp_mask,im_temp, NULL);
                     }
                     else{
                        cvNot(frame_BW, frame_BW);
                        if (param->path_im_mask!=NULL) cvAnd(frame_BW,im_mask,frame_BW, NULL);
                     }
                }
            }

            int threshold_found = -1;            
            int nb_cc=0;
            if (param->type_threshold==-1) {
                 threshold_found = cvThreshold(im_temp, im_temp, param->max_thr, param->max_thr, CV_THRESH_TOZERO+CV_THRESH_OTSU);
                 threshold_found = param->coef_increase_thr_otsu*threshold_found;
            }
            else if (param->type_threshold==-2) threshold_found = sauvola(im_temp, param->max_thr+1);
            else if (param->type_threshold==-3) threshold_found = wolf(im_temp, param->max_thr+1);
            else threshold_found=param->type_threshold;
            
            cvReleaseImage(&im_temp);
            cvReleaseImage(&im_temp_mask);
               
            if (threshold_found!=0)  refine_detection(&threshold_found, &ymin, &ymax, &xmin, &xmax, frame_BW, &ymin_final, &ymax_final, &xmin_final, &xmax_final, param);

            if (ymax_final-ymin_final>=param->y_min_size_text && (float)(xmax_final-xmin_final)>=param->ratio_width_height*(float)(ymax_final-ymin_final) && threshold_found!=0){    //if the box has the good geometry
            	ymin_final=ymin_final-2;
                xmin_final=xmin_final-2;
                ymax_final=ymax_final+2;
                xmax_final=xmax_final+2;                
                
                if (ymin_final<0)                    ymin_final=0;  
                if (xmin_final<0)                    xmin_final=0;
                if (ymax_final>=frame_BW->height)    ymax_final=frame_BW->height;
                if (xmax_final>=frame_BW->width)     xmax_final=frame_BW->width;
                cvSetImageROI(frame_BW, cvRect(xmin_final, ymin_final, xmax_final-xmin_final, ymax_final-ymin_final));
                IplImage* im_box = cvCreateImage(cvSize(xmax_final-xmin_final, ymax_final-ymin_final), frame_BW->depth, 1);
                cvCopy(frame_BW, im_box, NULL);
                cvResetImageROI(frame_BW);
                cvRectangle(im_box,cvPoint(0,0),cvPoint(im_box->width-1,im_box->height-1),cvScalar(0,0,0,0),1,4,0);
                cvRectangle(im_box,cvPoint(1,1),cvPoint(im_box->width-2,im_box->height-2),cvScalar(0,0,0,0),1,4,0);  
                                                                     
                box* pt_rect_box= create_init_box(frameNum, frameNum, (float)ymin_final, (float)xmin_final, (float)ymax_final, (float)xmax_final, threshold_found, (float)nb_cc, im_box, im_box, im_box, param);  
                idx=0;

                if ((seq_box->total==0) || (!cvSeqSearch(seq_box, &pt_rect_box, cmp_box, 0, &idx, frame_BW )))    cvSeqPush(seq_box, &pt_rect_box);  // if the box not in the list add the box
                else update_box(seq_box, idx, frameNum, threshold_found, pt_rect_box, nb_cc, frame_BW, im_box, param);                               // else update the boxes position
                
                cvReleaseImage(&im_box);                
            }
            cvReleaseImage(&frame_BW);
        }        
    }
    cvReleaseMemStorage(&storage);
}

// temporal tracking of boxes
void temporal_detection_box(CvSeq* seq_box, CvSeq* seq_box_final, int frameNum, IplImage* frame_BW, IplImage* im_mask, LOOV_params* param){
    int i;
    for (i=0;i<seq_box->total;i++){
        box* pt_search_box = *(box**)cvGetSeqElem(seq_box, i);
        // try to recover miss detection of boxes
        if ((pt_search_box->stop_frame - pt_search_box->start_frame > param->min_duration_box / 2) && (pt_search_box->stop_frame < frameNum-param->space_btfd + 1)){
            cvSetImageROI(frame_BW, cvRect(pt_search_box->xmin_avg_t, pt_search_box->ymin_avg_t, pt_search_box->xmax_avg_t-pt_search_box->xmin_avg_t, pt_search_box->ymax_avg_t-pt_search_box->ymin_avg_t));
            if (param->path_im_mask!=NULL) cvSetImageROI(im_mask, cvRect(pt_search_box->xmin_avg_t, pt_search_box->ymin_avg_t, pt_search_box->xmax_avg_t-pt_search_box->xmin_avg_t, pt_search_box->ymax_avg_t-pt_search_box->ymin_avg_t));
            
            IplImage* im_box = cvCreateImage(cvSize(pt_search_box->xmax_avg_t-pt_search_box->xmin_avg_t, pt_search_box->ymax_avg_t-pt_search_box->ymin_avg_t), frame_BW->depth, 1);
            cvCopy(frame_BW, im_box, NULL);
            if (param->path_im_mask!=NULL) cvAnd(im_box,im_mask,im_box, NULL);

            IplImage* im_temp = cvCreateImage(cvSize(pt_search_box->xmax_avg_t-pt_search_box->xmin_avg_t, pt_search_box->ymax_avg_t-pt_search_box->ymin_avg_t), frame_BW->depth, 1);
            cvCopy(frame_BW, im_temp, NULL); 
            if (param->path_im_mask!=NULL) cvAnd(im_temp,im_mask,im_temp, NULL);

            cvResetImageROI(frame_BW);

            int threshold_found = -1;
                        
            if (param->text_white == 0){             
                cvNot(frame_BW, frame_BW);
            }
            else if (param->text_white == -1){            
                int thr_otsu=0;
                int thr_otsu_var1=0;
                int thr_otsu_var2=0;
                double var=0;
                double var1=0;
                double var2=0;                
            
                var1 = noise_image(im_temp, &thr_otsu_var1, param);
                if (var1>param->max_var || var1==0.0){
                    cvNot(im_temp, im_temp); 
                    if (param->path_im_mask!=NULL) cvAnd(im_temp,im_mask,im_temp, NULL);
                    var2 = noise_image(im_temp, &thr_otsu_var2, param);
                     
                    if ((var1<var2 && var1!=0.0) || var2==0.0){
                        cvNot(im_temp, im_temp); 
                        if (param->path_im_mask!=NULL) cvAnd(im_temp,im_mask,im_temp, NULL);
                        var=var1;
                        thr_otsu=thr_otsu_var1;
                    }
                    else{
                        cvNot(frame_BW, frame_BW);
                        if (param->path_im_mask!=NULL) cvAnd(im_box,im_mask,im_box, NULL);
                        var=var2;
                        thr_otsu=thr_otsu_var2;
                    }
                }
                else{
                    var=var1;
                    thr_otsu=thr_otsu_var1;
                }
            }
            
            if (param->type_threshold==-1){
                 threshold_found = (cvThreshold(im_temp, im_temp, param->max_thr, param->max_thr, CV_THRESH_TOZERO+CV_THRESH_OTSU));
                 threshold_found = param->coef_increase_thr_otsu*threshold_found;
            }
            else if (param->type_threshold==-2) 	threshold_found = sauvola(im_temp, param->max_thr+1);
            else if (param->type_threshold==-3) 	threshold_found = wolf(im_temp, param->max_thr+1);
            else threshold_found=param->type_threshold;
            cvReleaseImage(&im_temp);

            cvRectangle(im_box,cvPoint(0,0),cvPoint(im_box->width-1,im_box->height-1),cvScalar(0,0,0,0),1,4,0);
            cvRectangle(im_box,cvPoint(1,1),cvPoint(im_box->width-2,im_box->height-2),cvScalar(0,0,0,0),1,4,0);
            
            cvThreshold(im_box, im_box, round_me(threshold_found), param->max_thr, CV_THRESH_TOZERO);     
            
            IplImage* im_diff = cvCloneImage(get_img_OCR_Image(pt_search_box->im_average_mask_t));
            cvAbsDiff(get_img_OCR_Image(pt_search_box->im_average_mask_t), im_box, im_diff);
            double v = cvAvg(im_diff, NULL).val[0];
            if (v<param->max_val_diff_b2b) pt_search_box->stop_frame=frameNum;
            cvReleaseImage(&im_box);
            cvReleaseImage(&im_diff);
            if (param->path_im_mask!=NULL) cvResetImageROI(im_mask);
        }
        // save intermediate images
        if ((((pt_search_box->stop_frame - pt_search_box->start_frame) % param->fr_reco) == 0) && (pt_search_box->stop_frame != pt_search_box->start_frame) && (pt_search_box->stop_frame == frameNum)){
            IplImage* im_box = cvCloneImage(get_img_OCR_Image(pt_search_box->im_average_mask));
            cvSeqSort( pt_search_box->seq_thr, cmp_thr, 0);
            int* thr_med = (int*)cvGetSeqElem( pt_search_box->seq_thr, (int)(pt_search_box->nb_img_detect_avg/2));                  
            cvClearSeq(pt_search_box->seq_thr);                
            cvSeqPush(pt_search_box->seq_thr,&pt_search_box->current_thr );
           
            OCR_Image *oi_tmp=alloc_init_OCR_Image(im_box,NULL,-1,*thr_med);
            pt_search_box->nb_img_detect_avg=0;
            cvSeqPush(pt_search_box->seq_im,&oi_tmp );
            change_img_OCR_Image(pt_search_box->im_average_mask,NULL);
            set_img_OCR_Image(pt_search_box->im_average_mask,cvCloneImage(pt_search_box->im_current)); 
        
            if (param->print_text == 1){            
                printf("box_%d img_%d ymin=%d ymax=%d xmin=%d xmax=%d " ,pt_search_box->num, pt_search_box->seq_im->total ,round_me(pt_search_box->ymin_avg), round_me(pt_search_box->xmin_avg), round_me(pt_search_box->ymax_avg), round_me(pt_search_box->xmax_avg));
                print_transcription_image(im_box, round_me(pt_search_box->thr_med), param);
            }
           
            pt_search_box->ymin_avg=pt_search_box->ymin_current;
            pt_search_box->xmin_avg=pt_search_box->xmin_current;
            pt_search_box->ymax_avg=pt_search_box->ymax_current;
            pt_search_box->xmax_avg=pt_search_box->xmax_current;
        }
        // if boxes disappear save it into seq_box_final
        if (pt_search_box->stop_frame < frameNum-param->space_btfd ){
            if (pt_search_box->stop_frame-pt_search_box->start_frame>param->min_duration_box) {
                cvSeqPush(seq_box_final, &pt_search_box);
                cvSeqSort( pt_search_box->seq_thr_t, cmp_thr, 0);
                int* thr_med = (int*)cvGetSeqElem( pt_search_box->seq_thr_t, (int)(pt_search_box->nb_img_detect_avg_t/2) );   
                set_threshold_OCR_Image(pt_search_box->im_average_mask_t,*thr_med); 
                transcription_box(pt_search_box, param);   
                
                if (param->print_text == 1){            
                    printf("box_%d img_avg ymin=%d ymax=%d xmin=%d xmax=%d " ,pt_search_box->num ,round_me(pt_search_box->ymin_avg), round_me(pt_search_box->xmin_avg), round_me(pt_search_box->ymax_avg), round_me(pt_search_box->xmax_avg));
                    print_transcription_image(get_img_OCR_Image(pt_search_box->im_average_mask_t), round_me(pt_search_box->thr_med), param);
                }
            }
            else{free_box(pt_search_box);}
            cvSeqRemove(seq_box, i);
        }
    }
}
