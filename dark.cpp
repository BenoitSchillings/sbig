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
     int     frame = 0;
    

    signal(SIGINT, intHandler);
    
    if (argc != 2) {
        printf("dark <name>\n");
        exit(0);
    }
    
    float exposure = GetValue("dark_exposure");
    
    cam = new Camera(1);
    printf("x1 %s\n", argv[1]);
   
    cam->Init();
   
    cam->SetTemperature(GetValue( "temperature"));
    cam->SetFilter(GetValue( "filter"));
    
    
    cam->StartDark(exposure);
    
    float x = 0.5;
    float y = 0.5;

    while(cam->ExposureBusy()) {
        char c = cvWaitKey(1);
        switch(c) {
            case 27:
                goto END;
        }
        
    }
    cam->ReadoutImage();
    imshow("cam", cam->GetImage() * 3);
    cam->Save(argv[1]);
    
END:
    cam->Close();
    return(0);
}
