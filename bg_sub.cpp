#include <opencv2/opencv.hpp>
#include <iostream>
#include <raspicam/raspicam_cv.h>
#ifdef HAVE_OPENCV_CONTRIB
#include <opencv2/bgsegm.hpp>
using namespace cv::bgsegm;
#endif
#include "bgsubcnt.h"
using namespace cv;
using namespace std;
const string keys =
        "{help h usage ? || print this message}"
        "{type           |CNT| bg subtraction type from - CNT/MOG2/KNN"
#ifdef HAVE_OPENCV_CONTRIB
        "/GMG/MOG"
#endif
        "}"
        "{bg             || calculate also the background}"
        "{nogui          || run without GUI to measure times}";
int main( int argc, char** argv )
{
    raspicam::RaspiCam_Cv cap;
    cap.set( CV_CAP_PROP_FORMAT, CV_8UC1 );
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);
    CommandLineParser parser(argc, argv, keys);
    parser.about("cv::bgsubcnt::BackgroundSubtractorCNT demo/benchmark/comparison");
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    bool hasGui = ! parser.has("nogui");
    bool bgImage = parser.has("bg");
    string type = parser.get<string>("type");
    string filePath;
    if (! parser.check())
    {
        parser.printErrors();
        return 1;
    }
    if( !cap.open() )
    {
        cout << "Could not initialize capturing...\n";
        return 0;
    }
    Ptr<BackgroundSubtractor> pBgSub;
    if (type == "CNT")
    {
        int fps = 15;
        pBgSub = cv::bgsubcnt::createBackgroundSubtractorCNT(fps, true, fps*60);
    }
    else if (type == "MOG2")
    {
        Ptr<BackgroundSubtractorMOG2> pBgSubMOG2 = createBackgroundSubtractorMOG2();
        pBgSubMOG2->setDetectShadows(false);
        pBgSub = pBgSubMOG2;
    }
    else if (type == "KNN")
    {
        pBgSub = createBackgroundSubtractorKNN();
    }
#ifdef HAVE_OPENCV_CONTRIB
    else if (type == "GMG")
    {
        pBgSub = createBackgroundSubtractorGMG();
    }
    else if (type == "MOG")
    {
        pBgSub = createBackgroundSubtractorMOG();
    }
#endif
    else
    {
        parser.printMessage();
        cout << "\nWrong type - please see above\n";
        return 1;
    }
    bool showFG=true;
    if (hasGui)
    {
        namedWindow("FG", 1);
        if (bgImage)
        {
            namedWindow("BG", 1);
        }
        cout << "Press 's' to save a frame to the current directory.\n"
                "Use ESC to quit.\n" << endl;
    }
    int64 startTime = getTickCount();
    Mat kernel, avg, frameDelta;
    bool init = false;
    kernel = cv::getStructuringElement(MORPH_ELLIPSE, Size(5,5));
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
        if(!init) {
            frame.copyTo(avg);
            init = true;
        }
        accumulateWeighted(frame, avg, 0.5);
        convertScaleAbs(avg, avg);
        absdiff(frame, avg, frameDelta);
        //pBgSub->apply(frame, fgMask);
        //processing steps!
        //cv::morphologyEx(frame, frame, MORPH_CLOSE, kernel);
        //cv::morphologyEx(frame, frame, MORPH_OPEN, kernel);
        //cv::dilate(frame, frame, kernel, Point(-1,-1), 2);
        vector<vector<Point> > contours0;
        findContours( frameDelta, contours0, RETR_TREE, CHAIN_APPROX_SIMPLE);
        for( int i = 0; i< contours0.size(); i++ ) // iterate through each contour. 
        {
           double a=contourArea( contours0[i],false);  //  Find the area of contour
           if(a>largest_area){
               largest_area=a;
               largest_contour_index=i;                //Store the index of largest contour
               bounding_rect=boundingRect(contours0[i]); // Find the bounding rectangle for biggest contour
           }
      
        }
        Scalar color( 255,255,255);
        drawContours( frame, contours0, largest_contour_index, color, CV_FILLED, 8 );
        imshow("FG", frame);
        if (hasGui)
        {
            char c = (char)waitKey(1);
            if (c == 27)
            {
                break;
            }
            else if (c == 's')
            {
                imwrite("frame.jpg", frame);
                imwrite("fg.jpg", fg);
            }
        }
    }
    double tfreq = getTickFrequency();
    double secs = ((double) getTickCount() - startTime)/tfreq;
    cout << "Execution took " << fixed << secs << " seconds." << endl;
}