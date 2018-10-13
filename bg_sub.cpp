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
        if (hasFile)
        {
            fps = int(cap.get(CAP_PROP_FPS));
        }
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
        namedWindow("Orig", 1);
        namedWindow("FG", 1);
        if (bgImage)
        {
            namedWindow("BG", 1);
        }
        cout << "Press 's' to save a frame to the current directory.\n"
                "Use ESC to quit.\n" << endl;
    }
    int64 startTime = getTickCount();
    for(;;)
    {
        Mat frame, fgMask, fg, bg;
        cap.grab();
        cap.retrieve ( frame);
        if( frame.empty() )
        {
            break;
        }
        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        pBgSub->apply(gray, fgMask);
        if (hasGui)
        {
            imshow("Orig", frame);
            if (showFG)
            {
                frame.copyTo(fg, fgMask);
                imshow("FG", fg);
            }
        }
        if (bgImage)
        {
            pBgSub->getBackgroundImage(bg);
            if (hasGui)
            {
                imshow("BG", bg);
            }
        }
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
                if (bgImage)
                {
                    imwrite("bg.jpg", bg);
                }
            }
        }
    }
    double tfreq = getTickFrequency();
    double secs = ((double) getTickCount() - startTime)/tfreq;
    cout << "Execution took " << fixed << secs << " seconds." << endl;
}