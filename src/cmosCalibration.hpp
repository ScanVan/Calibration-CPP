/*
 * cmosCalibration.hpp
 *
 *  Created on: Aug 17, 2017
 *      Author: miaou51914
 */

#ifndef SRC_CMOSCALIBRATION_HPP_
#define SRC_CMOSCALIBRATION_HPP_

#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdio>
#include "ICalibration.hpp"

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/opencv.hpp"


using namespace cv;
using namespace std;

class CmosParam : public ICalibration
{
public:
	CmosParam() : goodInput(false) {}

	CmosParam(const string inputSettingsFile) : CmosParam() {
		FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
		if (!fs.isOpened()) {
			cout << "Could not open the configuration file: \"" << inputSettingsFile
					<< "\"" << endl;
			throw;
		}
		fs.root() >> *this;
		fs.release();

		if (!goodInput) {
			cout << "Invalid input detected. Application stopping. " << endl;
			throw;
		}
	}
//    enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
//    enum InputType { INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST };

    void write(FileStorage& fs) const                        //Write serialization for this class
    {
    	//throw std::unexpected();
        /*fs << "{"
                  << "BoardSize_Width"  << boardSize.width
                  << "BoardSize_Height" << boardSize.height
                  << "Square_Size"         << squareSize
                  << "Calibrate_Pattern" << patternToUse
                  << "Calibrate_NrOfFrameToUse" << nrFrames
                  << "Calibrate_FixAspectRatio" << aspectRatio
                  << "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
                  << "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint

                  << "Write_DetectedFeaturePoints" << writePoints
                  << "Write_extrinsicParameters"   << writeExtrinsics
                  << "Write_outputFileName"  << outputFileName

                  << "Show_UndistortedImage" << showUndistorsed

                  << "Input_FlipAroundHorizontalAxis" << flipVertical
                  << "Input_Delay" << delay
                  << "Input" << input
           << "}";*/
    }
    void read(const FileNode& node)                          //Read serialization for this class
    {
    	node["fisheye_model"] >> useFisheye;
        node["camera_matrix" ] >> cameraMatrix;
        node["distortion_coefficients"] >> distCoeffs;
        node["image_width"] >> imageSize.width;
        node["image_height"] >> imageSize.height;


        validate();
    }
    void validate(){
        goodInput = true;

    	if (useFisheye) {
    		Mat newCamMat;
    		fisheye::estimateNewCameraMatrixForUndistortRectify(cameraMatrix,
    				distCoeffs, imageSize, Matx33d::eye(), newCamMat, 1);
    		fisheye::initUndistortRectifyMap(cameraMatrix, distCoeffs,
    				Matx33d::eye(), newCamMat, imageSize,
    				CV_16SC2, map1, map2);
    	} else {
    		initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
    				getOptimalNewCameraMatrix(cameraMatrix, distCoeffs,
    						imageSize, 1, imageSize, 0), imageSize,
    				CV_16SC2, map1, map2);

    	}
    }

    void rectifyImage(Mat &src, Mat &dst){
    	remap(src, dst, map1, map2, INTER_LINEAR);
    }


public:
    bool goodInput;
    int useFisheye;
    Mat cameraMatrix;
    Mat distCoeffs;

	//Transformation setup
	Mat view, rview, map1, map2;
};
//
static inline void read(const FileNode& node, CmosParam& x, const CmosParam& default_value = CmosParam())
{
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

static inline void write(FileStorage& fs, const String&, const CmosParam& s )
{
    s.write(fs);
}


#endif /* SRC_CMOSCALIBRATION_HPP_ */
