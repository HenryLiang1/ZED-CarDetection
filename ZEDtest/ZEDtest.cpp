// ZEDtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ctime>
#include <chrono>
#include <random>

#include <opencv2/core/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/tracking.hpp>
#include <zed/Camera.hpp>
#include <zed/utils/GlobalDefine.hpp>

using namespace sl::zed;
using namespace std;


typedef struct OCVStruct {
	float* data;
	uint32_t step;
	cv::Size _image;
	cv::Size _resize;
	std::string name;
	std::string unit;
} OCV;

typedef struct CarInfo {
	cv::Rect position;
	double trackDist = 0.0;
	double currentDist = 0.0;
	double previousDist = 0.0;
};

OCV ocvStruct;
std::vector<CarInfo> carsInfo;

string carsCascadeName = "car_cascade_28X24_600_600_25_haar.xml";
cv::CascadeClassifier carsCascade;


static void onMouseCallback(int event, int x, int y, int flag, void * param) {
	if (event == CV_EVENT_LBUTTONDOWN) {
		OCVStruct* data = (OCVStruct*)param;

		int y_int = ((/*data->_resize.height - */y)* data->_image.height / data->_resize.height);
		int x_int = ((/*data->_resize.width - */x)* data->_image.width / data->_resize.width);

		float* ptr_image_num = (float*)((char*)data->data + y_int * data->step);
		float dist = ptr_image_num[x_int];

		if (isValidMeasure(dist))
			std::cout << data->name.c_str() << " " << dist << " " << data->unit.c_str() << std::endl;
		else {
			if (dist == TOO_FAR)
				std::cout << data->name.c_str() << " " << dist << " " << data->unit.c_str() << std::endl;
			else if (dist == TOO_CLOSE)
				std::cout << data->name.c_str() << " " << dist << " " << data->unit.c_str() << std::endl;
			else
				std::cout << data->name.c_str() << " " << dist << " " << data->unit.c_str() << std::endl;
		}
	}
}

void detectAndDisplay(cv::Mat frame, int currentFrame, OCVStruct* data) {
	std::vector<cv::Rect> cars;
	std::vector<cv::Rect> prevCars;
	//std::vector<cv::Rect2d> cars;
	cv::Mat gray;

	//cv::cvtColor(frame, gray, CV_BGR2GRAY);
	//cv::equalizeHist(gray, gray);

	//-- Detect faces
	carsCascade.detectMultiScale(frame, cars, 1.1, 3, 0);

	//show current frame
	std::cout << "Current Frame : " << currentFrame << std::endl;

	for (size_t i = 0; i < cars.size(); i++)
	{
		cv::Point center(cars[i].x + cars[i].width*0.5, cars[i].y + cars[i].height*0.5 + data->_resize.height / 2);
		cv::rectangle(frame, cv::Point(cars[i].x, cars[i].y), cv::Point(cars[i].x + cars[i].width, cars[i].y + cars[i].height), cv::Scalar(0, 255, 0), 1);

		int y_int = ((center.y)* data->_image.height / data->_resize.height);
		int x_int = ((center.x)* data->_image.width / data->_resize.width);

		float* ptr_image_num = (float*)((char*)data->data + y_int * data->step);
		double dist = ptr_image_num[x_int];
		double currentDist;
		double previousDist;

		bool isOriCar = false;
		CarInfo carInfo;
		carInfo.position = cars[i];
		carInfo.currentDist = dist;

		
		if (isValidMeasure(dist)) {
			if (carsInfo.size() <= 0) {
				carsInfo.push_back(carInfo);
			}
			else {
				for (int j = 0; j < carsInfo.size(); j++) {
					//std::cout << j << " : " << carsInfo[j].position.x << " " << carsInfo[j].position.y << std::endl;
					if (abs(carInfo.position.x - carsInfo[j].position.x) <= 200 && abs(carInfo.position.y - carsInfo[j].position.y) <= 200) {
						isOriCar = true;


						carsInfo[j].position = carInfo.position;
						carsInfo[j].trackDist = dist;
						if (currentFrame % 30 == 0) {
							carsInfo[j].previousDist = carsInfo[j].currentDist;
						}
						else if (currentFrame % 30 == 1) {
							carsInfo[j].currentDist = dist;
						}
					}
				}
				if (isOriCar == false) {
					carsInfo.push_back(carInfo);
				}
			}
			
			stringstream ss;
			stringstream ss2;
			stringstream ss3;
			//ss << dist << " " << data->unit.c_str();
			//ss2 << carsInfo[i].previousDist << " " << data->unit.c_str();
			//previousDist = carsInfo[i].previousDist;
			//ss3 << abs(previousDist - currentDist) / 0.03 * 3.6 << " " << "km/hr";
			//cv::putText(frame, ss.str(), cv::Point(cars[i].x, cars[i].y - 15), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255));
			//cv::putText(frame, ss2.str(), cv::Point(carsInfo[i].position.x, carsInfo[i].position.y - 15), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 255, 0));
			//cv::putText(frame, ss3.str(), cv::Point(carsInfo[i].position.x, carsInfo[i].position.y), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 0, 0));
			

		}
		else {
			if (dist == TOO_FAR) {
				cv::putText(frame, "Too far", cv::Point(cars[i].x, cars[i].y), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255));
				//printf("\n%s is too far.\n", data->name.c_str(), dist, data->unit.c_str());
			}
			else if (dist == TOO_CLOSE) {
				cv::putText(frame, "Too close", cv::Point(cars[i].x, cars[i].y), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255));
				//printf("\n%s is too close.\n", data->name.c_str(), dist, data->unit.c_str());
			}
			else {
				cv::putText(frame, "Not avaliable", cv::Point(cars[i].x, cars[i].y), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255));
				//printf("\n%s not avaliable\n", data->name.c_str(), dist, data->unit.c_str());
			}

		}

	}

	

	for (int i = 0; i < carsInfo.size(); i++) {
		//std::cout << i << " : " << carsInfo[i].position << " currentDist : " << carsInfo[i].currentDist << " prevDist : " << carsInfo[i].previousDist << std::endl;
		stringstream ss;
		stringstream ss2;
		stringstream ss3;
		ss << carsInfo[i].trackDist << " " << data->unit.c_str();
		//ss2 << carsInfo[i].previousDist << " " << data->unit.c_str();
		
		//std::cout << abs(carsInfo[i].currentDist - carsInfo[i].previousDist) * 3.6 + 80 << endl;
		cv::putText(frame, ss.str(), cv::Point(carsInfo[i].position.x, carsInfo[i].position.y - 18), cv::FONT_HERSHEY_COMPLEX, 0.7, cv::Scalar(0, 0, 255));
		//cv::putText(frame, ss2.str(), cv::Point(carsInfo[i].position.x, carsInfo[i].position.y - 15), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 255, 0));
		if (carsInfo[i].previousDist != 0.0) {
			ss3 << (carsInfo[i].currentDist - carsInfo[i].previousDist) * 3.6 + 80 << " " << "km/h";
			cv::putText(frame, ss3.str(), cv::Point(carsInfo[i].position.x, carsInfo[i].position.y), cv::FONT_HERSHEY_COMPLEX, 0.7, cv::Scalar(255, 0, 0));
		}
		
	
	}





	//-- Show what you got
	//cv::namedWindow("Cars", 0);
	//cv::imshow("Cars", frame);
}

void playMode(char key, int& viewID) {
	switch (key) {
		// ______________  VIEW __________________
	case '0': // left
		viewID = 0;
		break;
	case '1': // right
		viewID = 1;
		break;
	case '2': // anaglyph
		viewID = 2;
		break;
	case '3': // gray scale diff
		viewID = 3;
		break;
	case '4': // Side by side
		viewID = 4;
		break;
	case '5': // overlay
		viewID = 5;
		break;
	}
}

int main() {
	ERRCODE err;
	Camera *zed;
	std::string SVOName = "D:/ZED_Record/20170228/ZED_footage_4488_720_28-02-2017-08-57-27.svo";
	std::string fileName = "C:/Users/A8859/Documents/ZED/zed_record.svo";

	if (!carsCascade.load(carsCascadeName)) {
		cout << "Cannot load cascade file !!!" << endl;
		system("pause");
		return -1;
	}

	//zed = new Camera(HD720);
	zed = new Camera(SVOName);

	InitParams parameters;
	parameters.mode = PERFORMANCE;
	parameters.unit = METER;
	parameters.verbose = 1;

	err = zed->init(parameters);

	//ERRCODE display
	cout << errcode2str(err) << endl;

	//Quit if an error occurred
	if (err != SUCCESS) {
		delete zed;
		return 1;
	}

	// The depth is limited to 20 METERS, as defined in zed::init()
	zed->setDepthClampValue(10000);

	//Get the width and height of the recorded image
	int width = zed->getImageSize().width;
	int height = zed->getImageSize().height;
	cv::Size size(width, height);


	//Set the display size
	cv::Size displaySize(720, 404);
	cv::Mat disp(width, height, CV_8UC4);
	cv::Mat dispDisplay(displaySize, CV_8UC4);

	//Mats to store view and resized view
	cv::Mat view, viewDisplay;
	//Create an Opencv display window
	cv::namedWindow("Display", cv::WINDOW_AUTOSIZE);
	//cv::namedWindow("Depth", cv::WINDOW_NORMAL);

	// Mouse callback initialization
	sl::zed::Mat depth;
	zed->grab(SENSING_MODE::STANDARD);
	depth = zed->retrieveMeasure(sl::zed::MEASURE::DEPTH); // Get the pointer

	ocvStruct._image = cv::Size(width, height);
	ocvStruct._resize = displaySize;
	ocvStruct.data = (float*)depth.data;
	ocvStruct.step = depth.step;
	ocvStruct.name = "DEPTH";
	ocvStruct.unit = unit2str(parameters.unit);

	// The depth is limited to 20 METERS, as defined in zed::init()
	zed->setDepthClampValue(10000);

	cv::setMouseCallback("Display", onMouseCallback, (void*)&ocvStruct);
	sl::zed::ZED_SELF_CALIBRATION_STATUS old_self_calibration_status = sl::zed::SELF_CALIBRATION_NOT_CALLED;

	//Initialize variables
	char key = ' '; // key pressed
	int viewID = 1; // view type
	int svoPosition = 1425; // SVO frame index
	bool paused = false; // play/pause toggle
	int ff = 1; // fast-forward speed
	int r = 0; // rewind speed
	zed->setSVOPosition(svoPosition);

	//slMat2cvMat(zed->retrieveImage(static_cast<SIDE> (viewID))).copyTo(view);
	//cv::resize(view, viewDisplay, displaySize);
	//cv::Ptr<cv::Tracker> tracker = cv::Tracker::create("KCF");
	//cv::Rect2d roi;;
	//roi = cv::selectROI("tracker", viewDisplay);
	//quit if ROI was not selected
	//if (roi.width == 0 || roi.height == 0)
	//	return 0;

	// initialize the tracker
	//tracker->init(viewDisplay, roi);

	//loop until 'q' is pressed
	for (;;) {
		//Get frames
		zed->grab(SENSING_MODE::STANDARD);
		if (old_self_calibration_status != zed->getSelfCalibrationStatus()) {
			std::cout << "Self Calibration Status : " << sl::zed::statuscode2str(zed->getSelfCalibrationStatus()) << std::endl;
			old_self_calibration_status = zed->getSelfCalibrationStatus();
		}


		//zed->record();
		//Even if Left and Right images are still available through getView() function, it's better since v0.8.1 to use retrieveImage for cpu readback because GPU->CPU is done async during depth estimation.
		// Therefore :
		// -- if disparity estimation is enabled in grab function, retrieveImage will take no time because GPU->CPU copy has already been done during disp estimation
		// -- if disparity estimation is not enabled, GPU->CPU copy is done in retrieveImage fct, and this function will take the time of copy.
		if (viewID < 2) {
			slMat2cvMat(zed->retrieveImage(static_cast<SIDE> (viewID))).copyTo(view);
			cv::resize(view, viewDisplay, displaySize);
			//cout << roi.x << " " << roi.y << endl;
			//update the tracking result
			// stop the program if no more images
			//if (viewDisplay.rows == 0 || viewDisplay.cols == 0)
			//	break;
			//tracker->update(viewDisplay, roi);

			// draw the tracked object
			// draw the tracked object
			//rectangle(viewDisplay, roi, cv::Scalar(255, 0, 0), 2, 1);

			// show image with the tracked object
			//cv::imshow("tracker", viewDisplay);



			//cv::Mat roi = dispDisplay(cv::Rect(0, 0, viewDisplay.cols, viewDisplay.rows));
			cv::Mat srcROI(viewDisplay, cv::Rect(0, viewDisplay.rows / 2, viewDisplay.cols, viewDisplay.rows / 2)); // detect the road except the sky
			cv::resize(srcROI, srcROI, cv::Size(720, 202));
			int currentFrame = zed->getSVOPosition();
			detectAndDisplay(srcROI, currentFrame, &ocvStruct);

			zed->setConfidenceThreshold(100);
			depth = zed->retrieveMeasure(sl::zed::MEASURE::DEPTH); // Get the pointer
			slMat2cvMat(zed->normalizeMeasure(sl::zed::MEASURE::DEPTH)).copyTo(disp);

			// To get the depth at a given position, click on the disparity / depth map image
			cv::resize(disp, dispDisplay, displaySize);
			//cv::imshow("Depth", disp);
			cv::imshow("Display", viewDisplay);

		}
		else {
			slMat2cvMat(zed->getView(static_cast<VIEW_MODE> (viewID - 2))).copyTo(view);
			cv::resize(view, viewDisplay, displaySize);

			//cv::imshow("Display", viewDisplay);
		}

		key = cv::waitKey(5);
		playMode(key, viewID);
	}
	//zed->stopRecording();
	delete zed;
	cout << "\nQuitting ..." << endl;
	return 0;
}