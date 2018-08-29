/*
 * CamCapture.cpp
 *
 *  Created on: Aug 16, 2017
 *      Author: miaou51914
 */

#include "opencv2/opencv.hpp"
#include <docopt.cpp/docopt.h>

#include <iostream>
#include "cmosCalibration.hpp"
#include "omniCalibration.hpp"
#include "imageSource.hpp"
#include <unistd.h>

static const char USAGE[] =
		R"(camCapture.

    Usage:
      camCapture (vc|v4l) <cameraId> [(cmos|omni) <calib.xml>] [--width=<width> --height=<height>]
      camCapture (-h | --help)

    Options:
      -h --help     Show this screen.
)";

using namespace cv;
using namespace std;
int main(int argc, char** argv) {
	std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
			{ argv + 1, argv + argc }, true,           // show help if requested
			"0.1");  // version string

	//Read calibration settings
	ICalibration *s = NULL;
	if(args["cmos"].asBool()){
		s = new CmosParam(args["<calib.xml>"].asString());
	}
	if(args["omni"].asBool()){
		s = new OmniParam(args["<calib.xml>"].asString());
	}

	ImageSource *imageSource;
	if(args["vc"].asBool()){
		imageSource = new VcImageSource(args["<cameraId>"].asLong());
	}

	Size imageSize;
	if(s != NULL)
		imageSize = s->imageSize;
	else
		imageSize = Size(args["--width"].asLong(),args["--height"].asLong());

	if(args["v4l"].asBool()){
		imageSource = new V4lImageSource(args["<cameraId>"].asLong(), CvSize(imageSize));
	}


	namedWindow( "stream", WINDOW_NORMAL);
	resizeWindow("stream", imageSize.width , imageSize.height);
	int bmpCounter = 1;
	for (;;) {
		Mat frame, frameUpscaled,frameRectified;
		imageSource->next(frame);
		if(s != NULL){
			resize(frame, frameUpscaled, s->imageSize); //resize image
			s->rectifyImage(frameUpscaled,frameRectified);
			imshow("stream", frameRectified);
			if (waitKey(50) == 'a'){
				cout << "miaou " << bmpCounter << endl;
				Mat grayScaled,grayRectified;
				cvtColor(frame, grayScaled, cv::COLOR_RGB2GRAY);
				cvtColor(frameRectified, grayRectified, cv::COLOR_RGB2GRAY);
				imwrite(string("im") + std::to_string(bmpCounter) + string(".bmp"), grayScaled);
				imwrite(string("rec") + std::to_string(bmpCounter) + string(".bmp"), grayRectified);
//				rectangle(frame,Rect(0,0,imageSize.width,imageSize.height), Scalar(0,0,0,0),1,FILLED);
//				imshow("stream", frameRectified);
				bmpCounter++;
				sleep(2);
			}
		} else {
			imshow("stream", frame);
			if (waitKey(50) == 'a'){
				cout << "miaou " << bmpCounter << endl;
				putchar('\a');
				Mat grayScaled;
				cvtColor(frame, grayScaled, cv::COLOR_RGB2GRAY);
				imwrite(string("im") + std::to_string(bmpCounter) + string(".bmp"), grayScaled);
				bmpCounter++;
//				rectangle(frame,Rect(0,0,imageSize.width,imageSize.height), Scalar(0,0,0,0),1,FILLED);
//				imshow("stream", frame);
				sleep(2);
			}
		}
		if (waitKey(10) == 27)
			break; // stop capturing by pressing ESC

	}
	return 0;
}
