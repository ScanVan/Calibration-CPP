#ifndef SRC_ICALIBRATION_HPP_
#define SRC_ICALIBRATION_HPP_

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/opencv.hpp"

using namespace cv;


class ICalibration{
public:
	virtual ~ICalibration(){}
    virtual void rectifyImage(Mat &src, Mat &dst) = 0;
    Size imageSize;
};


#endif
