CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

cmake_policy(SET CMP0011 NEW)

FIND_PACKAGE(OpenCV REQUIRED)

INCLUDE_DIRECTORIES("./tesseract-3.00/api"
                    "./tesseract-3.00/ccstruct"
					"./tesseract-3.00/wordrec"
				    "./tesseract-3.00/dict"
					"./tesseract-3.00/ccmain"
					"./tesseract-3.00/cutil"
					"./tesseract-3.00/ccutil"
					"./tesseract-3.00/textord"
					"./tesseract-3.00/classify"
					"./tesseract-3.00/image"
					"./tesseract-3.00/viewer")

SET(TESSERACT_LIB_STATIC  libtesseract_api.a  
                          libtesseract_training.a  
                          libtesseract_main.a 
                          libtesseract_wordrec.a 
                          libtesseract_classify.a 
                          libtesseract_dict.a 
                          libtesseract_textord.a 
                          libtesseract_ccstruct.a 
                          libtesseract_viewer.a 
                          libtesseract_image.a 
                          libtesseract_ccutil.a 
                          libtesseract_cutil.a)

ADD_EXECUTABLE(LOOV LOOV.cpp 
                    params.cpp 
                    params.h 
                    tools.cpp 
                    tools.h 
                    union_find.cpp 
                    union_find.h 
                    OCR_image.cpp 
                    OCR_image.h 
                    box_image.cpp 
                    box_image.h 
                    text_box_detection.cpp 
                    text_box_detection.h)                     

TARGET_LINK_LIBRARIES(LOOV ${OpenCV_LIBS} 
                           ${TESSERACT_LIB_STATIC} 
                           lept 
                           tiff 
                           pthread)  







