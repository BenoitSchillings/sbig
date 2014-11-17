#include <stdio.h>
#include <SBIGUDrv/SBIGUDrv.h>
#include "highgui.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "cam.h"
#include "./tiny/tinyxml.h"
#include <signal.h>
#include "util.h"

using namespace cv;
using namespace std;

Camera *cam;

#define DEBUG(S) {printf(S);printf("\n");}



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
    
    float   gain_x = 0.9;
    float   gain_y = -0.6;
    
    float   guide_exposure = 0.1;
    

    guide_exposure = GetValue( "guide_rate");
    
    printf("%f\n", guide_exposure);
    
    //QueryDoubleAttribute
    signal(SIGINT, intHandler);
    
    if (argc != 3) {
        printf("take <exposure> <name>. For instance take 2 take an exposure of 2 seconds\n");
        exit(0);
    }
    
    float exposure = atof(argv[1]);
    
    cam = new Camera(1);
    cam->Init();
    
    cam->AO(ao_x, ao_y);
    cam->SetTemperature(-20.5);
    cam->SetFilter(6);
    cam->init_guide_dark(guide_exposure);
    
    
    cam->StartExposure(exposure);
    cam->ExposeGuide(guide_exposure, SC_LEAVE_SHUTTER);
    cam->CalcCentroid();
    cx = 0.5 + cam->CentroidX();
    cy = 0.5 + cam->CentroidY();
    
    float x = 0.05;
    float y = 0.05;
    while(cam->ExposureBusy()) {
        char c = cvWaitKey(1);
        switch(c) {
            case 27:
                goto END;
        }
        {
            cam->ExposeGuidePartBox(guide_exposure, SC_LEAVE_SHUTTER);
            double minv;
            double maxv;
            Point minLoc;
            Point maxLoc;
            
            minMaxLoc(cam->GetGuide(), &minv, &maxv, &minLoc, &maxLoc);
            minv -= 10;
            printf("%f %f\n", minv, maxv);
            imshow("guide", (cam->GetGuide() - minv) * 132);
            cam->CalcCentroid();
            
            float   error_x = cx - cam->CentroidX();
            float   error_y = cy - cam->CentroidY();
            
            printf("error <%f> %f %f, %f %f\n", cy, error_x, error_y, ao_x, ao_y);
            ao_x += 0.001;
            if ((error_x * error_x + error_y * error_y) < 20) {
                
                ao_x += gain_x * error_x / 100.0;
                ao_y += gain_y * error_y / 100.0;
                cam->AO(ao_x, ao_y);
            }
            
        }
    }
    cam->ReadoutImage();
    imshow("cam", cam->GetImage() * 3);
    cam->Save(argv[2]);
    
END:
    cam->Close();
    return(0);
}
