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


#define DEBUG(S) {printf(S);printf("\n");}


void intHandler(int dummy=0) {
    printf("emergency close\n");    exit(0);
}


int main(int argc, const char* argv[])
{
    signal(SIGINT, intHandler);
    
    if (argc != 2) {
        printf("filter <filter_number> \n");
        exit(0);
    }
    
    float filter = atof(argv[1]);
    SetValue("filter", filter);
    return(0);
}
