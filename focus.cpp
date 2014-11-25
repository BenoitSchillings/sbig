#include <stdio.h>
#include <SBIGUDrv/SBIGUDrv.h>
#include "highgui.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "cam.h"
#include <signal.h>
#include "util.h"

using namespace cv;
using namespace std;

Camera *cam;

#define DEBUG(S) {printf(S);printf("\n");}


void setup_ui()
{
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
        printf("focus <exposure> <range>. For instance focus 2 100 will find with an exposure\n");
        printf("of 2 seconds and a range of display from (minv-30) to minv+100\n");
        exit(0);
    }
    
    exposure = atof(argv[1]);
    range = atof(argv[2]);
    
    printf("%f %f\n", exposure, range);
    
    cam = new Camera(1);
    cam->Init();
    setup_ui();
    
    cam->SetTemperature(GetValue( "temperature"));
    cam->SetFilter(GetValue( "filter"));
    
    cam->StartExposure(exposure);
    Mat zoom;
   
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
        
        
        cam->ReadoutImage();
        
        Mat tmp = cam->GetImage();
        
        Mat destination = Mat(tmp.size().width, tmp.size().height, CV_16UC1);
        
        cv::blur(cam->GetImage(), destination, cv::Size(7,7));
        minMaxLoc(destination, &minv, &maxv, &minLoc, &maxLoc );
        imshow("cam", (destination - minv) * 256.0 * (256.0/range));
       
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
        resize((cam->GetImagePart() - minv) * 256.0 * (256.0/range),
               zoom,
               Size(0, 0),
               6, 6,
               INTER_NEAREST);

        imshow("cam", zoom);
        
    } while(1);
    
END:
    cam->Close();
    return(0);
}
