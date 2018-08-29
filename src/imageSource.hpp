/*
 * imageSource.h
 *
 *  Created on: Aug 17, 2017
 *      Author: miaou51914
 */

#ifndef SRC_IMAGESOURCE_HPP_
#define SRC_IMAGESOURCE_HPP_

#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

class ImageSource {
public:
	virtual void next(Mat &image) = 0;

	virtual ~ImageSource() {

	}
};

class VcImageSource: public ImageSource {
public:
	VcImageSource(int cameraId) {
		if (!cap.open(cameraId)) {
			throw;
		}
	}

	virtual void next(Mat &image) {
		cap >> image;
		if (image.empty())
			throw;
	}
	VideoCapture cap;
};

#include "v4ldevice.h"
class V4lImageSource: public ImageSource {
public:
	V4lImageSource(int cameraId, CvSize ImageSize) {
		pOpenCVImage = cvCreateImage(ImageSize, IPL_DEPTH_8U, 1); // Grayscale
		pColorCVImage = cvCreateImage(ImageSize , IPL_DEPTH_8U, 3 ); // Color image
		open_device((char*)((string("/dev/video") + to_string(cameraId)).c_str()));
		init_device(ImageSize.width, ImageSize.height);
		start_capturing();
	}

	virtual void next(Mat &image) {
		ImageBuffer = snapFrame();

		if (ImageBuffer != NULL) {
			memcpy(pOpenCVImage->imageData, ImageBuffer,
					pOpenCVImage->imageSize);
			cvCvtColor(pOpenCVImage, pColorCVImage, CV_GRAY2RGB); // Create a color image from the raw data
			image = cv::cvarrToMat(pColorCVImage);
		} else {
			printf("No image buffer retrieved.\n");
			throw;
		}
	}
	IplImage* pOpenCVImage;
	IplImage* pColorCVImage;
	unsigned char* ImageBuffer = NULL;
};

#endif /* SRC_IMAGESOURCE_HPP_ */
