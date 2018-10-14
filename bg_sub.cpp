#include <opencv2/opencv.hpp>
#include <iostream>
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
    int thresh = 50;
    if( !cap.open() )
    {
        cout << "Could not initialize capturing...\n";
        return 0;
    }
    int64 startTime = getTickCount();
    Mat avg, frameDelta;
    bool init = false;
    for(;;)
    {
        Mat frame, fgMask, fg;
        int largest_area=0;
        int largest_contour_index=0;
        Rect bounding_rect;
        cap.grab();
        cap.retrieve ( frame);
        if( frame.empty() )
        {
            break;
        }
        GaussianBlur(frame, frame, Size(21, 21), 0);
        if(!init) {
            frame.copyTo(avg);
            avg.convertTo(avg, CV_32F);
            init = true;
        }
        if(argc > 1) {
            thresh = atoi(argv[1]);
        }
        avg.convertTo(avg, CV_32F);
        accumulateWeighted(frame, avg, 0.5);
        convertScaleAbs(avg, avg);
        absdiff(frame, avg, frameDelta);
        threshold( frameDelta, frameDelta, thresh, 255, THRESH_BINARY );
        vector<vector<Point> > contours0;
        findContours( frameDelta, contours0, RETR_TREE, CHAIN_APPROX_SIMPLE);
        std::sort(contours0.begin(), contours0.end(), compareContourAreas);
        Scalar color( 1,0,0);
        cvtColor(frameDelta, frameDelta, cv::COLOR_GRAY2BGR);
        if(contours0.size() > 0) {
            int cX, cY;
            cv::Moments M;
            M = moments(contours0[0]);
            cX = int(M.m10 / M.m00);
            cY = int(M.m01 / M.m00);
            circle(frameDelta, Point(cX, cY), 7, color, -1);
        }
        imshow("FG", frameDelta);
        waitKey(30);
    }
    double tfreq = getTickFrequency();
    double secs = ((double) getTickCount() - startTime)/tfreq;
    cout << "Execution took " << fixed << secs << " seconds." << endl;
}

