/*
 * cmosCalibration.hpp
 *
 *  Created on: Aug 17, 2017
 *      Author: miaou51914
 */

#ifndef SRC_OMNICALIBRATION_HPP_
#define SRC_OMNICALIBRATION_HPP_

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
#include "opencv2/ccalib/omnidir.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <time.h>

using namespace cv;
using namespace std;

class OmniParam : public ICalibration
{
public:
	OmniParam() : goodInput(false) {}


	OmniParam(const string inputSettingsFile) : OmniParam() {
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

    void write(FileStorage& fs) const                        //Write serialization for this class
    {

    }
    void read(const FileNode& node)                          //Read serialization for this class
    {
    	node["camera_matrix"] >> K;
        node["distortion_coefficients" ] >> D;
        node["xi"] >> xi;
        node["imageSize"] >> imageSize;
        validate();
    }
    void validate(){
        goodInput = true;
    }

    void rectifyImage(Mat &src, Mat &dst){
	    Mat R = Mat::eye(3, 3, CV_64F);
	    R.at<double>(0,0) = 0.5;
	    R.at<double>(1,1) = 0.5;
    	cv::omnidir::undistortImage(src, dst, K, D, xi, cv::omnidir::RECTIFY_PERSPECTIVE,cv::noArray(),imageSize,R);
    }


public:
    bool goodInput;
    Mat K, D;
    double xi;

};
//
static inline void read(const FileNode& node, OmniParam& x, const OmniParam& default_value = OmniParam())
{
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

static inline void write(FileStorage& fs, const String&, const OmniParam& s )
{
    s.write(fs);
}


#endif /* SRC_OMNICALIBRATION_HPP_ */
