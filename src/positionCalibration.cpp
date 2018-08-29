
//Find elipse and place marker
#include "opencv2/opencv.hpp"
#include <docopt.cpp/docopt.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "cmosCalibration.hpp"
#include "imageSource.hpp"

using namespace cv;
using namespace std;

Mat src;
Mat src_gray, laplacian;
int thresh = 44;
int max_thresh = 255;
RNG rng(12345);

/// Function header
void processFrame(CmosParam s);

static const char USAGE[] =
		R"(positionCalibration.

    Usage:
      positionCalibration vc <cameraId> <calib.xml> 
      positionCalibration v4l <cameraId> <calib.xml> 
      positionCalibration (-h | --help)

    Options:
      -h --help     Show this screen.
)";

int main(int argc, char** argv) {
	std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
			{ argv + 1, argv + argc }, true,           // show help if requested
			"0.1");  // version string

	//Read calibration settings
	CmosParam s(args["<calib.xml>"].asString());

	ImageSource *imageSource;
	if (args["vc"].asBool()) {
		imageSource = new VcImageSource(args["<cameraId>"].asLong());
	}

	if (args["v4l"].asBool()) {
		imageSource = new V4lImageSource(args["<cameraId>"].asLong(),
				CvSize(s.imageSize));
	}

	namedWindow("stream", WINDOW_NORMAL);
	resizeWindow("stream", s.imageSize.width / 2, s.imageSize.height / 2);

//	src = imread( "img/mirrorB_rgb.bmp", 1 );

	//Stream cam
	for (;;) {
		Mat frame, frameUpscaled/*,frameRectified*/;
		imageSource->next(frame);
		resize(frame, frameUpscaled, s.imageSize); //resize image
		s.rectifyImage(frameUpscaled, src);
		cvtColor(src, src_gray, cv::COLOR_RGB2GRAY);
		//imshow("this is you, smile! :)", frameRectified);
		processFrame(s);
		if (waitKey(10) == 27)
			break; // stop capturing by pressing ESC
	}

	return 0;
}

void placeMarker(Point2d &position, Point2d &delta) {
	while (src_gray.at<uint8_t>(position.y, position.x) < 100) {
		position += delta;
	}
}

class SphereF_CG: public cv::MinProblemSolver::Function {
public:
	std::vector<cv::Point2i> pointPositions;

	SphereF_CG(std::vector<cv::Point2i> pointPositions) :
			pointPositions(pointPositions) {

	}

	int getDims() const {
		return 3;
	}
	double calc(const double* solution) const {
		double x = solution[0], y = solution[1], r = solution[2];
		double error = 0;
		for (auto p : pointPositions) {
			double distanceSquare = ((p.x - x) * (p.x - x)
					+ (p.y - y) * (p.y - y));
			error += abs(distanceSquare - r * r);
		}
		return error;
	}
};

void processFrame(CmosParam s) {
	Mat threshold_output;
	vector<Vec4i> hierarchy;

	/// Detect edges using Threshold
	threshold(src_gray, threshold_output, thresh, 255, THRESH_BINARY);

	/// Draw contours + rotated rects + ellipses
	Mat drawing = Mat::zeros(threshold_output.size(), CV_8UC3);
	src.copyTo(drawing);

	Point center(s.cameraMatrix.at<double>(0,2), s.cameraMatrix.at<double>(1,2));

	int degreeStart = -65, degreeEnd = 180 + 65;
	std::vector<cv::Point2i> pointPositions;

	//Get mirror borders points
	for (int degree = degreeStart; degree < degreeEnd; degree += 2) {
		double radian = degree / 180.0 * M_PI;
		Point2d position(center.x + cos(radian) * (center.y - 5),
				center.y - sin(radian) * (center.y - 5));
		Point2d delta(-cos(radian), sin(radian));
		bool ok = false;
		while (position.x > 0 && position.y > 0 && position.x < drawing.cols
				&& position.y < drawing.rows) {
			if (src_gray.at<uint8_t>(position.y, position.x) >= thresh) {
				ok = true;
				break;
			}

			position += delta;

		}
		//	position.x += -(position.x-623)*0.001; //DEBUG
		if (ok)
			pointPositions.push_back(cv::Point2i(position.x, position.y));

		drawMarker(drawing, position,
				ok ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255),
				MARKER_CROSS, 10, 1);
	}

	//Compute errors
	{
		//cv::Ptr<cv::ConjGradSolver> solver=cv::ConjGradSolver::create();
		double res;
		cv::Ptr<cv::DownhillSolver> solver = cv::DownhillSolver::create();
		cv::Ptr<cv::MinProblemSolver::Function> ptr_F(
				new SphereF_CG(pointPositions));
		solver->setFunction(ptr_F);

		cv::Mat xx = (cv::Mat_<double>(3, 1) << 600, 500, 300);
		Mat step = (Mat_<double>(3, 1) << 600, 500, 300);
		solver->setInitStep(step);
		res = solver->minimize(xx);
		xx.at<double>(0) += 0;
		xx.at<double>(2) += 0;
		cout << (res) / pointPositions.size() << " error per point" << xx.t()
				<< endl;
		{
			double x = xx.at<double>(0), y = xx.at<double>(1), r =
					xx.at<double>(2);
			double error = 0;
			for (auto p : pointPositions) {
				double distanceSquare = ((p.x - x) * (p.x - x)
						+ (p.y - y) * (p.y - y));
				error += abs(sqrt(distanceSquare) - r);
			}
			error /= pointPositions.size();
			cout << "Real distance error per point = " << error << endl;
		}
		{
			double x = xx.at<double>(0), y = xx.at<double>(1), r =
					xx.at<double>(2);
			double errorX = 0, errorY = 0;
			double errorScaleX = 0, errorScaleY = 0;
			for (auto p : pointPositions) {
				double distance = sqrt(
						(p.x - x) * (p.x - x) + (p.y - y) * (p.y - y));
				double deltaX = (p.x - x), deltaY = (p.y - y);
				errorX += deltaX - deltaX * r / distance;
				errorY += deltaY - deltaY * r / distance;
				errorScaleX += abs(deltaX) - abs(deltaX) * r / distance;
				errorScaleY += abs(deltaY) - abs(deltaY) * r / distance;
			}
			errorX /= pointPositions.size();
			errorY /= pointPositions.size();
			errorScaleX /= pointPositions.size();
			errorScaleY /= pointPositions.size();
			cout << "Error fit       XY    = " << errorX << "  " << errorY << endl;
			cout << "Error fit scale XY    = " << errorScaleX << "  " << errorScaleY << endl;
			cout << "Error optic center XY = " << x-center.x << "  " << y-center.y << endl;
		}
		circle(drawing, Point(xx.at<double>(0), xx.at<double>(1)),
				xx.at<double>(2), Scalar(0, 255, 0), 1);
	}

	imshow("stream", drawing);
}

