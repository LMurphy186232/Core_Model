#ifndef ConstantsH
#define ConstantsH
//---------------------------------------------------------------------------
/**
* @file Constants.h
* Constant declarations
*
* Copyright 2003 Charles D. Canham.
* @author Lora E. Murphy
*
* <br>Edit history:
* <br>-----------------
* <br>November 12, 2012 - Chars became strings (LEM)
*/

const int MAX_VERSION_SIZE = 4;  /**<Max length of version number string*/
const std::string DETAILED_OUTPUT_FILE_EXT = ".xml"; /**<File extension for detailed output files*/
const std::string ZIPPED_FILE_EXT = ".xml.gz";/**<Zipped file extension*/
const std::string GZIP_EXT = ".gz"; /**<GZIP file extension*/
const std::string TARBALL_FILE_EXT = ".gz.tar"; /**<Zipped, tarred file extension -
                keep this lowercase always!*/
const std::string SHORT_OUTPUT_FILE_EXT = ".out";/**<File extension for short output files*/
const std::string TEXT_FILE_EXT = ".txt";/**<File extension for text files*/
const int DETAILED_OUTPUT_FILE_VERSION = 1; /**<File version for detailed output files*/
const float QUADRAT_CELL_SIZE = 2; /**<Grid cell resolution for quadrat light*/
const float CONVERT_TO_DEGREES = 57.29578; /**<Multiply radian values by this to get degrees*/
const float CONVERT_TO_RADIANS = 0.0174533;/**<Multiply degree values by this to get radians*/
const int M_SQ_PER_HA = 10000; /**<Number of square meters in a hectare*/
const float VERY_SMALL_VALUE = 1e-5; /**<Small value - arbitrarily chosen - this
                                     is the smallest value that I can get to
                                     always have an effect on float precision*/
const float CONVERT_IN_TO_CM = 2.54; /**<Multiply a value in inches by this to transform to cm*/
const float CONVERT_CM_TO_IN = 0.394; /**<Multiply a value in cm by this to transform to inches*/
const float CONVERT_LBS_TO_KG = 0.454; /**<Multiply a value in pounds by this to transform to kg*/
const float CONVERT_KG_TO_LB = 2.20462262; /**<Multiply a value in kg by this to transform to lb*/
const float CONVERT_KG_TO_MG = 0.001; /**<Multiply a value in kg by this to transform to metric tons (Mg)*/
const float CONVERT_G_TO_KG = 0.001; /**<Multiply a value in g by this to transform to kg*/
const float CONVERT_MM_TO_CM = 0.1; /**<Multiply a value in mm by this to transform to cm*/
const float CONVERT_CM_TO_MM = 10.0; /**<Multiply a value in cm by this to transform to mm*/
const float CONVERT_MG_TO_KG = 1000; /**<Multiply a value in Mg by this to transform to kg*/
const int MAX_SUBPLOT_NAME_SIZE = 100; /**<Size of subplot names*/

#endif
