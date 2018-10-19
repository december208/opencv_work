#include <stdio.h>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

//fuction prototype
static void edge_CB(int, void*);
static void rect_CB(int, void*);
static void color_CB(int, void*);
void setupWindow();
bool compareArea(vector<Point> a, vector<Point> b);
Mat preProcess(Mat input);
vector<RotatedRect> findObj( Mat input);
Mat drawMinRect(Mat src, vector<RotatedRect> minRects);


//global parameters
int low_thres=100;
int up_thres=200;
int MCitera = 3;
int n_obj = 3;
int redLowHSV[] = {176,151,128};
int redHighHSV[] = {179,220,230};
int greenLowHSV[] = {70,50,85};
int greenHighHSV[] = {80,255,255};
static int MAX_ITERATE = 20;
static int MAX_THRES = 300;
static int MAX_N_obj = 10;
char win_name[] = "Display Image";
char ctrl_win[] = "Control Panel";
Mat src,srcRed,srcGre;

int main(int argc, char** argv ){

    src = imread( argv[1], 1);

    setupWindow();    
    color_CB(0,0);
    edge_CB(0,0);
    waitKey(0);
    rect_CB(0,0);
    waitKey(0);
    return 0;
}

static void color_CB(int, void*){
    Mat srcHSV, srcRed, srcGre;
    cvtColor(src, srcHSV, COLOR_BGR2HSV);
    inRange(srcHSV, Scalar(greenLowHSV[0],greenLowHSV[1],greenLowHSV[2]), Scalar(greenHighHSV[0],greenHighHSV[1],greenHighHSV[2]), srcGre);
    inRange(srcHSV, Scalar(redLowHSV[0],redLowHSV[1],redLowHSV[2]), Scalar(redHighHSV[0],redHighHSV[1],redHighHSV[2]), srcRed);    
    dilate(srcRed,srcRed, Mat(), Point(-1,-1), MCitera);
    dilate(srcGre,srcGre, Mat(), Point(-1,-1), MCitera);
    imshow(ctrl_win, srcRed+srcGre);
}

static void edge_CB(int, void*){
    Mat edge;
    edge = preProcess(src);
    imshow(win_name, edge);

}
static void rect_CB(int, void*){
    Mat edge, dst;
    vector<RotatedRect> minRects(n_obj);
    edge = preProcess(src);
    minRects = findObj(edge);
    dst = drawMinRect(src, minRects);
    imshow(win_name, dst);
}

bool compareArea(vector<Point> a, vector<Point> b){
    return arcLength(a, false) > arcLength(b, false);
    //return contourArea(a, false) < contourArea(b, false);
}

Mat  preProcess(Mat input){
    Mat src_gray, blured, edge, dst;
    cvtColor(input,src_gray,COLOR_BGR2GRAY);
    GaussianBlur(src_gray,blured,Size(11,11),0,0);
    Canny(src_gray, edge, low_thres, up_thres, 3);
    dilate(edge, edge, Mat(),Point(-1,-1), MCitera);
    erode(edge, edge, Mat(),Point(-1,-1), MCitera);
    return edge;
}

vector<RotatedRect> findObj(Mat input){
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(input, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    vector<vector<Point> > contours_poly( contours.size() );
    for(int i=0; i<contours.size(); i++){
        approxPolyDP(Mat(contours[i]), contours_poly[i], 100, true);
    }
    sort(contours_poly.begin(), contours_poly.end(), compareArea);
    vector<RotatedRect> minRects(n_obj);
    if(n_obj>contours_poly.size()) return minRects;
    for(int i=0; i<n_obj; i++){
        minRects[i] = minAreaRect(Mat(contours_poly[i]));
    }
    return minRects;
}


Mat drawMinRect(Mat src, vector<RotatedRect> minRects){
    if(minRects.size()<n_obj) return src;
    Mat output = src.clone();
    for(int i=0; i<minRects.size(); i++){
        Point2f rect_points[4];
        minRects[i].points( rect_points );
        for( int j = 0; j < 4; j++ ){
            line( output, rect_points[j], rect_points[(j+1)%4], Scalar(0,0,0), 10, 8 );
        }
        int center_x = minRects[i].center.x;
        int center_y = minRects[i].center.y;
        string center = format("( %i, %i)", center_x, center_y);
        circle(output, minRects[i].center, 20, Scalar(255,0,0), 20,8);
        putText(output, center, minRects[i].center, 0, 3, Scalar(255,0,0), 5, 8);
    }
    return output;
}

void setupWindow(){
    namedWindow(win_name, WINDOW_KEEPRATIO);
    createTrackbar("Upper Threshold: ", win_name, &up_thres, MAX_THRES, edge_CB);
    createTrackbar("Lower Threshold: ", win_name, &low_thres, MAX_THRES, edge_CB);
    createTrackbar("Dilate Iterate: ", win_name, &MCitera, MAX_ITERATE, edge_CB);
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
    return ;
}