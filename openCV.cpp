//#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>

#include <cstring>
#include <PracticalSocket.h>


	

int main()
{
	//Camera settings to use, enter from command line
	system("v4l2-ctl -d /dev/video0 -c exposure_absolute=5");
	system("v4l2-ctl -d /dev/video0 -c brightness=3");
	system("v4l2-ctl -d /dev/video0 -c saturation=83");
	system("v4l2-ctl -d /dev/video0 -c contrast=0");
	

	char packet[64];
    	UDPSocket sock;
  
    	// Repeatedly send the string (not including \0) to the server
 
	cv::namedWindow("orig", cv::WINDOW_NORMAL);	
	//cv::namedWindow("contour", cv::WINDOW_NORMAL);	
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
		
		cv::copyMakeBorder(frame, frame, 3, 3, 3, 3, cv::BORDER_CONSTANT, cv::Scalar(0,0,0)); 
				

		cv::cvtColor(frame, pFrame_hsv, cv::COLOR_RGB2HSV);
		
		cv::inRange(pFrame_hsv, cv::Scalar(30, 80, 100), cv::Scalar(85, 255, 255), mask);
		

		cv::bitwise_and(pFrame_hsv, pFrame_hsv, pFrame, mask);
		
		cv::Mat element = cv::getStructuringElement(0/*Morph_rect*/,cv::Size(2*5+1,2*5+1), cv::Point(5,5));
		
		cv::erode(mask,mask,element);
		cv::dilate(mask,mask,element);		

		cv::Mat realMask = mask.clone();		
		
		cv::Canny(mask, canny, 100, 150);

		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(canny, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_TC89_KCOS);
		
		//cv::drawContours(frame, contours, -1, contourColor);

		int midLeftx = 0;
		int midLefty = 0;
		int midRightx = 0; 
		int midRighty = 0;
		double angleRad = 0;
		double angleDeg = 0;
		for(const std::vector<cv::Point> contour : contours){
			std::vector<cv::Point> quad(4);
			int a = cv::contourArea(contour);
			cv::RotatedRect rrect;
			if(a>10){
				//std::cout<<a<<" = area size"<<std::endl;
				cv::approxPolyDP(contour, quad, 20, true);
				rrect = cv::minAreaRect(contour);
				
				/*double dot = abs(quad[0].x*quad[1].x + quad[0].y*quad[1].y);
				double mag1 = sqrt(quad[0].x*quad[0].x + quad[0].y*quad[0].y);
				double mag2 = sqrt(quad[1].x*quad[1].x + quad[1].y*quad[1].y);
				//std::cout<<"Dot = "<<dot<<" Mag1 = "<<mag1<<" Mag2 = "<<mag2<<std::endl;	
				angleRad = acos(dot/(mag1*mag2));
				angleDeg = angleRad*180/M_PI;*/
			}

			if(quad.size() == 4){
				int distLine = std::max(quad[0].y, quad[3].y);
				int midLine = (quad[0].x + quad[3].x)/2;
				
				//cv::line(frame, {0, distLine}, {frame.cols, distLine}, contourColor);
				//cv::line(frame, {midLine, 0}, {midLine, frame.rows}, contourColor);
				
				//cv::Rect rect = cv::boundingRect(contour);
				
				//cv::Rect rect = rrect.boundingRect();

				//cv::Point2f verticies[4];
				//rrect.points(verticies);
				for(int i = 0; i <4; i++){
					//line(frame, verticies[i], verticies[(i+1)%4], cv::Scalar(255,0,0), 4);
					line(frame, quad[i], quad[(i+1)%4], cvScalar(0,0,255),4);
				}


				//cv::rectangle(frame,rect.tl(), rect.br(), contourColor, 2);

				if(midLeftx == 0){
					midLeftx = (rrect.center.x);
					midLefty = (rrect.center.y);
					midRightx = (rrect.center.x);
					midRighty = (rrect.center.y);
				}else if(midRightx == midLeftx){
					midRightx = (rrect.center.x);
					midRighty = (rrect.center.y);
				}
			}

		}
		if(midLeftx>midRightx){
			int temp = midLeftx;
			midLeftx = midRightx;
			midRightx = temp;
		}else if(midLeftx == midRightx && midLeftx == 0){
			midLeftx = frame.cols/2;
			midRightx = frame.cols/2;
		}
		int midLine = midLeftx + abs(midLeftx-midRightx)/2;
		cv::line(frame, {midLine, 0}, {midLine,frame.rows}, contourColor, 2); 
		
		cv::line(frame, {frame.cols/2,0}, {frame.cols/2, frame.rows}, contourColor, 2);
		
		std::vector<int> a(2,0);
		a[0] = midLeftx - midRightx;
		a[1] = midLefty - midRighty;
		std::vector<int> b(1,0);
		b[0] = 1;
		b[1] = 0;
		double dot = abs(a[0]*b[0] + a[1]*b[1]);
		double mag1 = sqrt(a[0]*a[0] + a[1]*a[1]);
		double mag2 = sqrt(b[0]*b[0] + b[1]*b[1]);
		//std::cout<<"Dot = "<<dot<<" Mag1 = "<<mag1<<" Mag2 = "<<mag2<<std::endl;	
		if(mag1*mag2!= 0){
			angleRad = acos(dot/(mag1*mag2));
			angleDeg = angleRad*180/M_PI;
		}	
		
	 	int offset = ((double)(midLine-frame.cols/2))/((double)(frame.cols/2))*100;
		std::cout<<"Offset  = "<<offset<<" Angle = "<<angleDeg<<std::endl;

		int temp = offset;
		int temp2 = (int)(angleDeg * 100);
		//std::cout << sizeof(int)<<endl;

		memcpy(packet, &temp, sizeof temp);
		memcpy(&packet[4], &temp2, sizeof temp2);

		
  		string destAddress = "10.55.14.63";             // First arg:  destination address
  		unsigned short destPort = 9876;  // Second arg: destination port
      		sock.sendTo(packet, 8, destAddress, destPort);
		cv::imshow("orig", frame);	
		cv::imshow("mask", realMask);	
		//cv::imshow("contour", canny);	
		if (cv::waitKey(33) >=0) {
			break;
		}	
	}
}
