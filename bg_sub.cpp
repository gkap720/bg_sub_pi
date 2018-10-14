#include <opencv2/opencv.hpp>
#include <iostream>
#include <wiringSerial.h>
#include <raspicam/raspicam_cv.h>
using namespace cv;
using namespace std;

bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 ) {
    double i = fabs( contourArea(cv::Mat(contour1)) );
    double j = fabs( contourArea(cv::Mat(contour2)) );
    return ( i < j );
}

int main( int argc, char** argv )
{
    raspicam::RaspiCam_Cv cap;
    cap.set(CV_CAP_PROP_FORMAT, CV_8UC1 );
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);
    cap.setAWB(0);
    cap.set(CAP_PROP_EXPOSURE, 0);
    bool display = true;
    int fd = serialOpen("/dev/ttyUSB0", 115200);
    if (fd < 0)
        cout << "Error opening serial\n";
    
    int thresh = 50;
    float speed = 0.5;
    if( !cap.open() )
    {
        cout << "Could not initialize capturing...\n";
        return 0;
    }

    if(argc > 1) {
        thresh = atoi(argv[1]);
        speed = atof(argv[2]);
        display = (bool) atoi(argv[3]);
    }
    int64 startTime = getTickCount();
    Mat avg, frameDelta;
    int moveAvg [10] = {};
    int index = 0;
    bool init = false;
    for(;;)
    {
        Mat frame, fgMask, fg;
        cap.grab();
        cap.retrieve ( frame);
        if( frame.empty() )
        {
            break;
        }
        GaussianBlur(frame, frame, Size(5, 5), 0);
        if(!init) {
            frame.copyTo(avg);
            avg.convertTo(avg, CV_32F);
            init = true;
        }
        avg.convertTo(avg, CV_32F);
        accumulateWeighted(frame, avg, speed);
        convertScaleAbs(avg, avg);
        absdiff(frame, avg, frameDelta);
        threshold( frameDelta, frameDelta, thresh, 255, THRESH_BINARY );
        vector<vector<Point> > contours0;
        findContours( frameDelta.clone(), contours0, RETR_TREE, CHAIN_APPROX_SIMPLE);
        std::sort(contours0.begin(), contours0.end(), compareContourAreas);
        if(contours0.size() > 0) {
            int cX, cY, n, outNumber;
            cv::Moments M;
            M = moments(contours0[contours0.size()-1]);
            cX = int(M.m10 / M.m00);
            cY = int(M.m01 / M.m00);
            int sum = 0;
            moveAvg[index] = cX;
            index = (index + 1) % 10;
            for(int i = 0; i < 10; i++) {
                sum += moveAvg[i];
            }
            outNumber = sum/10;
            serialPrintf(fd, "%d\n", outNumber);
            cout << outNumber << endl;
            if(display) {
                Scalar color( 255,0,0);
                cvtColor(frameDelta, frameDelta, cv::COLOR_GRAY2BGR);
                drawContours( frameDelta, contours0, 0, color, CV_FILLED, 8);
                circle(frameDelta, Point(cX, cY), 7, color, -1);
                imshow("FG", frameDelta);
            }
        }
        waitKey(30);
    }
    double tfreq = getTickFrequency();
    double secs = ((double) getTickCount() - startTime)/tfreq;
    cout << "Execution took " << fixed << secs << " seconds." << endl;
}

