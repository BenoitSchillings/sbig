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
    
    signal(SIGINT, intHandler);
    
    if (argc != 2) {
        printf("find <range>. For instance find 100\n");        exit(0);
    }
    
    exposure = GetValue("find_exposure");
    range = atof(argv[1]);
    
    
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
            
            Mat destination = Mat(cam->GetImage().size().width, cam->GetImage().size().height, CV_16UC1);
            
            cv::blur(cam->GetImage(), destination, cv::Size(14,14));

            minMaxLoc(destination, &minv, &maxv, &minLoc, &maxLoc );
            maxv = minv + range;
            minv += 0;
            
            imshow("cam", (cam->GetImage() - minv) * 256.0 * (256.0/range));
        }
    }while(1);
    
END:
    cam->Close();
    return(0);
}
