#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>

#include <cstring>
#include <PracticalSocket.h>


class cameraTracking{

	public:
		cameraTracking(int , string, unsigned short);
		void updateFrame();
		void findRect(cv::Mat);
		double getAngle(int, int, int, int);
		double getAngleRad(){return angleRad;}
		double getAngleDeg(){return angleRad*180/M_PI;}
		void sendPacket();
	private:
		cameraTracking(){};
		
		cv::VideoCapture cap;
		cv::Mat frame;
		cv::Mat hsvFilter;
		cv::Mat mask;
		cv::Mat bitWiseMat;
		cv::Mat edges;
		cv::Scalar contourColor(0,255,0);
		int midLeftx;
		int midLefty;
		int midRightx; 
		int midRighty;
		int midLinex;
		int offset;
		double angleRad;
		unsigned short port;
		string destAddress;

	cameraTracking(int cameraNumber, string destAddress, unsigned short port){
		if(!cap.open(cameraNumber)){
			std::cerr<<"Could not open camera"<<cameraNumber<<std::endl;
		}
		//no null ints
		int midLeftx = 0;
		int midLefty = 0;
		int midRightx = 0; 
		int midRighty = 0;
		int midLinex = 0;
		int offset = 0;
		double angleRad = 0;

		this.destAddress = destAddress;
		this.port = port;
	}

	void updateFrame(){
		//takes the videofeed from the camera and places the frame into the frame Mat
		cap>>frame;
		
		//adds a border to the frame Mat to increase accuracy of finding boxes on border cases
		cv::MakeBorder(frame, frame, 3, 3, 3, 3, cv::BORDER_CONSTANT, cv::Scalar(0,0,0));

		//takes image from frame and converts it to HSV and puts it into the hsvFilter
		cv::cvtColor(frame, hsvFilter, cv::COLOR_RGB2HSV);

		//finds all values that are part of the green lights and puts them into the mask Mat
		cv::inRange(hsvFilter,  cv::Scalar(30,80,100), cv::Scalar(85, 255, 255), mask);

		//ands the image to itself using the mask to get the values and puts them into the bitWiseMat
		cv::bitwise_and(hsvFilter, hsvFilter, bitWiseMat, mask);

		//creates element to give shape and size to erode and dilate operations to reduce noise in the image
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(2*5+1,2*5+1), cv::Point(5,5));
		cv::erode(mask,mask,element);
		cv::dilate(mask,mask,element);

		//find edges and store them in the edges Mat
		cv::Canny(mask, edges, 100,150);

		//finds the rectangles and gets their coordinates
		findRect(&edges);

		//gets angle of the vertex of the two centers of the two rectangles and the bottom of the frame
		angleRad = getAngle(midLeftx - midRightx, midLefty - midRighty, 1, 0);

		//finds offset between midLine and midFrame
		offset = ((double)(midLine-frame.cols/2))/((double)(frame.cols/2))*100;


	}

	void findRect(cv::Mat &edges){
		//holds all the contors found for a given object outer vector is the object and inner vectore is the set of points that make the object
		std::vector<std::vector<cv::Point> > contours;

		//find contours from edges Mat and place them in contours
		cv::findContours(edges, contours, cv::RETR_EXTERNAL, CV::CHAIN_APPROX_TC89_KCOS);

		//used for offset and angle
		midLeftx = 0;
		midLefty = 0;
		midRightx = 0; 
		midRighty = 0;
		angleRad = 0;
		angleDeg = 0;

		//for each region in contours
		for(const std::vector<cv::Point> contour: contours){

			//if the region is large enough then do the computation
			if(cv::contourArea(contour)>10){
				
				//make a 4 sided polygon surrounding the region
				//cv::approxPolyDP(contour, quad, 20, true);

				//make a rectangle surrounding the region with the smallest area 
				cv::RotatedRect rrect = cv::minAreaRect(contour);
				
				//assign the two rectangles center coordinates for further calculation
				if(midLeftx == 0){
					midLeftx = (rrect.center.x);
					midLefty = (rrect.center.y);
					//if there is only one box on the screen make the first one the only considered box
					midRightx = (rrect.center.x);
					midRighty = (rrect.center.y);
				}else if(midRightx == midLeftx){
					midRightx = (rrect.center.x);
					midRighty = (rrect.center.y);
				}
			}
		}//end for loop on contours

		//makes left rectangle on the left if it isn't already
		if(midLeftx>midRightx){
			int temp = midLeftx;
			midLeftx = midRightx;
			midRightx = temp;
		}else if(midLeftx == midRightx && midLeftx == 0){//if no rectagles are found do nothing
			midLeftx = frame.cols/2;
			midRightx = frame.cols/2;
		}

		midLinex = midLeftx + (midRightx-midLeftx)/2;

	}

	//uses the dot product to find the angle between two vectors, returns -1 if a vector is <0,0>

	double getAngle(int x1, int y1, int x2, int y2){
		int dot = abs(x1*x2 + y1*y2);
		int mag1 = sqrt(x1*x1 + y1*y1);
		int mag2 = sqrt(x2*x2 + y2*y2);
		
		if(mag1 != 0 && mag2 != 0){
			return acos(dot/(mag1*mag2));
		}
		return -1;
	}

	void sendPacket(){
		char packet[64];

		memcpy(packet, &offest, sizeof offest);
		memcpy(&packet[4], (int)angleRad*100, sizeof int);

		UDPSocket sock;

		sock.sendTo(packet, destAddress, port);
	}


}