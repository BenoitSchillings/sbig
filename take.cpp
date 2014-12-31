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


void correct(float ao_x, float ao_y)
{
    
    float val_dx = ao_to_dx(ao_x - 0.5, ao_y - 0.5);
    float val_dy = ao_to_dy(ao_x - 0.5, ao_y - 0.5);
    
    float correct_x = error_to_tx(val_dx, val_dy);
    float correct_y = error_to_ty(val_dx, val_dy);
    
    
    printf("correct\n");
    cam->Relay(correct_x * 1.4, correct_y * 1.4);
 

}

float gain_x = (1.0/44.0);
float gain_y = (1.0/44.0);

void update_gain(float error_x, float error0_x, float error_y, float error0_y)
{
    if (fsign(error_x) != fsign(error0_x)) gain_x *= 0.96;
    if (fsign(error_x) == fsign(error0_x)) gain_x *= 1.08;
    if (fsign(error_y) != fsign(error0_y)) gain_y *= 0.96;
    if (fsign(error_y) == fsign(error0_y)) gain_y *= 1.08;

}


int takeframe(int argc, const char* argv[], int idx)
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
    
    float   guide_exposure;
    int     frame = 0;
 
    guide_exposure = GetValue( "guide_rate");
    float exposure = GetValue("exposure");
    
    cam = new Camera(1);
    cam->Init();
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
    
    float error_x = 0;
    float error_y = 0;
    float error0_x = 0;
    float error0_y = 0;
    float correct_x = 0;
    float correct_y = 0; 

    float t0 = cam->time();
    
    while(cam->ExposureBusy()) {
        char c = cvWaitKey(1);
        switch(c) {
            case 27:
                goto END0;
        }
        {
            cam->ExposeGuidePartBox(guide_exposure, SC_LEAVE_SHUTTER);
            double minv;
            double maxv;
            Point minLoc;
            Point maxLoc;
            
            minMaxLoc(cam->GetGuide(), &minv, &maxv, &minLoc, &maxLoc);
            minv -= 10;
            imshow("guide", (cam->GuideCrop() - minv) * 22);
            cam->CalcCentroid();
            
            float   error_x = cx - cam->CentroidX();
            float   error_y = cy - cam->CentroidY();
            
            printf("tt = %f, error x = %f, ao_x = %f gain_x = %f   ", cam->time() - t0, error_x, ao_x, gain_x);
            printf("error y = %f, ao_y = %f gain_y = %f\n", error_y, ao_y, gain_y);
            
            if (error_x < -2) error_x = -2;
            if (error_y < -2) error_y = -2;
            if (error_x > 2) error_x = 2;
            if (error_y > 2) error_y = 2;
            
            if (fabs(error_x) < 0.1) error_x = 0.0;
            if (fabs(error_y) < 0.1) error_y = 0.0;
            
            update_gain(error_x, error0_x, error_y, error0_y);
            
            error0_x = error_x;
            error0_y = error_y;
            
            ao_x += error_x * gain_x;
            ao_y -= error_y * gain_y;
            
            if (ao_x0 != ao_x || ao_y0 != ao_y) {
                cam->AO(ao_x, ao_y);
                usleep(100);
            }
            
            float error = sqrt(pow(ao_x - 0.5, 2) + pow(ao_y - 0.5, 2));
            
            if (error > 0.06 && cam->time() - last_correction > 35) {
                last_correction = cam->time();
                correct(ao_x, ao_y);
            }
            
            ao_x0 = ao_x;
            ao_y0 = ao_y;
          
            frame++;
        }
    }
    
    correct(ao_x, ao_y);
 
    cam->ReadoutImage();
    char buf[256];
    
    sprintf(buf, "%s%d.fit", argv[1], idx);
    cam->Save(buf);
    
END:
    cam->Close();
    return(0);
    
END0:
    cam->Close();
    return -1;
}


int main(int argc, const char* argv[])
{
    int i;
    
    
    if (argc != 2) {
        printf("take <name>\n");
        exit(0);
    }

    signal(SIGINT, intHandler);
    
    for (i = 0; i < 30; i++) {
        int result = takeframe(argc, argv, i);
        if (result < 0)
            return -1;
    }
}


