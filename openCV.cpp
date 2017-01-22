//#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>


/*
	Camera settings to use, enter from command line
	
	v4l2-ctl -d /dev/video0 -c contrast=0
	v4l2-ctl -d /dev/video0 -c saturation=83
	v4l2-ctl -d /dev/video0 -c brightness=3
	v4l2-ctl -d /dev/video0 -c exposure_absolute=5
*/

int main()
{
	cv::namedWindow("orig", cv::WINDOW_NORMAL);	
	cv::namedWindow("contour", cv::WINDOW_NORMAL);	
	cv::namedWindow("mask", cv::WINDOW_NORMAL);	
	cv::VideoCapture cap;

	/*
	cv::resizeWindow("orig", 360,360);	
	cv::resizeWindow("color", 360,360);	
	cv::resizeWindow("mask", 360,360);	
	*/	

	cap.open(0);


	if (!cap.isOpened()) {
		//std::cerr << "Couldn't open" << std::endl;
		return -1;
	}

	cv::Mat frame;
	cv::Mat pFrame;
	cv::Mat pFrame_hsv;
	cv::Mat mask;
	cv::Mat canny;

	cv::Scalar contourColor(0,255,0);
	while(1) {
		cap>>frame;
		cv::cvtColor(frame, pFrame_hsv, cv::COLOR_RGB2HSV);
		
		cv::inRange(pFrame_hsv, cv::Scalar(30, 80, 100), cv::Scalar(85, 255, 255), mask);
		

		cv::bitwise_and(pFrame_hsv, pFrame_hsv, pFrame, mask);
		
		cv::Mat realMask = mask.clone();		
		
		cv::Canny(mask, canny, 100, 150);

		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(canny, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_TC89_KCOS);
		
		for(const std::vector<cv::Point> contour : contours){
			std::vector<cv::Point> quad(4);
			int a = cv::contourArea(contour);
			if(a>10){
				std::cout<<a<<" = area size"<<std::endl;
				cv::approxPolyDP(contour, quad, 10, true);
			}
			if(quad.size() == 4){
				int distLine = std::max(quad[0].y, quad[3].y);
				int midLine = (quad[0].x + quad[3].x)/2;
				
				
				//cv::line(frame, {0, distLine}, {frame.cols, distLine}, contourColor);
				//cv::line(frame, {midLine, 0}, {midLine, frame.rows}, contourColor);
				
				cv::Rect rect = cv::boundingRect(contour);
				cv::rectangle(frame,rect.tl(), rect.br(), contourColor, 2);
			}

		}

		cv::imshow("orig", frame);	
		cv::imshow("mask", realMask);	
		cv::imshow("contour", canny);	
		if (cv::waitKey(33) >=0) {
			break;
		}
	}

	return 0;
}
