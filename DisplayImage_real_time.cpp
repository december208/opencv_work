#include <stdio.h>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

//fuction prototype
static void edge_CB(int, void*);
static void rect_CB(int, void*);
bool compareArea(vector<Point> a, vector<Point> b);
Mat preProcess(Mat input);
Mat drawFindedObj(Mat src, Mat input);
Mat drawMinRect(Mat src, vector<RotatedRect> minRects);

//global parameters
int low_thres=50;
int up_thres=100;
int MCitera = 3;
int n_obj = 3;
static int MAX_ITERATE = 20;
static int MAX_THRES = 300;
static int MAX_N_obj = 10;
char win_name[] = "Display Image";
char ctrl_win[] = "Control Panel";
VideoCapture cap(0);

int main(int argc, char** argv ){

    namedWindow(win_name, WINDOW_KEEPRATIO);
    createTrackbar("UP Threshold: ", win_name, &up_thres, MAX_THRES, edge_CB);
    createTrackbar("LOW Threshold: ", win_name, &low_thres, MAX_THRES, edge_CB);
    createTrackbar("MORPH CLOSE ITERATE: ", win_name, &MCitera, MAX_ITERATE, edge_CB);
    createTrackbar("Number of object: ", win_name, &n_obj, MAX_N_obj, rect_CB);
    
    while(1){
        if(!cap.isOpened()) cout<<"not yet\n";
        else break;
    }
    Mat src, edge, dst;
    while(cap.isOpened()){
        cap>>src;
        for(int i=0; i<5; i++) cap.grab();
        edge = preProcess(src);
        imshow(win_name, edge);
        if(waitKey(10)>=0) break;;
    }
    while(cap.isOpened()){
        cap>>src;
        for(int i=0; i<5; i++) cap.grab();
        edge = preProcess(src);
        dst = drawFindedObj(src, edge);
        imshow(win_name, dst);
        if(waitKey(10)>=0) break;;
    }   
    return 0;
}

static void edge_CB(int, void*){}
static void rect_CB(int, void*){}

bool compareArea(vector<Point> a, vector<Point> b){
    return contourArea(a, false) < contourArea(b, false);
}

Mat  preProcess(Mat input){
    Mat src_gray, blured, edge, dst;
    cvtColor(input,src_gray,COLOR_BGR2GRAY);
    GaussianBlur(src_gray,blured,Size(3,3),0,0);
    Canny(src_gray, edge, low_thres, up_thres, 3);
    morphologyEx(edge, edge, MORPH_CLOSE, Mat(), Point(-1,-1), MCitera);
    return edge;
}

vector<RotatedRect> findObj(Mat src, Mat input){
    Mat output = src.clone();
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(input, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    vector<vector<Point> > contours_poly( contours.size() );
    for(int i=0; i<contours.size(); i++){
        approxPolyDP(Mat(contours[i]), contours_poly[i], 100, true);
    }
    sort(contours_poly.begin(), contours_poly.end(), compareArea);
    vector<RotatedRect> minRects(n_obj);
    if(n_obj>contours_poly.size()) return src;
    for(int i=0; i<n_obj; i++){
        minRects[i] = minAreaRect(Mat(contours_poly[i]));
    }
    return minRects;
}


Mat drawMinRect(Mat src, vector<RotatedRect> minRects){
    Mat output = src.clone();
    for(int i=0; i<minRects.size(); i++){
        Point2f rect_points[4];
        minRects[i].points( rect_points );
        for( int j = 0; j < 4; j++ ){
            line( output, rect_points[j], rect_points[(j+1)%4], Scalar(0,0,0), 3, 8 );
        }
        int center_x = minRects[i].center.x;
        int center_y = minRects[i].center.y;
        string center = format("( %i, %i)", center_x, center_y);
        circle(output, minRects[i].center, 10, Scalar(0,0,0), 10,8);
        putText(output, center, minRects[i].center, 0, 3, Scalar(0,0,0), 5, 8);
    }
    return output;
}

