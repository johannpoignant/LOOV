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
#include <stdlib.h>

#include "tools.h" 
#include "params.h"
#include <string.h>

char const *help[]={
    "option: ",
    "     -v                          : video to process, no default",
    "     -output                     : prefix output, default Video_Name",
    "     -path_im_mask               : mask image on video, no default",
    "     -lang                       : file language for tesseract, default value : 'fra'",
    "     -ss                         : start frame to display, default 0",
    "     -endpos                     : number of frame to display to display, default -1 (all video)",
    "  coarse detection: ",
    "     -threshold_sobel            : value of the threshold sobel, default : 103",
    "     -it_connected_caractere     : iteration number for the connection of the edges of characters after the sobel operator, default value : 0.02682",
    "                                   Calculation : iteration = 352(width of the image) * 0.0284(it_connected_caractere) = 9 iterations",
    "     -y_min_size_text            : minimum pixel height of a box, default value : 0.0056",
    "                                   Calculation : iteration = 288(height of the image) * 0.0056(y_min_size_text) = 1",
    "     -x_min_size_text            : minimum pixel width of a box, default value : 0.0466",
    "                                   Calculation : iteration = 352(width of the image) * 0.00466(x_min_size_text) = 16",
    "     -ratio_width_height         : ratio between height and width of a text box, default : 2.275", 
    "  fine_detection: ",
    "     -margin_coarse_detection_Y  : margin arround the box after coarse detection, default : 0.5",
    "                                   Calculation : margin = 10(height of the image box) * 0.5(margin_coarse_detection_Y) = 5 pixels",
    "     -margin_coarse_detection_X  : margin arround the box after coarse detection, default : 1.0",
    "                                   Calculation : margin = 10(height of the image) * 1(margin_coarse_detection_X) = 10 pixels",
    "  temporal tracking: "
    "     -tol_box                    : tolerance % of the f-measure between to box on 2 consecutive frames, default values : 50%",
    "     -min_duration_box           : min duration of a box in frame, default value : 19",
    "     -space_btfd                 : max space between to frames to continue detection of a box, default value : 14 frames",
    "     -max_val_diff_b2b           : max value in average color between two box compared, default value : 20.0",
    "     -fr_reco                    : make text recognition every fr_reco frames, default value : 10",
    "  adapting image for OCR:",
    "     -resize_para                : height of the image box after bicubic interpolation, default value : 200",
    "     -print_text                 : print text transcription after every fr_reco for each box, default no",

    "example of use : ./extract_text -v /Path_To_Video/video.MPG -ss 0 -endpos 100 -lang fra -it_connected_caractere 0.04",
    ""
};

/*print usage */
void usage_moi( FILE *fp, char *progName){
    int i=0;
    fprintf(fp,"Usage of %s:\n", basename_a(progName));
    while (*help[i]){ fprintf(fp,"\t\t%s\n",help[i]); i++;}
}

/*initiaialisation of the parameters*/
LOOV_params init_LOOV_params(){
    LOOV_params param;
    param.videoName=NULL;                                            // video to process, no default
    param.output_path=NULL;                                          // prefix output path, default same as video
    param.tessdata=sprintf_alloc("/usr/local/share/tessdata");       // path for tesseract character model
    param.lang=sprintf_alloc("fra");                                 // tesseract model, default fra
    param.print_text=0;                                              // print text in stdout
    param.path_im_mask=NULL;                                         // path to mask image, to process only a part of the image
    param.aspect_ratio = 1.0;                                        // aspect ratio of the image, default 1.0
    param.startFrame=0;                                              // start frame to process
    param.nbFrame=-1;                                                // number of frame to process, default -1 (all)
    param.max_thr=255;                                               // max threshold, depeding of the depth image, default 255 (8 bits)
    param.total_box=0;                                               // count of detected boxes
    
    //part coarse detection
    param.threshold_sobel = 103;                                     // value of the threshold sobel, default : 103
    param.it_connected_caractere = 0.02682;                          // iteration number for the connection of the edges of characters after the sobel operator, default
    param.y_min_size_text = 0.0056;                                  // minimum pixel height of a box, default value : 0.0056
    param.x_min_size_text = 0.0466;                                  // minimum pixel width of a box, default value : 0.0466
    param.ratio_width_height = 2.275;                                // ratio between height and width of a text box, default : 2.275

    //part find threshold
    param.text_white = -1;                                           // put 1 if texts are written in white, put 0 if there are written in black, put -1 if the color has to be detected, default -1
    param.type_threshold=-2;                                         // 
    param.density_coef_character = 0.72;                             // lenght/weight * coef = nb min of connecting component
    param.max_var = 2.25;
    param.coef_increase_thr_otsu = 1.42;

    //part fine detection
    param.threshold_find_detection=-1;
    param.margin_coarse_detection_Y = 0.5;                           // margin arround the box after coarse detection, default : 0.5
    param.margin_coarse_detection_X = 1;                             // margin arround the box after coarse detection, default : 1.0

    //part temporal monitoring
    param.tol_box = 0.50;                                            // tolerance % of the f-measure between to box on 2 consecutive frames, default values : 50%
    param.min_duration_box = 19;                                     // min duration of a box in frame, default value : 19
    param.space_btfd = 14;                                           // max space between to frames to continue detection of a box, default value : 14 frames
    param.max_val_diff_b2b = 20.0;                                   // max value in average color between two box compared, default value : 20.0
    param.fr_reco = 10;                                              // make text recognition every fr_reco frames, default value : 10

    //Adapting image for tesseract OCR
    param.resize_para = 200;                                         // height of the image send to tesseract
    
    return param;
}   

/*memory allocation for the parameters*/
LOOV_params* alloc_init_LOOV_params(){
    LOOV_params *param=(LOOV_params*) malloc(sizeof(LOOV_params));
    if (! param) return NULL;
    *param=init_LOOV_params();
    return param;
}

/*parse argument*/
LOOV_params* parse_arg(LOOV_params* param, int argc, char *argv[]){
    int argPos=0;
    char *progName=argv[0];
    
    param->lang=sprintf_alloc("fra");
    if(strrchr(progName,'/')) progName= strrchr(progName,'/')+1;
    if (get_Arguments (argc, argv, &argPos, (char*)"-h", (char*) ""))    {usage_moi(stdout,argv[0]); exit(0);}
    if (get_Arguments (argc, argv, NULL, (char*)"-ss",(char*)"%d",                              &param->startFrame))                       printf("starting at the %d frames\n",                param->startFrame);
    if (get_Arguments (argc, argv, NULL, (char*)"-endpos",(char*)"%d",                          &param->nbFrame))                          printf("displaying the next %d frames\n",            param->nbFrame);
    if (get_Arguments (argc, argv, NULL, (char*)"-aspect_ratio",(char*)"%f",                    &param->aspect_ratio))                     printf("aspect ratio %f\n",           			    param->aspect_ratio);
    if ((param->videoName              = get_String_Arguments (argc, argv, (char*)"-v")) ){                                                printf("videoName = %s\n",param->videoName);}
    if ((param->output_path            = get_String_Arguments (argc, argv, (char*)"-output"))){                                            printf("output_path = %s\n",param->output_path);}
    if ((param->path_im_mask           = get_String_Arguments (argc, argv, (char*)"-path_im_mask"))){                                      printf("path_im_mask = %s\n",param->path_im_mask);}
    if ((param->lang                   = get_String_Arguments (argc, argv, (char*)"-lang"))){                                              printf("lang = %s\n",param->lang);}
    param->print_text   = get_Arguments (argc, argv, NULL,     (char*)"-print_text",(char*)"");
    if (get_Arguments (argc, argv, NULL, (char*)"-threshold_sobel",(char*)"%d",                 &param->threshold_sobel))                  printf("threshold_sobel = %d\n",                  param->threshold_sobel);
    if (get_Arguments (argc, argv, NULL, (char*)"-it_connected_caractere",(char*)"%f",          &param->it_connected_caractere))           printf("it_connected_caractere = %f it\n",        param->it_connected_caractere);
    if (get_Arguments (argc, argv, NULL, (char*)"-y_min_size_text",(char*)"%f",                 &param->y_min_size_text))                  printf("y_min_size_text = %f pixels\n",           param->y_min_size_text);
    if (get_Arguments (argc, argv, NULL, (char*)"-x_min_size_text",(char*)"%f",                 &param->x_min_size_text))                  printf("x_min_size_text = %f pixels\n",           param->x_min_size_text);
    if (get_Arguments (argc, argv, NULL, (char*)"-ratio_width_height",(char*)"%f",              &param->ratio_width_height))               printf("ratio_width_height = %f\n",               param->ratio_width_height);
    if (get_Arguments (argc, argv, NULL, (char*)"-text_white",(char*)"%d",                      &param->text_white))                       printf("text_white = %d\n",                       param->text_white);
    if (get_Arguments (argc, argv, NULL, (char*)"-type_threshold",(char*)"%d",                  &param->type_threshold))                   printf("type_threshold = %d\n",                   param->type_threshold);
    if (get_Arguments (argc, argv, NULL, (char*)"-density_coef_character",(char*)"%f",          &param->density_coef_character))           printf("density_coef_character = %f\n",           param->density_coef_character);
    if (get_Arguments (argc, argv, NULL, (char*)"-max_var",(char*)"%f",                         &param->max_var))                          printf("max_var = %f\n",                          param->max_var);
    if (get_Arguments (argc, argv, NULL, (char*)"-coef_increase_thr_otsu",(char*)"%f",          &param->coef_increase_thr_otsu))           printf("coef_increase_thr_otsu = %f\n",           param->coef_increase_thr_otsu);
    if (get_Arguments (argc, argv, NULL, (char*)"-margin_coarse_detection_Y",(char*)"%f",       &param->margin_coarse_detection_Y))        printf("margin_coarse_detection_Y = %f\n",        param->margin_coarse_detection_X);
    if (get_Arguments (argc, argv, NULL, (char*)"-margin_coarse_detection_X",(char*)"%f",       &param->margin_coarse_detection_X))        printf("margin_coarse_detection_X = %f\n",        param->margin_coarse_detection_Y);
    if (get_Arguments (argc, argv, NULL, (char*)"-tol_box",(char*)"%f",                         &param->tol_box))                          printf("tol_box = %d pixels\n",                   round_me(param->tol_box));
    if (get_Arguments (argc, argv, NULL, (char*)"-fr_reco",(char*)"%d",                         &param->fr_reco))                          printf("fr_reco = %d\n",                          param->fr_reco);
    if (get_Arguments (argc, argv, NULL, (char*)"-min_duration_box",(char*)"%d",                &param->min_duration_box))                 printf("min_duration_box = %d\n",                 param->min_duration_box);
    if (get_Arguments (argc, argv, NULL, (char*)"-space_btfd",(char*)"%d",                      &param->space_btfd))                       printf("space_btfd = %d\n",                       param->space_btfd);
    if (get_Arguments (argc, argv, NULL, (char*)"-max_val_diff_b2b",(char*)"%f",                &param->max_val_diff_b2b))                 printf("max_val_diff_b2b = %f\n",                 param->max_val_diff_b2b);
    if (get_Arguments (argc, argv, NULL, (char*)"-resize_para",(char*)"%d",                     &param->resize_para))                      printf("resize_para = %d\n",                      param->resize_para);
    if (param->output_path==NULL) param->output_path=sprintf_alloc("%s",param->videoName);     
    if (param->lang==NULL) param->lang=sprintf_alloc("fra");        

    char * path_OCR_file=sprintf_alloc("%s/%s.traineddata", param->tessdata, param->lang);
    if (fileexists(path_OCR_file)) printf("%s/%s.traineddata found\n", param->tessdata, param->lang);
    else{ fprintf(stderr,"error : copy %s.traineddata into the path %s/\n", param->lang, param->tessdata); exit(0); }
    free(path_OCR_file);

    return param;
}
