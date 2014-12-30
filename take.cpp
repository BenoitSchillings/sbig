#include <stdio.h>
#include <SBIGUDrv/SBIGUDrv.h>
#include "highgui.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "cam.h"
#include "./tiny/tinyxml.h"
#include <signal.h>
#include "util.h"
#include <unistd.h>

using namespace cv;
using namespace std;

Camera *cam;

#define DEBUG(S) {printf(S);printf("\n");}

float mount_dx1, mount_dy1;
float mount_dx2, mount_dy2;

float ao_dx1, ao_dy1;
float ao_dx2, ao_dy2;

void intHandler(int dummy=0) {
    printf("emergency close\n");
    cam->Close();
    exit(0);
}

float error_to_tx(float mx, float my)
{
    return (my*mount_dx2 - mx*mount_dy2)/(mount_dx2*mount_dy1 - mount_dx2*mount_dy2);
}

float error_to_ty(float mx, float my)
{
    return (my*mount_dx1 - mx*mount_dy1)/(mount_dx1*mount_dy2 - mount_dx2*mount_dy1);
}


float ao_to_dx(float ao_x, float ao_y)
{
    return ao_x * ao_dx1 + ao_y * ao_dx2;
}


float ao_to_dy(float ao_x, float ao_y)
{
    return ao_x * ao_dy1 + ao_y * ao_dy2;
}


float fsign(float x)
{
    if (x < 0.0)
        return -1.0;
    if (x > 0.0)
        return 1.0;
    return 0;
}

int main(int argc, const char* argv[])
{
    float   cx = -1;
    float   cy = -1;
    
    float   ao_x = 0.5;
    float   ao_y = 0.5;
    float   ao_x0 = 0.0;
    float   ao_y0 = 0.0;
    double  last_correction = 0;
    
    mount_dx1 = GetValue("mount_dx1");
    mount_dx2 = GetValue("mount_dx2");
    mount_dy1 = GetValue("mount_dy1");
    mount_dy2 = GetValue("mount_dy2");
    
    ao_dx1 = GetValue("ao_dx1");
    ao_dx2 = GetValue("ao_dx2");
    ao_dy1 = GetValue("ao_dy1");
    ao_dy2 = GetValue("ao_dy2");
    
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
    float error_x = 0;
    float error_y = 0;
    float error0_x = 0;
    float error0_y = 0;
    
    float gain_x = (1.0/44.0);
    float gain_y = (1.0/44.0);

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
            //printf("%f %f\n", minv, maxv);
            imshow("guide", (cam->GuideCrop() - minv) * 22);
            cam->CalcCentroid();
            float   error_x = cx - cam->CentroidX();
            float   error_y = cy - cam->CentroidY();
            
            printf("error x = %f, ao_x = %f gain_x = %f   ", error_x, ao_x, gain_x);
            printf("error y = %f, ao_y = %f gain_y = %f\n", error_y, ao_y, gain_y);
            while (fabs(error_x) > 2.0) {
                error_x /= 4.0;
            }
            while (fabs(error_y) > 2.0) {
                error_y /= 4.0;
            }
            
            if (fabs(error_x) < 0.1) error_x = 0.0;
            if (fabs(error_y) < 0.1) error_y = 0.0;
            
            if (fsign(error_x) != fsign(error0_x)) {
                gain_x *= 0.95;
                printf("less gain_x\n");
            }
            
            if (fsign(error_x) == fsign(error0_x)) {
                gain_x *= 1.1;
                printf("more gain_x\n");
            }
            
            if (fsign(error_y) != fsign(error0_y)) {
                gain_y *= 0.95;
                printf("less gain_y\n");
            }
            
            if (fsign(error_y) == fsign(error0_y)) {
                gain_y *= 1.1;
                printf("more gain_y\n");
            }
            
            error0_x = error_x;
            error0_y = error_y;
            
            ao_x += error_x * gain_x;
            ao_y -= error_y * gain_y;
            
            if (ao_x < 0.0) ao_x = 0.0;
            if (ao_x > 1.0) ao_x = 1.0;
            if (ao_y < 0.0) ao_y = 0.0;
            if (ao_y > 1.0) ao_y = 1.0;
            
            
            if (ao_x0 != ao_x || ao_y0 != ao_y) {
                cam->AO(ao_x, ao_y);
                usleep(100);
            }
            
            float error = (ao_x - 0.5) * (ao_x - 0.5);
            error += (ao_y - 0.5) * (ao_y - 0.5);
            error = sqrt(error);
            
            if (error > 0.06) {
                float val_dx = ao_to_dx(ao_x - 0.5, ao_y - 0.5);
                float val_dy = ao_to_dy(ao_x - 0.5, ao_y - 0.5);
            
                float correct_x = error_to_tx(val_dx, val_dy);
                float correct_y = error_to_ty(val_dx, val_dy);
            
                printf("frame %d, vx = %f vy = %f cor_x = %f cor_y = %f\n", frame, val_dx, val_dy, correct_x, correct_y);
            
                // correct every 15 second at most
                if (cam->time() - last_correction > 35) {

                    printf("correct\n");
                    cam->Relay(correct_x, correct_y);
                    last_correction = cam->time();
                }
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
