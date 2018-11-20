#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

static void edge_CB(int, void*);
static void rect_CB(int, void*);
static void color_CB(int, void*);
void setupWindow();
bool compareArea(vector<Point> a, vector<Point> b);
Mat drawInfo(Mat src, vector<RotatedRect> minRects);
Mat preProcess(Mat input);
vector<RotatedRect> findObj( Mat input);
string classifyObj(RotatedRect minRect);

int low_thres=100;
int up_thres=200;
int dilate_itera = 3;
int n_obj = 3;
int ratioErrTolerance = 50;
int redLowHSV[] = {176,151,128};
int redHighHSV[] = {179,220,230};
int greenLowHSV[] = {65,50,85};
int greenHighHSV[] = {75,255,255};
static int MAX_ITERATE = 20;
static int MAX_THRES = 300;
static int MAX_N_obj = 10;
char win_name[] = "Display Image";
char ctrl_win[] = "Control Panel";
Mat src,srcR,srcG;

int main(int argc, char** argv ){
    src = imread( argv[1], 1);
    setupWindow();
    color_CB(0,0);
    edge_CB(0,0);
    rect_CB(0,0);
    while(1){
        int pressedKey = waitKey(0);
        if( pressedKey == 113) break;//press q key to quit
        if( pressedKey == 'e') edge_CB(0,0);//press e key to show edge
        if( pressedKey == 'r') rect_CB(0,0);//press r key to show bounding rectangle
        if( pressedKey == 'w') color_CB(0,0);//press w key to show color mask
    } 
    return 0;
}

static void color_CB(int, void*){
    Mat srcHSV;
    cvtColor(src, srcHSV, COLOR_BGR2HSV);
    inRange(srcHSV, Scalar(greenLowHSV[0],greenLowHSV[1],greenLowHSV[2]), Scalar(greenHighHSV[0],greenHighHSV[1],greenHighHSV[2]), srcG);
    inRange(srcHSV, Scalar(redLowHSV[0],redLowHSV[1],redLowHSV[2]), Scalar(redHighHSV[0],redHighHSV[1],redHighHSV[2]), srcR);    
    dilate(srcR,srcR, Mat(), Point(-1,-1), dilate_itera);
    dilate(srcG,srcG, Mat(), Point(-1,-1), dilate_itera);
    //imshow(win_name, srcR+srcG);
        imshow(win_name, srcG);


}

static void edge_CB(int, void*){
    Mat edge;
    edge = preProcess(srcR+srcG);
    imshow(win_name, edge);
}

static void rect_CB(int, void*){
    Mat dst, edge;
    vector<RotatedRect> minRects(n_obj);
    edge = preProcess(srcR+srcG);
    minRects = findObj(edge);
    dst = drawInfo(src, minRects);
    imshow(win_name, dst);
}

bool compareArea(vector<Point> a, vector<Point> b){
    return contourArea(a) > contourArea(b);
    //return arcLength(a, true) > arcLength(b, true);
}

Mat  preProcess(Mat input){
    Mat edge;
    Canny(srcG+srcR, edge, low_thres, up_thres, 3);
    dilate(edge, edge, Mat(),Point(-1,-1), dilate_itera);
    //erode(edge, edge, Mat(),Point(-1,-1), dilate_itera);
    return edge;
}

vector<RotatedRect> findObj(Mat input){
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    vector<RotatedRect> minRects;
    findContours(input, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    vector<vector<Point> > contours_poly(contours.size());
    for(int i=0; i<contours.size(); i++){
        approxPolyDP(Mat(contours[i]), contours_poly[i], 100, true);
    }
    sort(contours_poly.begin(), contours_poly.end(), compareArea);
    if(n_obj>contours_poly.size()) return minRects;
    for(int i=0; i<n_obj; i++){
        minRects.push_back(minAreaRect(Mat(contours_poly[i])));
    }
    return minRects;
}

Mat drawInfo(Mat src, vector<RotatedRect> minRects){
    if(minRects.size()<n_obj) return src;
    Mat output = src.clone();
    Point2f shift(30,-30);
    for(int i=0; i<minRects.size(); i++){
        Point2f rect_points[4];
        minRects[i].points( rect_points );
        for( int j = 0; j < 4; j++ ){
            line( output, rect_points[j], rect_points[(j+1)%4], Scalar(0,0,0), 10, 8 );
        }
        string objType = classifyObj(minRects[i]);
        //int center_x = minRects[i].center.x;
        //int center_y = minRects[i].center.y;
        //string coord = format("( %i, %i)", center_x, center_y);
        circle(output, minRects[i].center, 20, Scalar(0,0,0), CV_FILLED,8);
        putText(output, objType, minRects[i].center+shift, 0, 4, Scalar(0,0,0), 5, 8);
    }
    return output;
}

string classifyObj(RotatedRect minRect){
    float error = (float)ratioErrTolerance/100.0;
    float sideA = minRect.size.width;
    float sideB = minRect.size.height;
    float ratio=0;
    Point center = minRect.center;
    if(sideA>sideB || sideA == sideB) ratio = sideA/sideB;
    else ratio = sideB/sideA;
    if(fabs(ratio-1.0) < error){
        if(srcR.at<uchar>(center.y,center.x) == 255){
            return "Red Cubic";
        } 
        else if(srcG.at<uchar>(center.y,center.x) == 255){
            return "Green Cubic";
        }
        else{
            return "UNKNOWN";
        }
    }
    else if(fabs(ratio-2.0) < error){
        if(srcR.at<uchar>(center.y,center.x) == 255){
            return "UNKNOWN";
        }
        else if(srcG.at<uchar>(center.y,center.x) == 255){
            return "Green Oval";
        }
        else{
            return "UNKNOWN";
        }
    }
    else return "UNKNOWN";
}

void setupWindow(){
    namedWindow(win_name, WINDOW_KEEPRATIO);
    createTrackbar("Upper Threshold: ", win_name, &up_thres, MAX_THRES, edge_CB);
    createTrackbar("Lower Threshold: ", win_name, &low_thres, MAX_THRES, edge_CB);
    createTrackbar("Dilate Iterate: ", win_name, &dilate_itera, MAX_ITERATE, edge_CB);
    createTrackbar("Number of object: ", win_name, &n_obj, MAX_N_obj, rect_CB);
    namedWindow(ctrl_win, WINDOW_KEEPRATIO);
    createTrackbar("Red High H", ctrl_win, &(redHighHSV[0]), 179, color_CB);
    createTrackbar("Red Low H", ctrl_win, &(redLowHSV[0]), 179, color_CB);
    createTrackbar("Red High S", ctrl_win, &(redHighHSV[1]), 255, color_CB);
    createTrackbar("Red Low S", ctrl_win, &(redLowHSV[1]), 255, color_CB);
    createTrackbar("Red High V", ctrl_win, &(redLowHSV[1]), 255, color_CB);
    createTrackbar("Red Low V", ctrl_win, &(redLowHSV[2]), 255, color_CB);
    createTrackbar("Green High H", ctrl_win, &(greenHighHSV[0]), 179, color_CB);
    createTrackbar("Green Low H", ctrl_win, &(greenLowHSV[0]), 179, color_CB);
    createTrackbar("Green High S", ctrl_win, &(greenHighHSV[1]), 255, color_CB);
    createTrackbar("Green Low S", ctrl_win, &(greenLowHSV[1]), 255, color_CB);
    createTrackbar("Green High V", ctrl_win, &(greenHighHSV[2]), 255, color_CB);
    createTrackbar("Green Low V", ctrl_win, &(greenLowHSV[2]), 255, color_CB);
    createTrackbar("Ratio Error Tolerance (0\%~100\%)", ctrl_win, &ratioErrTolerance, 100, rect_CB);
    return ;
}