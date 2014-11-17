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
    float   cx = -1;
    float   cy = -1;
    
    float   ao_x = 0.5;
    float   ao_y = 0.5;
    
    float   gain_x = 1.0;
    float   gain_y = 1.0;
    
    
    signal(SIGINT, intHandler);
    
    if (argc != 3) {
        printf("findguide <exposure> <range>. \n");
        exit(0);
    }
    
    float exposure = atof(argv[1]);
    float range = atof(argv[2]);
    cam = new Camera(1);
    cam->Init();
    setup_ui();
    
    cam->AO(ao_x, ao_y);
    cam->SetTemperature(-20.5);
    cam->SetFilter(6);
    cam->init_guide_dark(exposure);
    
    do {
        cam->ExposeGuide(exposure, SC_OPEN_SHUTTER);
        
        char c = cvWaitKey(1);
        switch(c) {
            case 27:
                goto END;
        }
        
        
        {
            double minv;
            double maxv;
            Point minLoc;
            Point maxLoc;
            
            minMaxLoc(cam->GetGuide(), &minv, &maxv, &minLoc, &maxLoc );
            minv -= 10;
            imshow("guide", (cam->GetGuide() - minv)  * 256.0 * (256.0/range));
        }
    }
    while(1);
    
END:
    cam->Close();
    return(0);
}
