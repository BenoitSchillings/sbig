#include <stdio.h>
#include <SBIGUDrv/SBIGUDrv.h>
#include "highgui.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "cam.h"
#include <signal.h>

using namespace cv;
using namespace std;

Camera *cam;

#define DEBUG(S) {printf(S);printf("\n");}


void setup_ui()
{
    /*
     cvNamedWindow("video", 1);
     createTrackbar("gain", "video", 0, 63, 0);
     createTrackbar("exp", "video", 0, 256, 0);
     createTrackbar("mult", "video", 0, 1500, 0);
     setTrackbarPos("gain", "video", 40);
     setTrackbarPos("exp", "video", 50);
     */
    
}

void intHandler(int dummy=0) {
    printf("emergency close\n");
    cam->Close();
    exit(0);
}


int main(int argc, const char* argv[])
{
    float   exposure = 0;
    float   range = 0;
    int     cx, cy;
    
    signal(SIGINT, intHandler);
    
    if (argc != 3) {
        printf("focus <exposure> <range>. For instance find 2 100 will find with an exposure\n");
        printf("of 2 seconds and a range of display from (minv-30) to minv+100\n");
        exit(0);
    }
    
    exposure = atof(argv[1]);
    range = atof(argv[2]);
    
    printf("%f %f\n", exposure, range);
    
    cam = new Camera(1);
    cam->Init();
    setup_ui();
    
    cam->SetTemperature(-20.5);
    cam->SetFilter(6);
    
    cam->StartExposure(exposure);
    
    while(cam->ExposureBusy()) {
        char c = cvWaitKey(1);
        switch(c) {
            case 27:
                goto END;
        }
    }
    
    {
        double minv;
        double maxv;
        Point minLoc;
        Point maxLoc;
        
        
        printf("readout\n");
        cam->ReadoutImage();
        
        Mat tmp = cam->GetImage();
        
        Mat destination = Mat(tmp.size().width, tmp.size().height, CV_16UC1);
        
        cv::blur(cam->GetImage(), destination, cv::Size(7,7));
        minMaxLoc(destination, &minv, &maxv, &minLoc, &maxLoc );
        imshow("cam", (destination - minv) * 256.0 * (256.0/range));
       
        printf("%d %d\n", maxLoc.x, maxLoc.y);
        cx = maxLoc.x;
        cy = maxLoc.y;
    }
    
    
    do {
        cam->StartExposurePart(exposure, cx, cy);
        
        while(cam->ExposureBusy()) {
            char c = cvWaitKey(1);
            switch(c) {
                case 27:
                    goto END;
            }
        }
        cam->ReadoutImagePart(cx, cy);
        double minv;
        double maxv;
        Point minLoc;
        Point maxLoc;
        
        
        
        minMaxLoc(cam->GetImagePart(), &minv, &maxv, &minLoc, &maxLoc );
        printf("max = %f\n", maxv);
        
        maxv = minv + range;
        minv -= 30;
        
        imshow("cam", (cam->GetImagePart() - minv) * 256.0 * (256.0/range));
        
    } while(1);
    
END:
    cam->Close();
    return(0);
}
