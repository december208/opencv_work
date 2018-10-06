#include <stdio.h>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat src, src_gray, edge, blured;
vector<vector<Point> > contours;
int low_thres=50;
int up_thres=100;
int MCitera = 3;
static int MAX_ITERATE = 20;
static int MAX_THRES = 400;
char win_name[] = "Display Image";


static void track_CB(int, void*);


int main(int argc, char** argv ){
    if ( argc != 2 ){
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }
    src = imread( argv[1], 1 );
    if ( !src.data ){
        printf("No image data \n");
        return -1;
    }


    namedWindow(win_name, WINDOW_KEEPRATIO);
    createTrackbar("UP_Threshold", win_name, &up_thres, MAX_THRES, track_CB);
    createTrackbar("LOW_Threshold", win_name, &low_thres, MAX_THRES, track_CB);
    createTrackbar("MORPH_CLOSE_ITERATE", win_name, &MCitera, MAX_ITERATE, track_CB);


    
    cvtColor(src,src_gray,COLOR_BGR2GRAY);
    //blur(src_gray, blured, Size(11,11), Point(-1,-1));
    GaussianBlur(src_gray,blured,Size(7,7),0,0);
    Canny(blured, edge,low_thres,up_thres,3);
    morphologyEx(edge, edge, MORPH_CLOSE, Mat(), Point(-1,-1), MCitera);
    imshow(win_name, edge);
    waitKey(0);

    vector<Vec4i> hierarchy;
    findContours(edge, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    
    cout<<contours.size()<<endl;
    
    for(int i=0; i<contours.size(); i++){
        drawContours(src, contours, i, Scalar(0,0,255), 5,8, hierarchy);
    }
    imshow(win_name, src);
    waitKey(0);
    return 0;
}

static void track_CB(int, void*){
    Canny(blured, edge, low_thres, up_thres, 3);
    morphologyEx(edge, edge, MORPH_CLOSE, Mat(), Point(-1,-1), MCitera);
    imshow(win_name, edge);
}


