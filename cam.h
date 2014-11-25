#include <SBIGUDrv/SBIGUDrv.h>
#include "highgui.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define IMAGING 0
#define GUIDING 1

#define BIN11   0
#define BIN22   1
#define BIN33   2
#define BIN99   9


class Camera {
    int     m_eCameraType;
    Mat     imaging;
    Mat     imagingPart;
    
    Mat     guiding;
    Mat     guiding_dark;
    char    has_guide_dark;
    Mat     centroid;
    
    int     image_x;
    int     image_y;
    int     guide_x;
    int     guide_y;
    long    base_time;
    int     guide_bin;
    int     image_bin;
    
    float   centroid_x;
    float   centroid_y;
    
    int     hint_x;
    int     hint_y;
    float   guide_bias;
    int     guide_box_size;
    int     focus_box;
    
public:
    Camera(int bin);
    void    Init();
    void    Close();
    float   GetTemperature();
    void    SetTemperature(float temp);
    int     EstablishLink();
    char    ExposureBusy();
    char    GuideBusy();
    int     ReadoutImage();
    int     ReadoutImagePart(int cx, int cy);
    int     ReadoutGuidePart(int y1, int y2);
    void    Save(const char *filename);
    void    StartExposure(float duration);
    void    StartExposurePart(float duration, int cx, int cy);
    void    ExposeGuide(float duration, int shutter);
    void    ExposeGuidePart(float duration, int shutter, int y1, int y2);
    void    ExposeGuidePartBox(float duration, int shutter);
    
    void    CalcCentroid();
    void    Relay(float dx, float dy);
    void    AO(float x, float y);
    void    SetFilter(int filter);
    float   GP(int x, int y);
    
    void    init_guide_dark(float exposure);
    Mat     GuideCrop();


    
    float   CentroidX();
    float   CentroidY();

    Mat     GetImage();
    Mat     GetGuide();
    Mat     GetImagePart();
    
    //Random stuff which really doesn't need to be here.
    
    double  time();
    
private:
    unsigned short DegreesCToAD(double degC);
    void    WriteLine(FILE *file, int y);
    void    DumpGuideLines(int cnt);
    void    DumpImageLines(int cnt);
    int     binval(int binres);
    int     DrvCommand(short command, void *Params, void *pResults);


};