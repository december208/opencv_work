#include <string>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


//fuction prototype
static void edge_CB(int, void*);
static void rect_CB(int, void*);
bool compareArcLength(vector<Point> a, vector<Point> b);


//global parameters
Mat src, src_gray, blured, edge, dst;
int low_thres=50;
int up_thres=100;
int MCitera = 3;
int n_obj = 3;
static int MAX_ITERATE = 20;
static int MAX_THRES = 400;
static int MAX_N_obj = 10;
char win_name[] = "Display Image";


int main(int argc, char** argv ){

    if ( argc != 2 ){
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }
    src = imread( argv[1], 1);
    if ( !src.data ){
        printf("No image data \n");
        return -1;
    }

    namedWindow(win_name, WINDOW_KEEPRATIO);
    createTrackbar("UP Threshold: ", win_name, &up_thres, MAX_THRES, edge_CB);
    createTrackbar("LOW Threshold: ", win_name, &low_thres, MAX_THRES, edge_CB);
    createTrackbar("MORPH CLOSE ITERATE: ", win_name, &MCitera, MAX_ITERATE, edge_CB);
    createTrackbar("Number of object: ", win_name, &n_obj, MAX_N_obj, rect_CB);


    cvtColor(src,src_gray,COLOR_BGR2GRAY);
    GaussianBlur(src_gray,blured,Size(17,17),0,0);
    edge_CB(0,0);
    waitKey(0);
    rect_CB(0,0);
    waitKey(0);
    return 0;
}

static void edge_CB(int, void*){
    Canny(blured, edge, low_thres, up_thres, 3);
    //morphologyEx(edge, edge, MORPH_CLOSE, Mat(), Point(-1,-1), MCitera);
    dilate(edge,edge,Mat(),Point(-1,-1), MCitera);
    imshow(win_name, edge);
}

static void rect_CB(int, void*){
    dst = src.clone();
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(edge, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    vector<vector<Point> > contours_poly( contours.size() );
    
    for(int i=0; i<contours.size(); i++){
        approxPolyDP(Mat(contours[i]), contours_poly[i], 50, true);
    }
    sort(contours_poly.begin(), contours_poly.end(), compareArcLength);
    vector<RotatedRect> minRects(n_obj);
    if(contours.size()<n_obj){
        imshow(win_name, src);
        return;
    }
    for(int i=0; i<n_obj; i++){
        minRects[i] = minAreaRect(Mat(contours_poly[i]));
        Point2f rect_points[4];
        minRects[i].points( rect_points );
        for( int j = 0; j < 4; j++ ){
            line( dst, rect_points[j], rect_points[(j+1)%4], Scalar(0,0,255), 7, 8 );
        }
        int center_x = minRects[i].center.x;
        int center_y = minRects[i].center.y;
        string center = format("( %i, %i)", center_x, center_y);
        circle(dst, minRects[i].center, 10, Scalar(0,0,0), 10,8);
        putText(dst, center, minRects[i].center, 0, 3, Scalar(0,0,0), 5, 8);

    }   
    imshow(win_name, dst);
    return;
}

bool compareArcLength(vector<Point> a, vector<Point> b){
    return arcLength(a, true) > arcLength(b, true);
}