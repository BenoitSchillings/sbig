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
    
    float   guide_exposure;
    
    int     frame = 0;
    

    guide_exposure = GetValue( "guide_rate");
   
    
    signal(SIGINT, intHandler);
    
    cam = new Camera(1);
   
    cam->Init();
   
    cam->AO(ao_x, ao_y);
    cam->SetTemperature(GetValue( "temperature"));
    cam->SetFilter(GetValue( "filter"));
    cam->init_guide_dark(guide_exposure);
    
    
    int xx1, yy1;
    int xx2, yy2;
    int xx3, yy3;
    
    cam->ExposeGuide(guide_exposure, SC_OPEN_SHUTTER);
    cam->MaxPrivate(&xx1, &yy1);
    cam->Relay(2, 0);
    sleep(2);
    
    cam->ExposeGuide(guide_exposure, SC_OPEN_SHUTTER);
    cam->MaxPrivate(&xx2, &yy2);
    
    cam->Relay(-2, 0);
    sleep(2);
    cam->Relay(0, 2);
    sleep(2);
    
    cam->ExposeGuide(guide_exposure, SC_OPEN_SHUTTER);
    cam->MaxPrivate(&xx3, &yy3);

    cam->Relay(0, -2);
    sleep(2);
    
    printf("%d %d\n", xx1, yy1);
    printf("%d %d\n", xx2, yy2);
    printf("%d %d\n", xx3, yy3);
    
    SetValue("mount_dx1", (xx2-xx1));
    SetValue("mount_dy1", (yy2-yy1));
    SetValue("mount_dx2", (xx3-xx1));
    SetValue("mount_dy2", (yy3-yy1));
    
    cam->AO(0.5, 0.5);
    sleep(1);
    
    cam->ExposeGuide(guide_exposure, SC_OPEN_SHUTTER);
    cam->MaxPrivate(&xx1, &yy1);
    
    cam->AO(0.75, 0.5);
    sleep(1);
    cam->ExposeGuide(guide_exposure, SC_OPEN_SHUTTER);
    cam->MaxPrivate(&xx2, &yy2);

    cam->AO(0.5, 0.75);
    sleep(1);
    cam->ExposeGuide(guide_exposure, SC_OPEN_SHUTTER);
    cam->MaxPrivate(&xx3, &yy3);
    cam->AO(0.5, 0.5);
    
    //dx1 is how much x motion you will get for a delta on AO x of 1.0
    printf("%d %d\n", xx1, yy1);
    printf("%d %d\n", xx2, yy2);
    printf("%d %d\n", xx3, yy3);

    SetValue("ao_dx1", (xx2-xx1) * 4.0);
    SetValue("ao_dy1", (yy2-yy1) * 4.0);
    SetValue("ao_dx2", (xx3-xx1) * 4.0);
    SetValue("ao_dy2", (yy3-yy1) * 4.0);

    cam->Close();
    return(0);
}
