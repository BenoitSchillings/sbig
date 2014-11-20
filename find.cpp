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
    
    signal(SIGINT, intHandler);
    
    if (argc != 3) {
        printf("find <exposure> <range>. For instance find 2 100 will find with an exposure\n");
        printf("of 2 seconds and a range of display from (minv-30) to minv+100\n");
        exit(0);
    }
    
    exposure = atof(argv[1]);
    range = atof(argv[2]);
    
    printf("%f %f\n", exposure, range);
    
    
    cam = new Camera(GetValue( "find_bin"));
    cam->Init();
    setup_ui();
    
    cam->SetTemperature(GetValue( "temperature"));
    cam->SetFilter(GetValue( "filter"));
    
    
    do {
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
            
            
            cam->ReadoutImage();
            
            minMaxLoc(cam->GetImage(), &minv, &maxv, &minLoc, &maxLoc );
            maxv = minv + range;
            minv -= 30;
            
            imshow("cam", (cam->GetImage() - minv) * 256.0 * (256.0/range));
        }
    }while(1);
    
END:
    cam->Close();
    return(0);
}
