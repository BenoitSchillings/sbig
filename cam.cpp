#include <stdio.h>
#include <SBIGUDrv/SBIGUDrv.h>
#include "highgui.h"
#include "cam.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <mach/mach_time.h>
#include "util.h"

using namespace cv;
using namespace std;


#define DEBUG(...) {printf(__VA_ARGS__);printf("\n");}

int Camera::DrvCommand(short command, void *Params, void *pResults)
{
    int result;
    
    result = SBIGUnivDrvCommand(command, Params, pResults);
    //return 0;
    if (result != 0) {
        printf("Camera error %d for command <%d>\n", result, command);
        exit(-1);
    }
    return result;
}



Camera::Camera(int bin)
{
    base_time = mach_absolute_time();
    guide_bin = 3;
    image_bin = bin;
    
    image_x = 2184/image_bin;
    image_y = 1472/image_bin;
    
    guide_x = 640 / guide_bin;
    guide_y = 480 / guide_bin;
    
    guide_box_size = (int)GetValue("guide_box") / 2;
    star_box_size = (int)GetValue("star_box") / 2;
    
    imaging = Mat(Size(image_x, image_y), CV_16UC1);
    guiding = Mat::zeros(Size(guide_x, guide_y), CV_16UC1);
    guiding_dark = Mat(Size(guide_x, guide_y), CV_16UC1);
    focus_box = GetValue("focus_box");
    imagingPart = Mat(Size(focus_box, focus_box), CV_16UC1);
    guiding_dark.setTo(0);
    has_guide_dark = FALSE;
    hint_x = -1;
    hint_y = -1;
    guide_bias = 0;
}

void Camera::SetFilter(int filter)
{
    CFWParams   command;
    CFWResults  result;
    
    command.cfwModel =  CFWSEL_CFW10;
    command.cfwCommand = 1;
    command.cfwParam1 = filter;
    
    DrvCommand (CC_CFW, &command, &result);
}

double Camera::time()
{
    return (mach_absolute_time() - base_time) / (double)1e9;
}

void Camera::Init()
{
    DrvCommand(CC_OPEN_DRIVER, NULL, NULL);
    OpenDeviceParams param;
    
    param.deviceType = 0x7F00;
    
    DrvCommand(CC_OPEN_DEVICE, &param, NULL);
    EstablishLink();
}

void Camera::Close()
{
    DrvCommand(CC_CLOSE_DEVICE, NULL, NULL);
    DrvCommand(CC_CLOSE_DRIVER, NULL, NULL);
}

int Camera::EstablishLink()
{
    EstablishLinkResults elr;
    EstablishLinkParams elp;
    
    DrvCommand(CC_ESTABLISH_LINK, &elp, &elr);
    return 0;
}


void Camera::SetTemperature(float t)
{
    SetTemperatureRegulationParams strp;
    
    strp.regulation = REGULATION_ON;
    strp.ccdSetpoint = DegreesCToAD(t);
    DrvCommand(CC_SET_TEMPERATURE_REGULATION, &strp, NULL);
}

float Camera::GetTemperature()
{
    QueryTemperatureStatusResults2 temp_data;
    QueryTemperatureStatusParams temp_request;
    
    temp_request.request = TEMP_STATUS_ADVANCED2;
    
    DrvCommand(CC_QUERY_TEMPERATURE_STATUS, &temp_request, &temp_data);
    
    return temp_data.imagingCCDTemperature;
}

Mat Camera::GetImage()
{
    return imaging;
}

Mat Camera::GetGuide()
{
    /*
     guiding.setTo(50);
     guiding.at<ushort>(40, 40) = 1000;
     guiding.at<ushort>(41, 40) = 1000;
     guiding.at<ushort>(40, 41) = 1000;
     guiding.at<ushort>(41, 41) = 700;
     */
    return guiding;
}

Mat Camera::GuideCrop()
{
    Mat tmp;
    
    tmp = Mat(guiding, Rect(hint_x - guide_box_size, hint_y - guide_box_size, guide_box_size*2, guide_box_size * 2));
    resize(tmp, tmp, Size(0, 0), 6, 6, INTER_NEAREST);
    return tmp;
}

Mat Camera::GetImagePart()
{
    return imagingPart;
}


char Camera::ExposureBusy()
{
    QueryCommandStatusParams    params;
    QueryCommandStatusResults   res;
    
    params.command = CC_START_EXPOSURE;
    DrvCommand(CC_QUERY_COMMAND_STATUS, &params, &res);
    
    
    if ((res.status & 0x03) == 2)
    return 1;
    
    return 0;
}

char Camera::GuideBusy()
{
    QueryCommandStatusParams    params;
    QueryCommandStatusResults   res;
    
    params.command = CC_START_EXPOSURE;
    DrvCommand(CC_QUERY_COMMAND_STATUS, &params, &res);
    
    
    if (((res.status>>2) & 0x03) == 2)
    return 1;
    
    return 0;
}


void Camera::StartExposurePart(float duration, int cx, int cy)
{
    StartExposureParams2    params;
    
    params.exposureTime = duration * 100.0;
    params.ccd = IMAGING; //imaging ccd
    params.abgState = 0;
    
    
    params.readoutMode = binval(image_bin);
    
    params.top = cy - focus_box/2;
    params.left = cx + focus_box/2;
    params.height = focus_box;
    params.width = focus_box;
    
    params.openShutter = SC_OPEN_SHUTTER;
    
    DrvCommand(CC_START_EXPOSURE2, &params, NULL);
}



void Camera::StartExposure(float duration)
{
    StartExposureParams2    params;
    
    params.exposureTime = duration * 100.0;
    params.ccd = IMAGING; //imaging ccd
    params.abgState = 0;
    
    params.readoutMode = binval(image_bin);
    
    params.top = 0;
    params.left = 0;
    params.height = image_y;
    params.width = image_x;
    
    params.openShutter = SC_OPEN_SHUTTER;
    
    DrvCommand(CC_START_EXPOSURE2, &params, NULL);
}


void Camera::ExposeGuidePartBox(float duration, int shutter)
{
    ExposeGuidePart(duration, shutter, hint_y - guide_box_size, hint_y + guide_box_size);
}



void Camera::ExposeGuide(float duration, int shutter)
{
    ExposeGuidePart(duration, shutter, 0, guide_y);
}


void Camera::ExposeGuidePart(float duration, int shutter, int y1, int y2)
{
    StartExposureParams2    params;
    
    if (y1 < 0) y1 = 0;
    if (y1 > guide_y) y1 = guide_y;
    if (y2 < 0) y2 = 0;
    if (y2 > guide_y) y2 = guide_y;
    
    params.exposureTime = duration * 100.0;
    params.ccd = GUIDING | START_SKIP_VDD; //guide ccd
    params.abgState = 0;
    
    params.readoutMode = binval(guide_bin);
    
    params.top = y1;
    params.left = 0;
    params.height = y2;
    params.width = guide_x;
    params.openShutter = shutter;
    
    DrvCommand(CC_START_EXPOSURE2, &params, NULL);
    
    do {
    } while(GuideBusy());
    
    ReadoutGuidePart(y1, y2);
}


void    Camera::init_guide_dark(float exposure)
{
#define DKCOUNT 3
    int cnt = DKCOUNT;
    
    // skip one
    ExposeGuide(exposure, SC_CLOSE_SHUTTER);
    
    guiding_dark.setTo(0);
    while(cnt > 0) {
        ExposeGuide(exposure, SC_CLOSE_SHUTTER);
        guiding_dark = guiding_dark + (1.0/DKCOUNT) * GetGuide();
        cnt--;
        printf("guide dark %d\n", cnt);
    }
    guiding_dark = guiding_dark - 30; 
    has_guide_dark = TRUE;
}


#include "fits_header.h"

void Camera::WriteLine(FILE *file, int y)
{
    ushort  tmp[16384];      //large enough for my camera
    
    ushort  *src = imaging.ptr<ushort>(y);
    
    int     x;
    
    for (int x = 0; x < image_x; x++) {
        ushort  v;
        
        v = *src++;
        v = v + 32768;
        
        v = (v>>8) | ((v&0xff) << 8);
        
        tmp[x] = v;
    }
    fwrite(tmp, image_x, 2, file);
}

float Camera::CentroidX()
{
    return centroid_x;
}

float Camera::CentroidY()
{
    return centroid_y;
}


float Camera::GP(int x, int y)
{
    return  GetGuide().at<ushort>(y, x);
}


void Camera::MaxPrivate(int *xx, int *yy)
{
    int     x,y;
    Point   maxLoc;
    int xp, yp;
    
    double   maxv = 0;
    double   minv = 1e9;
    
    for (yp = guide_box_size; yp <= guide_y-guide_box_size; yp++) {
        for (xp = guide_box_size; xp <= guide_x-guide_box_size; xp++) {
            float   val = GP(xp,yp) +
            GP(xp+1, yp) +
            GP(xp-1, yp) +
            GP(xp, yp - 1) +
            GP(xp, yp + 1);
            
            if (val > maxv) {
                maxv = val;
                maxLoc.x = xp;
                maxLoc.y = yp;
            }
        }
    }
    *xx = maxLoc.x;
    *yy = maxLoc.y;
}


void Camera::CalcCentroid()
{
    int     x,y;
    Point   maxLoc;
    int xp, yp;
    
    double   maxv = 0;
    double   minv = 1e9;
    
    if (hint_x < 0) {
        for (yp = guide_box_size; yp <= guide_y-guide_box_size; yp++) {
            for (xp = guide_box_size; xp <= guide_x-guide_box_size; xp++) {
                float   val = GP(xp,yp) +
                GP(xp+1, yp) +
                GP(xp-1, yp) +
                GP(xp, yp - 1) +
                GP(xp, yp + 1);
                
                if (val > maxv) {
                    maxv = val;
                    maxLoc.x = xp;
                    maxLoc.y = yp;
                }
            }
        }
        
        
        hint_x = x = maxLoc.x;
        hint_y = y = maxLoc.y;
    }
    else {
        x = hint_x;
        y = hint_y;
    }
    
    
    
    
    
    if (guide_bias == 0.0) {
        guide_bias = GetGuide().at<ushort>(x-guide_box_size,y-guide_box_size);
    }
    else {
        //low pass moving bias
        guide_bias = guide_bias * 9.0 + GetGuide().at<ushort>(x-guide_box_size,y-guide_box_size);
        guide_bias /= 10.0;
    }
    
    float xsum, ysum, total;
    
    xsum = 0;
    ysum = 0;
    total = 0;
    
    for (yp = (y - guide_box_size); yp <= (y + guide_box_size); yp++) {
        for (xp = (x - guide_box_size); xp <= (x + guide_box_size); xp++) {
            float  pix;
            
            pix = GP(xp,yp);
            
            pix -= (guide_bias + 8);
            if (pix < 0) pix = 0;
            xsum += (float)xp * pix;
            ysum += (float)yp * pix;
            total += pix;
        }
    }
    xsum /= total;
    ysum /= total;
    
    //printf("%f %f\n", xsum, ysum);
    
    x = centroid_x = xsum;
    y = centroid_y = ysum;
    
    xsum = 0;
    ysum = 0;
    total = 0;
    
    for (yp = (y - star_box_size); yp <= (y + star_box_size); yp++) {
        for (xp = (x - star_box_size); xp <= (x + star_box_size); xp++) {
            float  pix;
            
            pix = GP(xp,yp);
            
            pix -= (guide_bias + 8);
            if (pix < 0) pix = 0;
            xsum += (float)xp * pix;
            ysum += (float)yp * pix;
            total += pix;
        }
    }
    xsum /= total;
    ysum /= total;
    
    //printf("%f %f\n", xsum, ysum);
    
    centroid_x = xsum;
    centroid_y = ysum;
    
    
}

void Camera::Save(const char *filename)
{
    FILE *file = fopen(filename, "wb");
    
    char  header_buf[0xb40];
    
    int i;
    
    for (i = 0; i < 0xb40; i++) header_buf[i] = ' ';
    
    i = 0;
    
    do {
        const char*   header_line;
        
        header_line = header[i];
        
        if (strlen(header_line) > 0) {
            memcpy(&header_buf[i*80], header_line, strlen(header_line));
        }
        else
        break;
        i++;
    } while(i < 40);
    
    fwrite(header_buf, 0xb40, 1, file);
    
    int     y;
    
    for (y = 0; y < image_y; y++) {
        WriteLine(file, y);
    }
    
    fclose(file);
}

int Camera::ReadoutGuidePart(int cy0, int cy1)
{
    
    {
        EndExposureParams    params;
        
        params.ccd = GUIDING | END_SKIP_DELAY;
        
        DrvCommand(CC_END_EXPOSURE, &params, NULL);
    }
    
    {
        StartReadoutParams  params;
        
        params.ccd = GUIDING; //guide ccd
        
        params.readoutMode = binval(guide_bin);
        
        
        params.top = cy0;
        params.left = 0;
        params.height = cy1;
        params.width = guide_x;
        
        DrvCommand(CC_START_READOUT, &params, NULL);
    }
    
    int     y;
    
    for (y = cy0; y < cy1; y++) {
        ReadoutLineParams   params;
        
        params.ccd = GUIDING;
        
        params.readoutMode = binval(guide_bin);
        
        params.pixelStart = 0;
        params.pixelLength = guide_x;
        
        ushort *ptr;
        ushort *dark;
        
        ptr = guiding.ptr<ushort>(y);
        
        DrvCommand(CC_READOUT_LINE, &params, ptr);
        
        
        if (has_guide_dark) {
            dark = guiding_dark.ptr<ushort>(y);
            for (int x = 0; x < guide_x; x++) {
                *ptr = (*ptr + 200) - *dark;
                ptr++;dark++;
            }
        }
    }
    
    {
        EndReadoutParams  params;
        
        params.ccd = GUIDING;
    }
    return 0;
}


void Camera::AO(float x, float y)
{
    AOTipTiltParams   params;
    
    if (x < 0.0) x = 0;
    if (y < 0.0) y = 0;
    if (x > 1.0) x = 1.0;
    if (y > 1.0) y = 1.0;
    
    params.xDeflection = x * 4095.0;
    params.yDeflection = y * 4095.0;
    
    DrvCommand(CC_AO_TIP_TILT, &params, NULL);
}

void Camera::Relay(float dx, float dy)
{
    ActivateRelayParams     params;
    
    printf("Relay %f %f\n", dx, dy);
    params.tXPlus = 0;
    params.tXMinus = 0;
    params.tYPlus = 0;
    params.tYMinus = 0;
    
    if (dx < 0) {
        params.tXMinus = -dx * 100.0;
    }
    if (dx > 0) {
        params.tXPlus = dx * 100.0;
    }
    
    
    if (dy < 0) {
        params.tYMinus = -dy * 100.0;
    }
    if (dy > 0) {
        params.tYPlus = dy * 100.0;
    }
    DrvCommand(CC_ACTIVATE_RELAY, &params, NULL);
}

int Camera::ReadoutImage()
{
    {
        EndExposureParams    params;
        
        params.ccd = 0;
        
        int result = DrvCommand(CC_END_EXPOSURE, &params, NULL);
        if (result != 0)
        return result;
    }
    
    {
        StartReadoutParams  params;
        
        
        params.ccd = IMAGING; //imaging ccd
        
        params.readoutMode = binval(image_bin);
        
        
        params.top = 0;
        params.left = 0;
        params.height = image_y;
        params.width = image_x;
        
        DrvCommand(CC_START_READOUT, &params, NULL);
    }
    
    int     y;
    
    for (y = 0; y < image_y; y++) {
        ReadoutLineParams   params;
        
        params.ccd = 0;
        params.readoutMode = binval(image_bin);
        params.pixelStart = 0;
        params.pixelLength = image_x;
        
        ushort *ptr;
        
        ptr = imaging.ptr<ushort>(y);
        
        DrvCommand(CC_READOUT_LINE, &params, ptr);
    }
    
    {
        EndReadoutParams  params;
        
        params.ccd = IMAGING; //imaging ccd
        DrvCommand(CC_END_READOUT, &params, NULL);
    }
    
    return 0;
}

void Camera::DumpGuideLines(int count)
{
    DumpLinesParams     params;
    
    params.ccd = 1;
    params.readoutMode = binval(guide_bin);
    params.lineLength = count;
    DrvCommand(CC_DUMP_LINES, &params, NULL);
    
    
}

void Camera::DumpImageLines(int count)
{
    DumpLinesParams     params;
    
    params.ccd = 0;
    params.readoutMode = binval(image_bin);
    params.lineLength = count;
    DrvCommand(CC_DUMP_LINES, &params, NULL);
}

int Camera::ReadoutImagePart(int cx, int cy)
{
    {
        EndExposureParams    params;
        
        params.ccd = 0;
        
        DrvCommand(CC_END_EXPOSURE, &params, NULL);
    }
    
    {
        StartReadoutParams  params;
        
        
        params.ccd = IMAGING; //imaging ccd
        
        params.readoutMode = binval(image_bin);
        params.top = cy - focus_box/2;
        params.left = cx -  focus_box/2;
        params.height =  focus_box;
        params.width =  focus_box;
        
        DrvCommand(CC_START_READOUT, &params, NULL);
    }
    
    int     y;
    
    for (y = 0; y < focus_box; y++) {
        ReadoutLineParams   params;
        
        params.ccd = 0;
        params.readoutMode = 0;
        params.pixelStart = cx -  focus_box/2;
        params.pixelLength = focus_box;
        
        ushort *ptr;
        
        ptr = imagingPart.ptr<ushort>(y);
        
        DrvCommand(CC_READOUT_LINE, &params, ptr);
    }
    
    {
        EndReadoutParams  params;
        
        params.ccd = IMAGING; //imaging ccd
        DrvCommand(CC_END_READOUT, &params, NULL);
    }
    
    return 0;
}




unsigned short Camera::DegreesCToAD(double degC)
{
	double r;
	unsigned short setpoint;
    
#define T0      25.0
#define R0       3.0
#define DT_CCD  25.0
#define RR_CCD   2.57
#define RB_CCD  10.0
#define MAX_AD  4096
    
	
	if ( degC < -50.0 )
    degC = -50.0;
	else if ( degC > 35.0 )
    degC = 35.0;
    
    r = R0 * exp(log(RR_CCD)*(T0 - degC)/DT_CCD);
    setpoint = (unsigned short)(MAX_AD/((RB_CCD/r) + 1.0) + 0.5);
	return setpoint;
}


int Camera::binval(int binres)
{
    if (binres == 1) {
        return BIN11;
    }
    if (binres == 2) {
        return BIN22;
    }
    if (binres == 3) {
        return BIN33;
    }
    
    if (binres == 9) {
        return BIN99;
    }
    return BIN11;
}


;

