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

#ifndef _PARAMS_H
#define _PARAMS_H


typedef struct{
    char* videoName;			// video to process
    char* output_path;			// prefix output path
    char* tessdata;			// path for tesseract character model
    char* lang;				// tesseract model
    int print_text;			// print text in stdout
    char* path_im_mask;			// path to mask image, to process only a part of the image		
    double aspect_ratio;			// aspect ratio of the image
    int startFrame;			// start frame to process
    int nbFrame;			// number of frame to process
    int max_thr;			// max threshold, depeding of the depth image
    int total_box;			// count of detected boxes
    
    //part coarse detection
    int threshold_sobel;		// value of the threshold sobel
    float it_connected_caractere;	// iteration number for the connection of the edges of characters after the sobel operator
    float y_min_size_text;		// minimum pixel height of a box
    float x_min_size_text;		// minimum pixel width of a box
    float ratio_width_height;		// ratio between height and width of a text box

    //part find threshold
    int text_white;
    int type_threshold;
    float density_coef_character;                        // lenght/weight * coef = nb min of connecting component
    float max_var;
    float coef_increase_thr_otsu;

    //part fine detection
    int threshold_find_detection;
    float margin_coarse_detection_Y;	// margin arround the box after coarse detection
    float margin_coarse_detection_X;	// margin arround the box after coarse detection

    //part temporal monitoring
    float tol_box;    			// tolerance % of the f-measure between to box on 2 consecutive frames                 
    int min_duration_box;		// min duration of a box in frame
    int space_btfd;			// max space between to frames to continue detection of a box
    float max_val_diff_b2b;		// max value in average color between two box compared
    int fr_reco;			// make text recognition every fr_reco frames

    //part on adapting image for OCR
    int resize_para;			// height of the image send to tesseract
}LOOV_params;

/*print usage */
void usage_moi( FILE *fp, char *progName);

/*initiaialisation of the parameters*/
LOOV_params init_LOOV_params();

/*memory allocation for the parameters*/
LOOV_params* alloc_init_LOOV_params();

/*parse argument*/
LOOV_params* parse_arg(LOOV_params* param, int argc, char *argv[]);




#endif
