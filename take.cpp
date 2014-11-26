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
    float   ao_x0 = 0.0;
    float   ao_y0 = 0.0;
    
    float   guide_exposure = 0.1;
    
    int     frame = 0;
    

    guide_exposure = GetValue( "guide_rate");
    
    
    //QueryDoubleAttribute
    signal(SIGINT, intHandler);
    
    if (argc != 2) {
        printf("take <name>\n");
        exit(0);
    }
    
    float exposure = GetValue("exposure");
    
    cam = new Camera(1);
    printf("x1 %s\n", argv[1]);
   
    cam->Init();
    printf("x1\n");
   
    cam->AO(ao_x, ao_y);
    cam->SetTemperature(GetValue( "temperature"));
    cam->SetFilter(GetValue( "filter"));
    cam->init_guide_dark(guide_exposure);
    
    
    cam->StartExposure(exposure);
    cam->ExposeGuide(guide_exposure, SC_LEAVE_SHUTTER);
    cam->CalcCentroid();
    cam->CalcCentroid();
    cx = (int)cam->CentroidX() + 0.5;
    cy = (int)cam->CentroidY() + 0.5;
    
    float x = 0.5;
    float y = 0.5;

    while(cam->ExposureBusy()) {
        char c = cvWaitKey(1);
        switch(c) {
            case 27:
                goto END;
        }
        {
            //PartBox
            cam->ExposeGuidePartBox(guide_exposure, SC_LEAVE_SHUTTER);
            double minv;
            double maxv;
            Point minLoc;
            Point maxLoc;
            
            minMaxLoc(cam->GetGuide(), &minv, &maxv, &minLoc, &maxLoc);
            minv -= 10;
            printf("%f %f\n", minv, maxv);
            imshow("guide", (cam->GuideCrop() - minv) * 42);
            cam->CalcCentroid();
            float   error_x = cx - cam->CentroidX();
            float   error_y = cy - cam->CentroidY();
            
            printf("error x = %f, ao_x = %f ", error_x, ao_x);
            printf("error y = %f, ao_y = %f\n", error_y, ao_y);
            while (fabs(error_x) > 1.0) {
                error_x /= 4.0;
            }
            while (fabs(error_y) > 1.0) {
                error_y /= 4.0;
            }
            
            if (fabs(error_x) < 0.12) error_x = 0;
            if (fabs(error_y) < 0.12) error_y = 0.0;
            
            ao_x += (error_x / 28.0);
            ao_y -= (error_y / 28.0);
            
            if (ao_x < 0.0) ao_x = 0.0;
            if (ao_x > 1.0) ao_x = 1.0;
            if (ao_y < 0.0) ao_y = 0.0;
            if (ao_y > 1.0) ao_y = 1.0;
            
            
            if (ao_x0 != ao_x || ao_y0 != ao_y) {
                cam->AO(ao_x, ao_y);
            }
            
            ao_x0 = ao_x;
            ao_y0 = ao_y;
            
            frame++;
        }
    }
    cam->ReadoutImage();
    imshow("cam", cam->GetImage() * 3);
    cam->Save(argv[1]);
    
END:
    cam->Close();
    return(0);
}
